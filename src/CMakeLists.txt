cmake_minimum_required(VERSION 3.7)

add_executable(game-server main.cpp nanoarch.cpp nanoarch.h parse_cl.cpp parse_cl.h)

set_target_properties(game-server PROPERTIES
	CXX_STANDARD 17
	OUTPUT_NAME game-server)
target_link_libraries(game-server datachannel dl z glfw GLEW GL asound xdo)
