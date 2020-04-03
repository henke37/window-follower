#include "window-follower.h"

#include <obs-module.h>



void window_follower_onAdd(void *data, calldata_t *cd) {
	window_follower_data_t *filter = data;
	obs_sceneitem_t *item = calldata_ptr(cd, "item");
}

void window_follower_onRemove(void *data, calldata_t *cd) {
	window_follower_data_t *filter = data;
	obs_sceneitem_t *item=calldata_ptr(cd, "item");

	if(item != filter->sceneItem) return;

	if(filter->sceneItem) {
		obs_sceneitem_release(filter->sceneItem);
		obs_source_release(filter->mainSource);

		filter->sceneItem = NULL;
		filter->mainSource = NULL;
		filter->hwndPtr = NULL;
	}
}

void window_follower_signal_lateSetup(window_follower_data_t *filter) {
	signal_handler_t *sigHandler = obs_source_get_signal_handler(obs_scene_get_source(filter->scene));

	signal_handler_connect(sigHandler, "item_add", window_follower_onAdd, filter);
	signal_handler_connect(sigHandler, "item_remove", window_follower_onRemove, filter);
}

void window_follower_signal_setup(window_follower_data_t *filter) {
}

void window_follower_signal_cleanup(window_follower_data_t *filter) {
	if(!filter->scene) return;
	signal_handler_t *sigHandler = obs_source_get_signal_handler(obs_scene_get_source(filter->scene));

	signal_handler_disconnect(sigHandler, "item_add", window_follower_onAdd, filter);
	signal_handler_disconnect(sigHandler, "item_remove", window_follower_onRemove, filter);
}