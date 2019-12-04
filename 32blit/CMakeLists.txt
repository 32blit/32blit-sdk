file(GLOB SOURCES */*.cpp)
add_library(BlitEngine STATIC ${SOURCES})

target_include_directories(BlitEngine
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

set(BlitEngine BlitEngine PARENT_SCOPE)
