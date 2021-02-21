#ifndef __NANOARCH_COLLAB_HPP_
#define __NANOARCH_COLLAB_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void start_game ( const char* core_name, const char* game_name );
int run_frame (int counter, GLubyte* pixel_data);
void send_input ( char* msg);

#endif // __NANOARCH_COLLAB_HPP_
