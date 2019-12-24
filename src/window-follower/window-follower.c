#include "getHWND.h"
#include "window-follower.h"

#include <obs-module.h>
#include <util/dstr.h>
#include <obs-frontend-api.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("window-follower", "en-US")

void updateMonitorField(window_follower_data_t* filter, obs_data_t* settings);

static void window_follower_save(void* data, obs_data_t* settings)
{
	window_follower_data_t* filter = data;
}

static void window_follower_updateSettings(void* data, obs_data_t* settings)
{
	window_follower_data_t* filter = data;

}

static bool source_enum_proplist_add(obs_scene_t* scene,
	obs_sceneitem_t* item, void* p)
{
	obs_source_t* source = obs_sceneitem_get_source(item);
	if(!CanGetHWND(source)) return true;

	const char* name = obs_source_get_name(source);
	
	obs_property_list_add_string((obs_property_t*)p, name, name);
	UNUSED_PARAMETER(scene);
	return true;
}

void setupSceneItem(window_follower_data_t* filter, obs_data_t* settings) {

	const char* sourceName = obs_data_get_string(settings, "sourceId");

	if (filter->sceneItem) {
		obs_sceneitem_release(filter->sceneItem);
	}

	filter->sceneItem = obs_scene_find_source(filter->scene, sourceName);

	if (filter->sceneItem) {
		obs_sceneitem_addref(filter->sceneItem);
	}

	if (filter->mainSource) {
		obs_source_release(filter->mainSource);
	}

	obs_source_t* source = obs_sceneitem_get_source(filter->sceneItem);

	if (source) {
		obs_source_addref(source);
		filter->mainSource = source;
	}

	filter->hwndPtr = GetHWND(source);
}

static bool source_changed(void* data, obs_properties_t* props,
	obs_property_t* p, obs_data_t* settings) {
	window_follower_data_t* filter = data;

	setupSceneItem(filter, settings);

	return false;//no need to rebuild the properties
}

enum PosScaleMode parsePosScale(const char* posScaleName) {
	if (strcmp(posScaleName, "None") == 0) return PosScaleNone;
	if (strcmp(posScaleName, "MonitorToCanvas") == 0) return PosScaleMonitorToCanvas;
	if (strcmp(posScaleName, "DesktopToCanvas") == 0) return PosScaleDesktopToCanvas;
	if (strcmp(posScaleName, "MonitorToScene") == 0) return PosScaleMonitorToScene;
	if (strcmp(posScaleName, "DesktopToScene") == 0) return PosScaleDesktopToScene;

	return PosScaleNone;
}

bool posScaleUsesMonitor(enum PosScaleMode posScale) {
	switch (posScale) {
	case PosScaleMonitorToCanvas:
	case PosScaleMonitorToScene: return true;
	default: return false;
	}
}

bool updatePosScale(window_follower_data_t* filter, obs_data_t* settings) {
	enum PosScaleMode newPosScale = parsePosScale(obs_data_get_string(settings, "posScale"));

	if (newPosScale == filter->posScale) return false;

	enum PosScaleMode oldPosScale = filter->posScale;
	filter->posScale = newPosScale;

	if (posScaleUsesMonitor(newPosScale)) {
		updateMonitorField(filter, settings);
	}
	else {
		filter->monitor = NULL;
		filter->baseWindowDisplayArea.left = GetSystemMetrics(SM_YVIRTUALSCREEN);
		filter->baseWindowDisplayArea.top = GetSystemMetrics(SM_XVIRTUALSCREEN);
		filter->baseWindowDisplayArea.right = filter->baseWindowDisplayArea.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
		filter->baseWindowDisplayArea.bottom = filter->baseWindowDisplayArea.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}

	//we need to rebuild the properties if we switch between posscale modes that use and doesn't use a monitor
	return posScaleUsesMonitor(newPosScale) != posScaleUsesMonitor(oldPosScale);
}

static bool posScale_changed(void* data, obs_properties_t* props,
	obs_property_t* p, obs_data_t* settings) {
	window_follower_data_t* filter = data;

	return updatePosScale(filter, settings);
}

BOOL monitor_enum_proplist_add(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT Arg3,
	LPARAM Arg4
) {
	obs_property_t* p = (obs_property_t * )Arg4;
	MONITORINFOEXA info;
	info.cbSize = sizeof(info);

	BOOL success=GetMonitorInfoA(Arg1, (LPMONITORINFO) &info);
	if (!success) return TRUE;

	obs_property_list_add_string(p, info.szDevice, info.szDevice);

	return TRUE;
}

struct monitor_enum_set_monitor_data {
	window_follower_data_t* filter;
	const char *monitorName;
};

BOOL monitor_enum_set_monitor(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT Arg3,
	LPARAM Arg4
) {
	struct monitor_enum_set_monitor_data* cbData = (struct monitor_enum_set_monitor_data* )Arg4;

	MONITORINFOEXA info;
	info.cbSize = sizeof(info);

	BOOL success = GetMonitorInfoA(Arg1, (LPMONITORINFO)&info);

	if (strcmp(info.szDevice, cbData->monitorName) != 0) return TRUE;

	cbData->filter->monitor = Arg1;
	cbData->filter->baseWindowDisplayArea = *Arg3;
	return FALSE;
}

void updateMonitorField(window_follower_data_t* filter, obs_data_t* settings) {
	filter->monitor = NULL;
	struct monitor_enum_set_monitor_data cbData = { .filter = filter, .monitorName = obs_data_get_string(settings, "monitor") };
	EnumDisplayMonitors(NULL, NULL, monitor_enum_set_monitor, (LPARAM)&cbData);

}

static bool monitor_changed(void* data, obs_properties_t* props,
	obs_property_t* p, obs_data_t* settings) {
	window_follower_data_t* filter = data;

	updateMonitorField(filter, settings);

	return false;
}

static obs_properties_t* window_follower_properties(void* data)
{
	window_follower_data_t* filter = data;
	obs_properties_t* props = obs_properties_create();

	if (!filter->scene) return props;

	{
		obs_property_t* p = obs_properties_add_list(props, "sourceId", T_("SourceId"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p, T_("SourceId.None"), "sourceIdNone");
		obs_property_set_modified_callback2(p, source_changed, filter);

		// A list of sources
		obs_scene_enum_items(filter->scene, source_enum_proplist_add, (void*)p);
		obs_property_set_modified_callback2(p, source_changed, filter);
	}

	{
		obs_property_t* p = obs_properties_add_list(props, "posScale", T_("ScalePos"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p, T_("ScalePos.None"), "None");
		obs_property_list_add_string(p, T_("ScalePos.MonitorToCanvas"), "MonitorToCanvas");
		obs_property_list_add_string(p, T_("ScalePos.DesktopToCanvas"), "DesktopToCanvas");
		obs_property_list_add_string(p, T_("ScalePos.MonitorToScene"), "MonitorToScene");
		obs_property_list_add_string(p, T_("ScalePos.DesktopToScene"), "DesktopToScene");
		obs_property_set_modified_callback2(p, posScale_changed, filter);
	}

	if (posScaleUsesMonitor(filter->posScale)) {
		obs_property_t* p = obs_properties_add_list(props, "monitor", T_("Monitor"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		EnumDisplayMonitors(NULL, NULL, monitor_enum_proplist_add, (LPARAM)p);
		obs_property_set_modified_callback2(p, monitor_changed, filter);
	}

	return props;
}

static void* window_follower_create(obs_data_t* settings, obs_source_t* context)
{
	window_follower_data_t* filter = bzalloc(sizeof(*filter));
	filter->filterSource = context;

	return filter;
}

static void window_follower_lateInit(window_follower_data_t *filter) {
	obs_source_t* sceneSource = obs_filter_get_parent(filter->filterSource);
	filter->scene = obs_scene_from_source(sceneSource);

	obs_scene_addref(filter->scene);

	filter->lateInitializationDone = true;
}

static void window_follower_remove(void* data, obs_source_t* source)
{
	window_follower_data_t* filter = data;

	if (filter->mainSource) {
		obs_source_release(filter->mainSource);
		filter->mainSource = NULL;
	}

	if (filter->sceneItem) {
		obs_sceneitem_release(filter->sceneItem);
		filter->sceneItem = NULL;
	}

	if (filter->scene) {
		obs_scene_release(filter->scene);
		filter->scene = NULL;
	}

	UNUSED_PARAMETER(source);
}

static void window_follower_destroy(void* data)
{
	window_follower_data_t* filter = data;
	bfree(filter);
}

static void window_follower_tick(void* data, float seconds)
{
	window_follower_data_t* filter = data;

	if (!filter->lateInitializationDone) {
		window_follower_lateInit(filter);
	}

	if (filter->sceneItem) {
		//filter->pos.x += 0.1f;
		//if (filter->pos.x > 400) filter->pos.x -= 400;

		if (filter->hwndPtr) {
			HWND hwnd = *filter->hwndPtr;

			if (IsWindow(hwnd)) {
				RECT wndPos;
				if (GetWindowRect(hwnd, &wndPos)) {
					filter->pos.x = (float)wndPos.left;
					filter->pos.y = (float)wndPos.top;
				}
			}
		}

		obs_sceneitem_set_pos(filter->sceneItem, &filter->pos);
	}
}

static void window_follower_defaults(obs_data_t* settings)
{
	obs_data_set_default_string(settings, "posScale", "None");
}

static void window_follower_load(void* data, obs_data_t* settings)
{
	window_follower_data_t* filter = data;

	setupSceneItem(filter, settings);
	updatePosScale(filter, settings);
	updateMonitorField(filter, settings);
}

static void window_follower_show(void* data) {
	window_follower_data_t* filter = data;

	obs_data_t* settings = obs_source_get_settings(filter->filterSource);

	setupSceneItem(filter, settings);
	updatePosScale(filter, settings);
	updateMonitorField(filter, settings);

	obs_data_release(settings);
}

static const char* window_follower_get_name(void* unused)
{
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
	obs_register_source(&window_follower);
	return true;
}