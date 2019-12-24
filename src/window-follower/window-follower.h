#ifndef WINDOW_FOLLOWER_H
#define WINDOW_FOLLOWER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <obs-module.h>

struct window_follower_data {
	HWND *hwndPtr;
	obs_source_t *filterSource;
	obs_source_t *mainSource;
	obs_scene_t *scene;
	obs_sceneitem_t *sceneItem;

	struct vec2 pos;

	bool lateInitializationDone;
};

typedef struct window_follower_data window_follower_data_t;


#define T_(v) obs_module_text(v)

#endif