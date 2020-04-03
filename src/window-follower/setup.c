#include "window-follower.h"

#include <obs-module.h>
#include <util/dstr.h>
#include <obs-frontend-api.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("window-follower", "en-US")

static void window_follower_save(void *data, obs_data_t *settings) {
	window_follower_data_t *filter = data;
}

static void window_follower_updateSettings(void *data, obs_data_t *settings) {
	window_follower_data_t *filter = data;

}

static void *window_follower_create(obs_data_t *settings, obs_source_t *context) {
	window_follower_data_t *filter = bzalloc(sizeof(*filter));
	filter->filterSource = context;
	window_follower_signal_setup(filter);

	return filter;
}

void window_follower_lateInit(window_follower_data_t *filter) {
	obs_source_t *sceneSource = obs_filter_get_parent(filter->filterSource);
	filter->scene = obs_scene_from_source(sceneSource);

	window_follower_signal_lateSetup(filter);

	filter->lateInitializationDone = true;
}

static void window_follower_remove(void *data, obs_source_t *source) {
	window_follower_data_t *filter = data;

	if(filter->mainSource) {
		obs_source_release(filter->mainSource);
		filter->mainSource = NULL;
	}

	if(filter->sceneItem) {
		obs_sceneitem_release(filter->sceneItem);
		filter->sceneItem = NULL;
	}

	window_follower_signal_cleanup(filter);

	UNUSED_PARAMETER(source);
}

static void window_follower_destroy(void *data) {
	window_follower_data_t *filter = data;
	bfree(filter);
}

static void window_follower_defaults(obs_data_t *settings) {
	struct obs_video_info vidInfo;
	obs_get_video_info(&vidInfo);

	obs_data_set_default_string(settings, "posScale", "DesktopToScene");
	obs_data_set_default_bool(settings, "stayInBounds", false);

	obs_data_set_default_int(settings, "bounds.Left", 0);
	obs_data_set_default_int(settings, "bounds.Width", vidInfo.base_width);
	obs_data_set_default_int(settings, "bounds.Top", 0);
	obs_data_set_default_int(settings, "bounds.Height", vidInfo.base_height);
}

static void window_follower_load(void *data, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	setupSceneItem(filter, settings);
	updateStayInBoundsField(filter, settings);
	updatePosScale(filter, settings);
	updateBounds(filter, settings);
}

static void window_follower_show(void *data) {
	window_follower_data_t *filter = data;

	obs_data_t *settings = obs_source_get_settings(filter->filterSource);

	setupSceneItem(filter, settings);
	updateStayInBoundsField(filter, settings);
	updatePosScale(filter, settings);
	updateBounds(filter, settings);

	obs_data_release(settings);
}

static const char *window_follower_get_name(void *unused) {
	UNUSED_PARAMETER(unused);
	return T_("WindowFollower");
}

struct obs_source_info window_follower = {
	.id = "window-follower",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = window_follower_get_name,
	.create = window_follower_create,
	.destroy = window_follower_destroy,
	.update = window_follower_updateSettings,
	.get_properties = window_follower_properties,
	.get_defaults = window_follower_defaults,
	.video_tick = window_follower_tick,
	.save = window_follower_save,
	.load = window_follower_load,
	.filter_remove = window_follower_remove,
	.show = window_follower_show
};

bool obs_module_load(void) {
	if(obs_get_version() != LIBOBS_API_VER) {
		return false;
	}

	obs_register_source(&window_follower);
	return true;
}

OBS_MODULE_AUTHOR("Henke37");

const char *obs_module_name(void) {
	return "Window-follower";
}

const char *obs_module_description(void) {
	return "Makes scene items follow the movement of the window the source is capturing from";
}