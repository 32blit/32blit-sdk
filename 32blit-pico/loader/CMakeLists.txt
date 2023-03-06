add_executable(blit-loader loader.cpp)
target_link_libraries(blit-loader BlitHalPico BlitEngine)
target_link_options(blit-loader PUBLIC -specs=nano.specs -u _printf_float)
target_compile_definitions(blit-loader PRIVATE BUILD_LOADER PICO_EMBED_XIP_SETUP=1)

pico_set_linker_script(blit-loader ${CMAKE_CURRENT_LIST_DIR}/../memmap_blit_loader.ld)

pico_enable_stdio_uart(blit-loader 1)
pico_enable_stdio_usb(blit-loader 0)

pico_add_extra_outputs(blit-loader)

pico_set_program_name(blit-loader "32blit loader")
pico_set_program_description(blit-loader "Loads 32blit games")
pico_set_program_url(blit-loader "https://github.com/32blit/32blit-sdk")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/blit-loader.uf2
    DESTINATION bin
)
