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