set(BLIT_BOARD_NAME "Display Pack 2.0")

set(BLIT_BOARD_DEFINITIONS
    DISPLAY_WIDTH=320

    LED_INVERTED
    LED_R_PIN=6
    LED_G_PIN=7
    LED_B_PIN=8
)

blit_driver(display st7789)
blit_driver(input gpio_abxy)
