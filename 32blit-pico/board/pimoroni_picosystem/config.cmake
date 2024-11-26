set(BLIT_BOARD_NAME "PicoSystem")

set(BLIT_BOARD_DEFINITIONS
    # non-default PWM audio support
    PICO_AUDIO_PWM_MONO_PIN=PICOSYSTEM_AUDIO_PIN
    PICO_AUDIO_PWM_PIO=1

    # global for compatibility reasons
    DISPLAY_ST7789
)

blit_driver(audio beep)
blit_driver(display dbi)
blit_driver(input gpio)
