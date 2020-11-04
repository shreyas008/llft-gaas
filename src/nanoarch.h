#ifndef __NANOARCH_H_
#define __NANOARCH_H_

#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include "parse_cl.h"

#include "libretro.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <alsa/asoundlib.h>

struct GameRetro {
	void *handle;
	bool initialized;

	void (*retro_init)(void);
	void (*retro_deinit)(void);
	unsigned (*retro_api_version)(void);
	void (*retro_get_system_info)(struct retro_system_info *info);
	void (*retro_get_system_av_info)(struct retro_system_av_info *info);
	void (*retro_set_controller_port_device)(unsigned port, unsigned device);
	void (*retro_reset)(void);
	void (*retro_run)(void);
	bool (*retro_load_game)(const struct retro_game_info *game);
	void (*retro_unload_game)(void);
};

// Variables from nanoarch
extern struct GameRetro g_retro;
extern int nheight;
extern int nwidth;
extern const int16_t* sound_data;
extern GLFWwindow *g_win;

// Variable from main
extern std::weak_ptr<rtc::DataChannel> weak_dc;

void die(const char *fmt, ...);
void core_load(const char *sofile);
void core_load_game(const char *filename);
void video_render();
void audio_deinit();
void video_deinit();
void core_unload();

#endif // __NANOARCH_H_
