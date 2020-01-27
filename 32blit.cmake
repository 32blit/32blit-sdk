if (NOT DEFINED BLIT_ONCE)
	set(BLIT_ONCE TRUE)

	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_EXTENSIONS OFF)

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit 32blit)

	if (${CMAKE_SYSTEM_NAME} STREQUAL Generic)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-stm32 32blit-stm32)
	else()
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/32blit-sdl 32blit-sdl)
	endif()
endif ()
