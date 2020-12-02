function(fetch_sdl2_library directory url filename hash)
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${filename})
        message(STATUS "Downloading ${url}")
        file(DOWNLOAD ${url}${filename} ${CMAKE_CURRENT_LIST_DIR}/${filename}
            SHOW_PROGRESS
            TIMEOUT 120
            EXPECTED_HASH SHA1=${hash}
            TLS_VERIFY ON)
        message(STATUS "Extracting ${filename}")
    endif()
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${directory})
        # tar -xf should work on Windows 10 build 17063 or later (Dec 2017)
        execute_process(COMMAND tar -xf ${filename}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
        # in future we might be able to use: 
        # file(ARCHIVE_EXTRACT INPUT ${filename})
        file(COPY ${CMAKE_CURRENT_LIST_DIR}/${directory}/include
            DESTINATION ${CMAKE_CURRENT_LIST_DIR}/)
        file(COPY ${CMAKE_CURRENT_LIST_DIR}/${directory}/lib
            DESTINATION ${CMAKE_CURRENT_LIST_DIR}/)
    endif()
endfunction(fetch_sdl2_library)


if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include)

    fetch_sdl2_library(
        SDL2-2.0.12
        https://www.libsdl.org/release/
        SDL2-devel-2.0.12-VC.zip
        bd40f726fbcb1430709eaa671ec6f6b5b36e2e4a
    )

    fetch_sdl2_library(
        SDL2_image-2.0.5
        https://www.libsdl.org/projects/SDL_image/release/
        SDL2_image-devel-2.0.5-VC.zip
        137f86474691f4e12e76e07d58d5920c8d844d5b
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