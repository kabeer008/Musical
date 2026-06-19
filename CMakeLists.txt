cmake_minimum_required(VERSION 3.20)

project(Musical LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SFML_STATIC_LIBRARIES ON)

find_package(SFML 3 REQUIRED COMPONENTS
    Graphics
    Window
    Audio
    System
)

add_executable(main
    main.cpp
)

target_link_libraries(main PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)