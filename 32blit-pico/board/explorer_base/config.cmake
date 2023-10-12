set(BLIT_BOARD_NAME "Explorer Base")

set(AUDIO_PIN 7 CACHE STRING "Pin for audio output")

set(BLIT_BOARD_DEFINITIONS
    AUDIO_BEEP_PIN=${AUDIO_PIN} # not connected to anything by default
)

blit_driver(audio beep)
blit_driver(display st7789)
blit_driver(input gpio)
