#ifndef WINDOW_FOLLOWER_H
#define WINDOW_FOLLOWER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <obs-module.h>

enum PosScaleMode {
	PosScaleNone,
	PosScaleMonitorToScene,
	PosScaleDesktopToScene
};

struct window_follower_data {
	HWND *hwndPtr;
	obs_source_t *filterSource;
	obs_source_t *mainSource;
	obs_scene_t *scene;
	obs_sceneitem_t *sceneItem;

	struct vec2 pos;
	RECT baseWindowDisplayArea;

	bool lateInitializationDone;

	bool stayInBounds;
	bool hideMinimized;

	enum PosScaleMode posScale;

	HMONITOR monitor;

	int sceneBoundsTop;
	int sceneBoundsLeft;
	int sceneBoundsWidth;
	int sceneBoundsHeight;
};

typedef struct window_follower_data window_follower_data_t;

void window_follower_lateInit(window_follower_data_t *filter);

void window_follower_tick(void *data, float seconds);
obs_properties_t *window_follower_properties(void *data);
void window_follower_defaults(obs_data_t *settings);

void setupSceneItem(window_follower_data_t *filter, obs_data_t *settings);
void updateStayInBoundsField(window_follower_data_t *filter, obs_data_t *settings);
bool updatePosScale(window_follower_data_t *filter, obs_data_t *settings);
void updateBounds(window_follower_data_t *filter, obs_data_t *settings);
void updateStayInBoundsField(window_follower_data_t *filter, obs_data_t *settings);
void updateHideMinimizedField(window_follower_data_t *filter, obs_data_t *settings);

void createMonitorProperty(window_follower_data_t *filter, obs_properties_t *props);
void updateMonitor(window_follower_data_t *filter, obs_data_t *settings);

void window_follower_signal_lateSetup(window_follower_data_t *filter);
void window_follower_signal_setup(window_follower_data_t *filter);
void window_follower_signal_cleanup(window_follower_data_t *filter);


#define T_(v) obs_module_text(v)

#endif