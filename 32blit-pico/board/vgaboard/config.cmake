set(BLIT_BOARD_NAME "VGA Board")

set(BLIT_BOARD_DEFINITIONS
    PICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=1
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=12
)

blit_driver(audio i2s)
blit_driver(display scanvideo)
blit_driver(input usb_hid)
blit_driver(storage sd_spi)
blit_driver(usb host)
