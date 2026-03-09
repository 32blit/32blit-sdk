
#include "powman.h"

static powman_power_state off_state;
static powman_power_state on_state;

// Long press to sleep LED effect
#define LED_PEAK_BRIGHTNESS 150 // Total brightness value per led (this is multiplied by GAMMA)
#define LED_IN_PHASE 30 // The delay between the current and next LED lighting up
#define LED_OUT_PHASE 0
#define LED_TOTAL (LED_PEAK_BRIGHTNESS + LED_IN_PHASE * 3) // Total bright phase for the LEDs
#define LED_FADEOUT_SPEED 5 // Speedup for fading out
#define LED_GAMMA 1.8f
#define LED_DELAY_MS 5 // Delay between steps
#define LED_ON_DELAY_MS 200 // A short delay before the LEDs start their thing
#define LED_ON_DELAY (LED_ON_DELAY_MS / LED_DELAY_MS) // Convert the above delay in terms of steps

// This is effectively the sequence order for the swooshy LED effect
const uint led_gpios[4] = {BW_LED_1, BW_LED_2, BW_LED_3, BW_LED_0};
//const uint led_gpios_poweron[4] = {BW_LED_0, BW_LED_2, BW_LED_3, BW_LED_1};

//#define DEBUG

static void i2c_enable(void) {
    gpio_init(BW_SW_POWER_EN);
    gpio_set_dir(BW_SW_POWER_EN, GPIO_OUT);
    gpio_put(BW_SW_POWER_EN, 1);

    sleep_ms(500);

    i2c_init(BW_RTC_I2C, 400 * 1000);
    gpio_set_function(BW_RTC_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(BW_RTC_I2C_SCL, GPIO_FUNC_I2C);
}

static void i2c_disable(void) {
    gpio_init(BW_SW_POWER_EN);
    gpio_init(BW_RTC_I2C_SDA);
    gpio_init(BW_RTC_I2C_SCL);
}

static inline uint8_t pcf85063_get_timer_flag() {
    uint8_t buf = 0x01;
    i2c_write_blocking(BW_RTC_I2C, BW_RTC_ADDR, &buf, 1, false);
    i2c_read_blocking(BW_RTC_I2C, BW_RTC_ADDR, (uint8_t *)&buf, 1, false);
    return (buf & 0x08) == 0x08;
}

static inline void pcf85063_clear_timer_flag() {
    uint8_t buf[2] = {0x01, 0b00000000}; // Control_2
    i2c_write_blocking(BW_RTC_I2C, BW_RTC_ADDR, buf, 2, false);
}

static inline void pcf85063_disable_interrupt() {
    // Disable RTC timer interrupt
    uint8_t data3[2] = {0x11, 0b00000000};
    i2c_write_blocking(BW_RTC_I2C, 0x51, data3, 2, false);
}

static void powman_init() {
    uint64_t abs_time_ms = 1746057600000; // 2025/05/01 - Milliseconds since epoch

    // Run everything from pll_usb pll and stop pll_sys
    set_sys_clock_48mhz();

    // Use the 32768 Hz clockout from the RTC to keep time accurately
    //clock_configure_gpin(clk_ref, 12, 32.768f * KHZ, 32.768f * KHZ);
    //clock_configure_gpin(clk_sys, 12, 32.768f * KHZ, 32.768f * KHZ);
    //clock_configure_undivided(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, 32.768f * KHZ);
    //powman_timer_set_1khz_tick_source_lposc_with_hz(32768);

    // Redundant? - we're using the RTC clock
    //powman_timer_set_1khz_tick_source_lposc();

    // Does this accomplish *anything*?
    //pll_deinit(pll_sys);

    for (unsigned i = 0; i < NUM_BANK0_GPIOS; ++i) {
        gpio_set_function(i, GPIO_FUNC_SIO);
        gpio_set_dir(i, GPIO_IN);
        gpio_set_input_enabled(i, false);
        switch (i) {
            case 8:
                gpio_set_pulls(i, true, false);
                break;
            case BW_RESET_SW:
            case BW_SWITCH_HOME:
            case 40:
            case BW_SW_POWER_EN:
            case 42: // Floating
                gpio_disable_pulls(i);
                break;
            case BW_SWITCH_A:
            case BW_SWITCH_B:
            case BW_SWITCH_C:
            case BW_SWITCH_UP:
            case BW_SWITCH_DOWN: // Don't mess with the button pulls, must be pulled up
                break;
            default: // Pull down
                gpio_set_pulls(i, false, true);
        }
    }

    // Unlock the VREG control interface
    hw_set_bits(&powman_hw->vreg_ctrl, POWMAN_PASSWORD_BITS | POWMAN_VREG_CTRL_UNLOCK_BITS);

    // Reset usb controller
    reset_block_mask(RESETS_RESET_USBCTRL_BITS);
    unreset_block_mask_wait_blocking(RESETS_RESET_USBCTRL_BITS);

    // Mux the controller to the onboard usb phy
    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;

    // Initializes the USB peripheral for device mode and enables it.
    // Don't need to enable the pull up here. Force VBUS
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    // Enable individual controller IRQS here. Processor interrupt enable will be used
    // for the global interrupt enable...
    // Note: Force VBUS detect cause disconnection not detectable
    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS | USB_INTS_SETUP_REQ_BITS |
                    USB_INTS_DEV_SUSPEND_BITS | USB_INTS_DEV_RESUME_FROM_HOST_BITS | USB_INTS_DEV_CONN_DIS_BITS;


    // Turn off USB PHY and apply pull downs on DP & DM
    usb_hw->phy_direct = USB_USBPHY_DIRECT_TX_PD_BITS | USB_USBPHY_DIRECT_RX_PD_BITS | USB_USBPHY_DIRECT_DM_PULLDN_EN_BITS | USB_USBPHY_DIRECT_DP_PULLDN_EN_BITS;

    usb_hw->phy_direct_override = USB_USBPHY_DIRECT_RX_DM_BITS | USB_USBPHY_DIRECT_RX_DP_BITS |          USB_USBPHY_DIRECT_RX_DD_BITS |
        USB_USBPHY_DIRECT_OVERRIDE_TX_DIFFMODE_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_DM_PULLUP_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_TX_FSSLEW_OVERRIDE_EN_BITS |
        USB_USBPHY_DIRECT_OVERRIDE_TX_PD_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_RX_PD_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_TX_DM_OVERRIDE_EN_BITS |
        USB_USBPHY_DIRECT_OVERRIDE_TX_DP_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_TX_DM_OE_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_TX_DP_OE_OVERRIDE_EN_BITS |
        USB_USBPHY_DIRECT_OVERRIDE_DM_PULLDN_EN_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_DP_PULLDN_EN_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_DP_PULLUP_EN_OVERRIDE_EN_BITS |
        USB_USBPHY_DIRECT_OVERRIDE_DM_PULLUP_HISEL_OVERRIDE_EN_BITS | USB_USBPHY_DIRECT_OVERRIDE_DP_PULLUP_HISEL_OVERRIDE_EN_BITS;

    // start powman and set the time
    powman_timer_start();
    powman_timer_set_ms(abs_time_ms);

    // Allow power down when debugger connected
    powman_set_debug_power_request_ignored(true);

    // Power states
    powman_power_state P1_7 = POWMAN_POWER_STATE_NONE;

    powman_power_state P0_3 = POWMAN_POWER_STATE_NONE;
    P0_3 = powman_power_state_with_domain_on(P0_3, POWMAN_POWER_DOMAIN_SWITCHED_CORE);
    P0_3 = powman_power_state_with_domain_on(P0_3, POWMAN_POWER_DOMAIN_XIP_CACHE);

    off_state = P1_7;
    on_state = P0_3;
}

// Initiate power off
static int __no_inline_not_in_flash_func(powman_off)(void) {
    // Set power states
    bool valid_state = powman_configure_wakeup_state(off_state, on_state);
    if (!valid_state) {
        return PICO_ERROR_INVALID_STATE;
    }

    // reboot to main
    powman_hw->boot[0] = 0;
    powman_hw->boot[1] = 0;
    powman_hw->boot[2] = 0;
    powman_hw->boot[3] = 0;

    // Switch to required power state
    int rc = powman_set_power_state(off_state);
    if (rc != PICO_OK) {
        return rc;
    }

    // Power down
    while (true) __wfi();
}

static int powman_setup_gpio_wakeup(int hw_wakeup, int gpio, bool edge, bool high, uint64_t timeout_ms) {
    gpio_init(gpio);
    gpio_set_dir(gpio, false);
    gpio_set_input_enabled(gpio, true);

    // Must set pulls here, or our pin may never go into its idle state
    gpio_set_pulls(gpio, !high, high);

    // If the pin is currently in a triggered state, wait for idle
    absolute_time_t timeout = make_timeout_time_ms(timeout_ms);
    if (gpio_get(gpio) == high) {
        while(gpio_get(gpio) == high) {
            sleep_ms(10);
            if(time_reached(timeout)) return -1;
        }
    }
    powman_enable_gpio_wakeup(hw_wakeup, gpio, edge, high);

    return 0;
}

static inline void setup_gpio(bool buttons_only) {
    // Init all button GPIOs
    gpio_init_mask(BW_SWITCH_MASK);
    gpio_set_dir_in_masked(BW_SWITCH_MASK);
    gpio_set_pulls(BW_SWITCH_A, true, false);
    gpio_set_pulls(BW_SWITCH_B, true, false);
    gpio_set_pulls(BW_SWITCH_C, true, false);
    gpio_set_pulls(BW_SWITCH_UP, true, false);
    gpio_set_pulls(BW_SWITCH_DOWN, true, false);

    if(buttons_only) {
        return;
    }

    // Init the home button
    gpio_init(BW_SWITCH_HOME);
    gpio_set_dir(BW_SWITCH_HOME, GPIO_IN);
    gpio_set_pulls(BW_SWITCH_HOME, true, false);

    // Init the button interrupt
    gpio_init(BW_SWITCH_INT);
    gpio_set_dir(BW_SWITCH_INT, GPIO_IN);
    gpio_set_pulls(BW_SWITCH_INT, true, false);

    // Init the RTC interrupt
    gpio_init(BW_RTC_ALARM);
    gpio_set_dir(BW_RTC_ALARM, GPIO_IN);
    gpio_set_pulls(BW_RTC_ALARM, true, false);

    // Init the VBUS detect
    gpio_init(BW_VBUS_DETECT);
    gpio_set_dir(BW_VBUS_DETECT, GPIO_IN);
    gpio_set_pulls(BW_VBUS_DETECT, false, false);

    // Moved to RM2
    // Init the charge status detect
    // gpio_init(BW_CHARGE_STAT);
    // gpio_set_dir(BW_CHARGE_STAT, GPIO_IN);
    // gpio_set_pulls(BW_CHARGE_STAT, true, false);

    // Set up LEDs
    gpio_init_mask(0b1111);
    gpio_set_dir_out_masked(0b1111);

    // Set up long press detect
    gpio_init(BW_RESET_SW);
    gpio_set_dir(BW_RESET_SW, GPIO_IN);
    gpio_pull_up(BW_RESET_SW);

    // Enable I2C power
    gpio_init(BW_SW_POWER_EN);
    gpio_set_dir(BW_SW_POWER_EN, GPIO_OUT);
    gpio_put(BW_SW_POWER_EN, 1);
}

// disable RTC interrupt
static inline void setup_system(void) {
    i2c_enable();
    pcf85063_disable_interrupt();
}

static void shipping_mode() {
    powman_init();
    i2c_disable();
    int rc = powman_off();
    hard_assert(rc == PICO_OK);
    hard_assert(false); // should never get here!
}

static void long_press_sleep() {
    powman_init();

    // We must set the pulls on the user buttons or they will not be sufficient
    // to trigger the interrupt pin
    setup_gpio(true);

    int err;
    //(void)powman_setup_gpio_wakeup(POWMAN_WAKE_PWRUP0_CH, BW_VBUS_DETECT, true, true, 1000);
    err = powman_setup_gpio_wakeup(POWMAN_WAKE_PWRUP1_CH, BW_RTC_ALARM, true, false, 1000);
    //err = powman_setup_gpio_wakeup(POWMAN_WAKE_PWRUP2_CH, BW_RESET_SW, true, true, 1000);
    err = powman_setup_gpio_wakeup(POWMAN_WAKE_PWRUP3_CH, BW_SWITCH_INT, true, false, 1000);
    (void)err;

    i2c_disable();
    int rc = powman_off();
    hard_assert(rc == PICO_OK);
    hard_assert(false); // should never get here!
}

static void handle_long_press() {
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / 2048.0f);
    pwm_config_set_wrap(&config, 1024);
    for(unsigned i = 0u; i < 4; i++) {
        gpio_set_function(led_gpios[i], GPIO_FUNC_PWM);
        pwm_init(pwm_gpio_to_slice_num(led_gpios[i]), &config, true);
    }
    int br = 0;
    while (true) {
        // If reset is released (high), cut the long press short
        if(gpio_get(BW_RESET_SW)) {
            break;
        }
        int cbr = br < LED_ON_DELAY ? 0 : br - LED_ON_DELAY;
        int o = cbr >= LED_TOTAL ? LED_TOTAL - (cbr - LED_TOTAL) * LED_FADEOUT_SPEED : cbr;
        int phase = cbr >= LED_TOTAL ? LED_OUT_PHASE : LED_IN_PHASE;
        int level = 0;
        for(unsigned i = 0u; i < 4; i++) {
            int v = fmax(0, fmin(LED_PEAK_BRIGHTNESS, o));
            pwm_set_gpio_level(led_gpios[i], v * LED_GAMMA);
            level += v * LED_GAMMA;
            o -= phase;
        }
        // If the LEDs have completed their fade to black
        if(cbr > 0 && level == 0) {
            if(!gpio_get(BW_SWITCH_UP) && !gpio_get(BW_SWITCH_DOWN)) {
                shipping_mode();
            } else {
                long_press_sleep();
            }
            break; // unreachable
        };
        br++;
        sleep_ms(LED_DELAY_MS);
    }
    for(unsigned i = 0u; i < 4; i++) {
        pwm_set_enabled(pwm_gpio_to_slice_num(led_gpios[i]), false);
    }
    gpio_init_mask(0b1111);
    gpio_set_dir_out_masked(0b1111);
    gpio_put_masked(0b1111, 0);
}

static void __attribute__((constructor)) powman_startup(void) {
    setup_gpio(false);

    // If we haven't reset via a button press we ought not to delay startup
    if (!(powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_RUN_LOW_BITS) || watchdog_caused_reboot()) {
        setup_system();
        //power_on_leds();
        return;
    };

    // If reset is held (low), it could be a long press
    if(gpio_get(BW_RESET_SW) == 0) {
        handle_long_press();
    }

    setup_system();
    //power_on_leds();
}
