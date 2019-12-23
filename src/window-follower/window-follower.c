#include "getHWND.h"
#include "window-follower.h"

#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("window-follower", "en-US")

static void window_follower_save(void* data, obs_data_t* settings)
{
	window_follower_data_t* filter = data;
}

static void window_follower_update(void* data, obs_data_t* settings)
{
	window_follower_data_t* filter = data;
}

static obs_properties_t* window_follower_properties(void* data)
{
	window_follower_data_t* filter = data;
	obs_properties_t* props = obs_properties_create();


	return props;
}

static void window_follower_tick(void* data, float seconds)
{
	window_follower_data_t* filter = data;
}

static void* window_follower_create(obs_data_t* settings, obs_source_t* context)
{
	window_follower_data_t* filter = bzalloc(sizeof(*filter));

	return filter;
}

static void window_follower_remove(void* data, obs_source_t* source)
{
	window_follower_data_t* filter = data;
	UNUSED_PARAMETER(source);
}

static void window_follower_destroy(void* data)
{
	window_follower_data_t* filter = data;
	bfree(filter);
}

static void window_follower_defaults(obs_data_t* settings)
{
}

static const char* window_follower_get_name(void* unused)
{
	UNUSED_PARAMETER(unused);
	return T_("Window follower");
}

struct obs_source_info window_follower = {
	.id = "window-follower",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = window_follower_get_name,
	.create = window_follower_create,
	.destroy = window_follower_destroy,
	.update = window_follower_update,
	.get_properties = window_follower_properties,
	.get_defaults = window_follower_defaults,
	.video_tick = window_follower_tick,
	.save = window_follower_save,
	.filter_remove = window_follower_remove
};

bool obs_module_load(void) {
	obs_register_source(&window_follower);
	return true;
}