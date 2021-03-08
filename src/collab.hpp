#ifndef __NANOARCH_COLLAB_HPP_
#define __NANOARCH_COLLAB_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
extern "C" {
#include <xdo.h>
}

void start_game ( const char* core_name, const char* game_name );
void run_frame ();
void send_input ( char* msg);
struct win_dimensions get_window_dimensions();

struct win_dimensions {
	int nwidth;
	int nheight;
};
#endif // __NANOARCH_COLLAB_HPP_
