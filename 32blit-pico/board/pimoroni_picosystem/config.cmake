set(BLIT_BOARD_NAME "PicoSystem")

set(BLIT_BOARD_DEFINITIONS
    # non-default PWM audio support
    PICO_AUDIO_PWM_MONO_PIN=PICOSYSTEM_AUDIO_PIN
    PICO_AUDIO_PWM_PIO=1
)

blit_driver(audio beep)
blit_driver(display st7789)
blit_driver(input picosystem)
