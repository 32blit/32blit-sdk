function(fetch_sdl2_library directory url filename hash)
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${filename})
        message(STATUS "Downloading ${filename}")
        file(DOWNLOAD ${url}${filename} ${CMAKE_CURRENT_LIST_DIR}/${filename}
            SHOW_PROGRESS
            TIMEOUT 120
            EXPECTED_HASH SHA1=${hash}
            TLS_VERIFY ON)
    endif()
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${directory})
        message(STATUS "Extracting ${filename}")
        # tar -xf should work on Windows 10 build 17063 or later (Dec 2017)
        execute_process(COMMAND tar -xf ${filename}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    endif()
    # in future we might be able to use:
    # file(ARCHIVE_EXTRACT INPUT ${filename})
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/${directory}/include
        DESTINATION ${CMAKE_CURRENT_LIST_DIR}/)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/${directory}/lib
        DESTINATION ${CMAKE_CURRENT_LIST_DIR}/)
endfunction(fetch_sdl2_library)


if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include)

    fetch_sdl2_library(
        SDL2-2.24.0
        https://github.com/libsdl-org/SDL/releases/download/release-2.24.0/
        SDL2-devel-2.24.0-VC.zip
        8a54459189e88c30ba024ee5f18ce4b1a5d1d9e7
    )

    fetch_sdl2_library(
        SDL2_image-2.6.2
        https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.2/
        SDL2_image-devel-2.6.2-VC.zip
        4111affcca1f4b41c2f4b4c445ccf06fe081b5e9
    )

    fetch_sdl2_library(
        SDL2_net-2.2.0
        https://github.com/libsdl-org/SDL_net/releases/download/release-2.2.0/
        SDL2_net-devel-2.2.0-VC.zip
        c8ff358a5c8338002b05ab6de7ce91ee1c86bd45
    )

endif()

set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include")

if(CMAKE_GENERATOR_PLATFORM)
    set(SDL2_LIBDIR "${SDL2_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}")
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_LIBDIR "${SDL2_DIR}/lib/x64/")
    else()
        set(SDL2_LIBDIR "${SDL2_DIR}/lib/x86/")
    endif()
endif()

set(SDL2_LIBRARIES "${SDL2_LIBDIR}/SDL2.lib" "${SDL2_LIBDIR}/SDL2main.lib")

set(SDL2_DLL "${SDL2_LIBDIR}/SDL2.dll")
