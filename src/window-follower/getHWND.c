#include "getHWND.h"

#include <stdlib.h>

HWND GetHWND(obs_source_t *source) {
	HWND ret=NULL;
	if(!source) return NULL;

	proc_handler_t *ph = obs_source_get_proc_handler(source);
	calldata_t *cd = calldata_create();
	if(!proc_handler_call(ph, "get_window_handle", cd)) {
		return NULL;
	}

	//failure just returns the already initialized value of null
	calldata_get_ptr(cd, "window", &ret);

	calldata_free(cd);

	return ret;
}

bool CanGetHWND(obs_source_t *source) {
	proc_handler_t *ph = obs_source_get_proc_handler(source);

	return proc_handler_exists(ph, "get_window_handle");
}