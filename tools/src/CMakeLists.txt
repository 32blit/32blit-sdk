add_executable(32Blit 32Blit.cpp)

install(TARGETS 32Blit
    RUNTIME DESTINATION bin
)

if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(32Blit ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
endif()