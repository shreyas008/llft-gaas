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
    int counter;

    while (sem_wait(frame_sync) == 0) counter = run_frame(counter, pixel_data);
    return 0;
}
