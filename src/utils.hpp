#ifndef __UTILS_H_
#define __UTILS_H_

#include<string>
#include <algorithm>
#include <memory>
#include <random>
#include <cstring>
extern "C" {
#include <xdo.h>
}

std::string randomId(size_t length);
Window get_window(xdo_t* xwin);

#endif // __UTILS_H_
