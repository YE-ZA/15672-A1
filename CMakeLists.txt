cmake_minimum_required(VERSION 3.25)

project(viewer)

file(GLOB SRC_FILES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/*.cpp")

find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/libs/glfw-3.3.9.bin.WIN64/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/glm)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${Vulkan_LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/glfw-3.3.9.bin.WIN64/lib-vc2019/glfw3.lib)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)

if(MSVC)
	set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
endif()
