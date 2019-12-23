#include "getHWND.h"
#include "capture-internals.h"

#include <stdlib.h>

HWND* GetHWND(obs_source_t *source) {
	return NULL;
}

bool CanGetHWND(obs_source_t *source) {
	const char * id=obs_source_get_id(source);

	if (strcmp(id, "game_capture") == 0) return true;
	if (strcmp(id, "window_capture") == 0) return true;

	return false;
}