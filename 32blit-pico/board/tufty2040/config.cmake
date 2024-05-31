set(BLIT_BOARD_NAME "Tufty 2040")

set(BLIT_BOARD_DEFINITIONS
    PICO_FLASH_SIZE_BYTES=8388608
)

blit_driver(display st7789)
blit_driver(input gpio)
