add_compile_definitions(ecs_ftime_t=double)

add_subdirectory(flecs)
add_subdirectory(SDL)
add_subdirectory(glm)
add_subdirectory(glad/cmake)

set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "Build the Box2D unit tests")
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "Build the Box2D testbed")
add_subdirectory(box2d)

add_subdirectory(freetype) 