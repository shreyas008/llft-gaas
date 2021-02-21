#ifndef __UTILS_H_
#define __UTILS_H_

#include<string>
#include <algorithm>
#include <memory>
#include <random>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
extern "C" {
#include <xdo.h>
}

std::string randomId(size_t length);
Window get_window(xdo_t* xwin);
void* create_shared_memory(char* mem_name, int size);
void* open_shared_memory(char* mem_name, int size);
void destroy_shared_memory(std::vector<char*>);

#endif // __UTILS_H_
