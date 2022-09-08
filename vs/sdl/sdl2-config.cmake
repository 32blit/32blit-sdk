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
        SDL2-2.0.12
        https://www.libsdl.org/release/
        SDL2-devel-2.0.12-VC.zip
        6839b6ec345ef754a6585ab24f04e125e88c3392
    )

    fetch_sdl2_library(
        SDL2_image-2.0.5
        https://www.libsdl.org/projects/SDL_image/release/
        SDL2_image-devel-2.0.5-VC.zip
        137f86474691f4e12e76e07d58d5920c8d844d5b
    )

    fetch_sdl2_library(
        SDL2_net-2.0.1
        https://www.libsdl.org/projects/SDL_net/release/
        SDL2_net-devel-2.0.1-VC.zip
        90adcf4d0d17aed26c1e56ade159d90db4b98b54
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
