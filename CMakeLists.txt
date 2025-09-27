cmake_minimum_required(VERSION 3.16)
project(display_tool LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_NUKLEAR "Enable Nuklear UI integration if available" ON)

add_executable(display_tool)
target_sources(display_tool PRIVATE src/main.cpp)

# Dependencies: GLFW and OpenGL
find_package(OpenGL REQUIRED)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GLFW3 QUIET glfw3)
endif()

if(GLFW3_FOUND)
    message(STATUS "Found GLFW via pkg-config")
    target_include_directories(display_tool PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_link_directories(display_tool PRIVATE ${GLFW3_LIBRARY_DIRS})
    target_link_libraries(display_tool PRIVATE ${GLFW3_LIBRARIES})
else()
    find_package(glfw3 REQUIRED)
    target_link_libraries(display_tool PRIVATE glfw)
endif()

find_package(GLEW  REQUIRED)
target_link_libraries(display_tool PRIVATE OpenGL::GL)

# Try to add Nuklear if requested and available
if(ENABLE_NUKLEAR)
    # Prefer vendored headers if present
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nuklear/nuklear.h)
        set(NUKLEAR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nuklear)
    else()
        find_path(NUKLEAR_INCLUDE_DIR nuklear.h PATH_SUFFIXES include)
    endif()
    if(NUKLEAR_INCLUDE_DIR)
        target_compile_definitions(display_tool PRIVATE USE_NUKLEAR=1)
        target_include_directories(display_tool PRIVATE ${NUKLEAR_INCLUDE_DIR})
        message(STATUS "Nuklear found: ${NUKLEAR_INCLUDE_DIR}")
    else()
        message(STATUS "Nuklear not found; building without UI (vendor into third_party/nuklear or disable ENABLE_NUKLEAR)")
    endif()
endif()

target_include_directories(display_tool PRIVATE src)

if(MSVC)
    target_compile_options(display_tool PRIVATE /W4)
else()
    target_compile_options(display_tool PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(SRC examples/glfw_window_2d.cpp examples/glfw_initializer.cpp )

# Examples
add_executable(image_2d examples/image_2d.cpp ${SRC})
target_link_libraries(image_2d PRIVATE OpenGL::GL)
if(GLFW3_FOUND)
    target_include_directories(image_2d PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_link_directories(image_2d PRIVATE ${GLFW3_LIBRARY_DIRS})
    target_link_libraries(image_2d PRIVATE  GLEW::GLEW ${GLFW3_LIBRARIES})
else()
    target_link_libraries(image_2d PRIVATE GLEW::GLEW glfw)
endif()
target_include_directories(image_2d PRIVATE src)
if(ENABLE_NUKLEAR AND NUKLEAR_INCLUDE_DIR)
    target_compile_definitions(image_2d PRIVATE USE_NUKLEAR=1)
    target_include_directories(image_2d PRIVATE ${NUKLEAR_INCLUDE_DIR})
endif()

add_executable(mesh_3d examples/mesh_3d.cpp)
target_link_libraries(mesh_3d PRIVATE OpenGL::GL)
if(GLFW3_FOUND)
    target_include_directories(mesh_3d PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_link_directories(mesh_3d PRIVATE ${GLFW3_LIBRARY_DIRS})
    target_link_libraries(mesh_3d PRIVATE ${GLFW3_LIBRARIES})
else()
    target_link_libraries(mesh_3d PRIVATE glfw)
endif()
target_include_directories(mesh_3d PRIVATE src)
if(ENABLE_NUKLEAR AND NUKLEAR_INCLUDE_DIR)
    target_compile_definitions(mesh_3d PRIVATE USE_NUKLEAR=1)
    target_include_directories(mesh_3d PRIVATE ${NUKLEAR_INCLUDE_DIR})
endif()


