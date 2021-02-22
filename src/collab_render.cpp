#include <iostream>
#include <semaphore.h>
#include "collab.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char **argv) {
    start_game(argv[1], argv[2]);
    vector<char*> mem_names {"frame-sync", "pixel-data", "input-str"};

    sem_t *frame_sync = (sem_t*)open_shared_memory(mem_names[0], sizeof(sem_t));
	GLubyte* pixel_data = (GLubyte*)create_shared_memory(mem_names[1], (1036800)*sizeof(GLubyte*));
    int i;

    struct win_dimensions win_d = get_window_dimensions();
    while (sem_wait(frame_sync) == 0) {
        run_frame();
        if(i%6 == 0) {
            glReadPixels(0, 0, win_d.nwidth, win_d.nheight, GL_BGR, GL_UNSIGNED_BYTE, pixel_data);
            i=0;
        }
        i++;
    }
    return 0;
}
