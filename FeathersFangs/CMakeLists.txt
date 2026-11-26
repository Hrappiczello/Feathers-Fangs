cmake_minimum_required(VERSION 3.15)
project(FeathersFangs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)
add_executable(FeathersFangs ${SOURCES})

target_include_directories(FeathersFangs PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(FeathersFangs PRIVATE glfw glad::glad glm::glm)

# Kopiuje shaders/ i textures/ obok .exe, zeby gra je znalazla
add_custom_command(TARGET FeathersFangs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:FeathersFangs>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:FeathersFangs>/textures)

if(WIN32)
    target_link_libraries(FeathersFangs PRIVATE winmm)
endif()
add_custom_command(TARGET FeathersFangs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/audio $<TARGET_FILE_DIR:FeathersFangs>/audio)
