#include <iostream>
#include <semaphore.h>
#include "collab.hpp"
#include "utils.hpp"
#include <Magick++.h>

using namespace std;
using namespace Magick;

int main(int argc, char **argv) {
    start_game(argv[1], argv[2]);
    vector<char*> mem_names {"frame-sync", "pixel-data", "input-str"};

    sem_t *frame_sync = (sem_t*)open_shared_memory(mem_names[0], sizeof(sem_t));
	GLubyte* pixel_data = (GLubyte*)open_shared_memory(mem_names[1], (1036800)*sizeof(GLubyte*));
	char* input_str = (char*)open_shared_memory(mem_names[2], sizeof(char*));
    int i;

    struct win_dimensions win_d = get_window_dimensions();
    Image im;
    Geometry g(win_d.nwidth*2, win_d.nheight*2);

    while (sem_wait(frame_sync) == 0) {
        glfwPollEvents();
        run_frame();
        if(i%6 == 0) {
            glReadPixels(0, 0, win_d.nwidth, win_d.nheight, GL_BGR, GL_UNSIGNED_BYTE, pixel_data);

            im.read(win_d.nwidth, win_d.nheight, "BGR", CharPixel, pixel_data);
            im.scale(g);
            im.write(0, 0, win_d.nwidth*2, win_d.nheight*2, "BGR", CharPixel, pixel_data);
            i=0;
        }
        if (int(input_str[0]) != 0) {
            send_input(input_str);
            // cout << "collab - " << int(input_str[0]) << endl;
            input_str[0] = 0;
        }
        i++;
    }
    return 0;
}
