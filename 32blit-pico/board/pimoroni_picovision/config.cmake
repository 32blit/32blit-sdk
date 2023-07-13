set(BLIT_BOARD_NAME "PicoVision")

set(BLIT_BOARD_DEFINITIONS
    PICO_AUDIO_I2S_PIO=0
    PICO_AUDIO_I2S_DATA_PIN=26
    PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
    PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH=256
)

blit_driver(audio i2s)
#blit_driver(display picovision)
blit_driver(input usb_hid)
#blit_driver(storage sd_spi)
blit_driver(usb host)
