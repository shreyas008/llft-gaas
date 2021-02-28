#ifndef __NANOARCH_H_
#define __NANOARCH_H_

#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include <zlib.h>
extern "C" {
#include <xdo.h>
}

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <semaphore.h>
#include <unordered_map>
#include "parse_cl.h"
#include "utils.hpp"

#include "libretro.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <alsa/asoundlib.h>

struct Client {
    enum class State {
        Waiting,
        WaitingForVideo,
        WaitingForAudio,
        Ready
    };
    const std::shared_ptr<rtc::PeerConnection> & peerConnection = _peerConnection;
    Client(std::shared_ptr<rtc::PeerConnection> pc) {
        _peerConnection = pc;
    }
    // std::optional<std::shared_ptr<ClientTrackData>> video;
    // std::optional<std::shared_ptr<ClientTrackData>> audio;
    std::shared_ptr<rtc::DataChannel> dataChannel{};
    void setState(State state);
    State getState();

private:
    std::shared_mutex _mutex;
    State state = State::Waiting;
    std::string id;
    std::shared_ptr<rtc::PeerConnection> _peerConnection;
};

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

// Delta Encoding
GLubyte* get_delta(GLubyte* high_res, GLubyte* low_res, unsigned long int video_buffer_size);

#endif // __NANOARCH_H_
