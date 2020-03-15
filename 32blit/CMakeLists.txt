set(SOURCES
	audio/audio.cpp
	engine/engine.cpp
	engine/file.cpp
	engine/api.cpp
	engine/input.cpp
	engine/output.cpp
	engine/particle.cpp
	engine/profiler.cpp
	engine/running_average.cpp
	engine/timer.cpp
	engine/tweening.cpp
	engine/version.cpp
  engine/CDCCommandHandler.cpp
  engine/CDCDataStream.cpp
  engine/CDCCommandStream.cpp 
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
	types/map.cpp
	types/mat3.cpp
	types/mat4.cpp
	types/vec2.cpp
	types/vec3.cpp
)

add_library(BlitEngine STATIC ${SOURCES})

target_include_directories(BlitEngine
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# version variables
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
if(DEFINED ENV{TRAVIS_TAG})
	# travis release
	set(BUILD_VER $ENV{TRAVIS_TAG})
else()
	set(BUILD_VER "DEV") # TODO: could parse something from git here
endif()

configure_file(version_defs.hpp.in version_defs.hpp)
target_include_directories(BlitEngine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
