#ifndef CAPTURE_INTERNALS_H
#define CAPTURE_INTERNALS_H

#include <stdlib.h>
#include <util/dstr.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <obs-module.h>
#include <ipc-util/pipe.h>

struct dc_capture {
	gs_texture_t *texture;
	bool texture_written;
	int x, y;
	uint32_t width;
	uint32_t height;

	bool compatibility;
	HDC hdc;
	HBITMAP bmp, old_bmp;
	BYTE *bits;

	bool capture_cursor;
	bool cursor_captured;
	bool cursor_hidden;
	CURSORINFO ci;

	bool valid;
};

struct winrt_exports {
	bool *(*winrt_capture_supported)();
	struct winrt_capture *(*winrt_capture_init)(bool cursor, HWND window,
		bool client_area);
	void (*winrt_capture_free)(struct winrt_capture *capture);
	void (*winrt_capture_render)(struct winrt_capture *capture,
		gs_effect_t *effect);
	uint32_t(*winrt_capture_width)(const struct winrt_capture *capture);
	uint32_t(*winrt_capture_height)(const struct winrt_capture *capture);
};

enum window_capture_method {
	METHOD_AUTO,
	METHOD_BITBLT,
	METHOD_WGC,
};

struct window_capture {
	obs_source_t *source;

	char *title;
	char *class;
	char *executable;
	enum window_capture_method method;
	enum window_priority priority;
	bool cursor;
	bool compatibility;
	bool client_area;
	bool use_wildcards; /* TODO */

	struct dc_capture capture;

	bool wgc_supported;
	void *winrt_module;
	struct winrt_exports exports;
	struct winrt_capture *capture_winrt;

	float resize_timer;
	float check_window_timer;
	float cursor_check_time;

	HWND window;
	RECT last_rect;
};

struct cached_cursor {
	gs_texture_t *texture;
	uint32_t cx;
	uint32_t cy;
};

struct cursor_data {
	gs_texture_t *texture;
	HCURSOR current_cursor;
	POINT cursor_pos;
	long x_hotspot;
	long y_hotspot;
	bool visible;

	uint32_t last_cx;
	uint32_t last_cy;

	DARRAY(struct cached_cursor) cached_textures;
};


enum capture_mode {
	CAPTURE_MODE_ANY,
	CAPTURE_MODE_WINDOW,
	CAPTURE_MODE_HOTKEY
};

enum hook_rate {
	HOOK_RATE_SLOW,
	HOOK_RATE_NORMAL,
	HOOK_RATE_FAST,
	HOOK_RATE_FASTEST
};

struct game_capture_config {
	char *title;
	char *class;
	char *executable;
	enum window_priority priority;
	enum capture_mode mode;
	uint32_t scale_cx;
	uint32_t scale_cy;
	bool cursor;
	bool force_shmem;
	bool force_scaling;
	bool allow_transparency;
	bool limit_framerate;
	bool capture_overlays;
	bool anticheat_hook;
	enum hook_rate hook_rate;
};

struct game_capture {
	obs_source_t *source;

	struct cursor_data cursor_data;
	HANDLE injector_process;
	uint32_t cx;
	uint32_t cy;
	uint32_t pitch;
	DWORD process_id;
	DWORD thread_id;
	HWND next_window;
	HWND window;
	float retry_time;
	float fps_reset_time;
	float retry_interval;
	struct dstr title;
	struct dstr class;
	struct dstr executable;
	enum window_priority priority;
	obs_hotkey_pair_id hotkey_pair;
	volatile long hotkey_window;
	volatile bool deactivate_hook;
	volatile bool activate_hook_now;
	bool wait_for_target_startup;
	bool showing;
	bool active;
	bool capturing;
	bool activate_hook;
	bool process_is_64bit;
	bool error_acquiring;
	bool dwm_capture;
	bool initial_config;
	bool convert_16bit;
	bool is_app;
	bool cursor_hidden;

	struct game_capture_config config;

	ipc_pipe_server_t pipe;
	gs_texture_t *texture;
	struct hook_info *global_hook_info;
	HANDLE keepalive_mutex;
	HANDLE hook_init;
	HANDLE hook_restart;
	HANDLE hook_stop;
	HANDLE hook_ready;
	HANDLE hook_exit;
	HANDLE hook_data_map;
	HANDLE global_hook_info_map;
	HANDLE target_process;
	HANDLE texture_mutexes[2];
	wchar_t *app_sid;
	int retrying;
	float cursor_check_time;

	union {
		struct {
			struct shmem_data *shmem_data;
			uint8_t *texture_buffers[2];
		};

		struct shtex_data *shtex_data;
		void *data;
	};

	void (*copy_texture)(struct game_capture *);
};

#endif