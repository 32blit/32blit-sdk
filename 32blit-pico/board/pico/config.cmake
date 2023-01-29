set(BLIT_BOARD_NAME "Pico/default")

# TODO: this is the hacky explorer setup I initially used
set(BLIT_BOARD_DEFINITIONS
    PICO_AUDIO_I2S_DATA_PIN=6
    PICO_AUDIO_I2S_CLOCK_PIN_BASE=20
    AUDIO_I2S
    DISPLAY_ST7789
    INPUT_GPIO
)

set(BLIT_BOARD_LIBRARIES pico_audio_i2s)

set(BLIT_REQUIRE_PICO_EXTRAS TRUE) # audio_i2s
