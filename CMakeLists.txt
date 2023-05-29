cmake_minimum_required (VERSION 2.8.11)
project (gl_test)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(spinningcube_withlight spinningcube_withlight.cpp textfile.c)

target_include_directories(spinningcube_withlight PUBLIC $GLEW_INCLUDE_DIR)

target_link_libraries (spinningcube_withlight PRIVATE GLEW::GLEW glfw GL)
