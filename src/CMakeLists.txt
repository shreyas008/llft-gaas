cmake_minimum_required(VERSION 3.7)
project(llft-gaas LANGUAGES CXX)

add_executable(game-server
  nanoarch.h parse_cl.h utils.hpp
  main.cpp nanoarch.cpp parse_cl.cpp utils.cpp)
set_target_properties(game-server PROPERTIES
	CXX_STANDARD 17
	OUTPUT_NAME game-server)
target_link_libraries(game-server datachannel dl z glfw GLEW GL asound xdo rt pthread)

add_executable(collab-render
  collab_render.cpp utils.cpp
  collab.hpp utils.hpp)
set_target_properties(collab-render PROPERTIES
	CXX_STANDARD 17
	OUTPUT_NAME collab-render)
target_link_directories(collab-render PUBLIC lib)
target_link_libraries(collab-render collab dl glfw GLEW GL xdo rt pthread)
