set(SOURCES
	audio/audio.cpp
	engine/engine.cpp
	engine/file.cpp
	engine/input.cpp
	engine/output.cpp
	engine/particle.cpp
	engine/profiler.cpp
	engine/running_average.cpp
	engine/timer.cpp
	engine/tweening.cpp
	engine/menu/menu.cpp
	engine/menu/menuItem.cpp
	engine/menu/menuController.cpp
	graphics/blend.cpp
	graphics/color.cpp
	graphics/filter.cpp
	graphics/font.cpp
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
