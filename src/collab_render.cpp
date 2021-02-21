#include "collab.hpp"
#include <iostream>

int main(int argc, char **argv) {
    start_game(argv[1], argv[2]);
    while (run_frame()) ;
    return 0;
}
