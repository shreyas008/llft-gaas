#include <iostream>
#include "collab.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char **argv) {
    start_game(argv[1], argv[2]);
    int *test = (int*)open_shared_memory("test_run", sizeof(int));
    while (run_frame()) test[0]++;
    return 0;
}
