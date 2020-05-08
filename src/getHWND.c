#include "getHWND.h"
#include "capture-internals.h"

#include <stdlib.h>
#include <obs-internal.h>

HWND *GetHWND(obs_source_t *source) {

	if(!source) return NULL;

	void *data = source->context.data;
	const char *id = obs_source_get_id(source);

	if(!data) return NULL;

	if(strcmp(id, "game_capture") == 0) {
		struct game_capture *gcData = (struct game_capture *)data;
		return &gcData->window;
	}
	if(strcmp(id, "window_capture") == 0) {
		struct window_capture *wcData = (struct window_capture *)data;
		return &wcData->window;
	}

	return NULL;
}

bool CanGetHWND(obs_source_t *source) {
	const char *id = obs_source_get_id(source);

	if(strcmp(id, "game_capture") == 0) return true;
	if(strcmp(id, "window_capture") == 0) return true;

	return false;
}