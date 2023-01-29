set(BLIT_BOARD_NAME "PicoSystem")

set(BLIT_BOARD_DEFINITIONS
    AUDIO_BEEP_PIN=PICOSYSTEM_AUDIO_PIN
    DISPLAY_ST7789 # code in both the sdk and some games still depends on this
    INPUT_GPIO
    LED_R_PIN=PICOSYSTEM_LED_R_PIN
    LED_G_PIN=PICOSYSTEM_LED_G_PIN
    LED_B_PIN=PICOSYSTEM_LED_B_PIN
    USB_PRODUCT_STR="PicoSystem"

    # non-default PWM audio support
    PICO_AUDIO_PWM_MONO_PIN=PICOSYSTEM_AUDIO_PIN
    PICO_AUDIO_PWM_PIO=1
)

blit_driver(audio beep)
blit_driver(display st7789)
