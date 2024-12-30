set(BLIT_BOARD_NAME "VGA Board")

set(BLIT_BOARD_DEFINITIONS
    PICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=1
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=12

    # these are duplicated so we can build for non-pico boards (vgaboard.h includes pico.h)
    PICO_SCANVIDEO_COLOR_PIN_BASE=0
    PICO_SCANVIDEO_SYNC_PIN_BASE=16
)

blit_driver(audio i2s)
blit_driver(display scanvideo)
blit_driver(input usb_hid)
blit_driver(storage sd_spi)
blit_driver(usb host)
