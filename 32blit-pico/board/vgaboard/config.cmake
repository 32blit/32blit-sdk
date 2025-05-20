set(BLIT_BOARD_NAME "VGA Board")

blit_driver(audio i2s)
blit_driver(display dpi)
blit_driver(input usb_hid)
blit_driver(storage sd_spi)
blit_driver(usb host)

set(BLIT_ENABLE_CORE1 TRUE)
