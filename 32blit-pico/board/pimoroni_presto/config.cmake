set(BLIT_BOARD_NAME "Presto")

set(BLIT_BOARD_DEFINITIONS
    PICO_DEFAULT_I2C=0
    PICO_DEFAULT_I2C_SDA_PIN=40
    PICO_DEFAULT_I2C_SCL_PIN=41
)

blit_driver(audio beep)
blit_driver(display dpi)
blit_driver(input tca9555)
blit_driver(storage sd_spi)
