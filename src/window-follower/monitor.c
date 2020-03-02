#include "window-follower.h"

#include <obs-module.h>
#include <util/dstr.h>
#include <obs-frontend-api.h>

struct monitor_enum_set_monitor_data {
	window_follower_data_t *filter;
	const char *monitorName;
};

BOOL monitor_enum_set_monitor(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT Arg3,
	LPARAM Arg4
) {
	struct monitor_enum_set_monitor_data *cbData = (struct monitor_enum_set_monitor_data *)Arg4;

	MONITORINFOEXA info;
	info.cbSize = sizeof(info);

	BOOL success = GetMonitorInfoA(Arg1, (LPMONITORINFO)&info);

	if(strcmp(info.szDevice, cbData->monitorName) != 0) return TRUE;

	cbData->filter->monitor = Arg1;
	cbData->filter->baseWindowDisplayArea = *Arg3;
	return FALSE;
}

void updateMonitor(window_follower_data_t *filter, obs_data_t *settings) {
	struct monitor_enum_set_monitor_data cbData = {.filter = filter, .monitorName = obs_data_get_string(settings, "monitor")};
	EnumDisplayMonitors(NULL, NULL, monitor_enum_set_monitor, (LPARAM)&cbData);
}

static bool monitor_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	updatePosScale(filter, settings);

	return false;
}

BOOL monitor_enum_proplist_add(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT Arg3,
	LPARAM Arg4
) {
	obs_property_t *p = (obs_property_t *)Arg4;
	MONITORINFOEXA info;
	info.cbSize = sizeof(info);

	BOOL success = GetMonitorInfoA(Arg1, (LPMONITORINFO)&info);
	if(!success) return TRUE;

	obs_property_list_add_string(p, info.szDevice, info.szDevice);

	return TRUE;
}

void createMonitorProperty(window_follower_data_t *filter, obs_properties_t *props) {
	obs_property_t *p = obs_properties_add_list(props, "monitor", T_("Monitor"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	EnumDisplayMonitors(NULL, NULL, monitor_enum_proplist_add, (LPARAM)p);
	obs_property_set_long_description(p, T_("Monitor.LongDesc"));
	obs_property_set_modified_callback2(p, monitor_changed, filter);
}