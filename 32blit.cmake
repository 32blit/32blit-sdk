message(DEPRECATION "\
Please update your 32Blit cmake project to use:
    find_package(32BLIT CONFIG REQUIRED /path/to/32blit-beta)
instead of:
    include(32blit.cmake)
and use the variable 32BLIT_DIR instead of 32BLIT_PATH.
")
include(${CMAKE_CURRENT_LIST_DIR}/32blit-config.cmake)
