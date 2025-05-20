set(SOURCES
	audio/audio.cpp
	audio/mp3-stream.cpp
	engine/engine.cpp
	engine/file.cpp
	engine/api.cpp
	engine/input.cpp
	engine/multiplayer.cpp
	engine/particle.cpp
	engine/profiler.cpp
  engine/running_average.cpp
  engine/save.cpp
	engine/timer.cpp
	engine/tweening.cpp
	engine/version.cpp
	graphics/blend.cpp
	graphics/color.cpp
	graphics/filter.cpp
	graphics/font.cpp
	graphics/jpeg.cpp
	graphics/mask.cpp
	graphics/mode7.cpp
	graphics/primitive.cpp
	graphics/sprite.cpp
	graphics/surface.cpp
	graphics/text.cpp
	graphics/tilemap.cpp
	math/geometry.cpp
	math/interpolation.cpp
	types/mat3.cpp
	types/mat4.cpp
	types/vec2.cpp
	types/vec3.cpp
)

add_library(BlitEngine STATIC ${SOURCES})

target_include_directories(BlitEngine
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BlitEngine LINK_PRIVATE MiniMP3)

# version variables
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
if(DEFINED ENV{TRAVIS_TAG})
	# travis release
	set(BUILD_VER $ENV{TRAVIS_TAG})
else()
  execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE BUILD_VER OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE GIT_STATUS
    ERROR_QUIET
  )
  if(NOT ${GIT_STATUS} EQUAL 0)
    set(BUILD_VER "DEV")
  endif()
endif()

configure_file(version_defs.hpp.in version_defs.hpp)
target_include_directories(BlitEngine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  target_compile_options(BlitEngine PRIVATE
    -wd4244 # conversions
  )
endif()
