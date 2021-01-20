#include "window-follower.h"

#include <util/dstr.h>
#include <obs.h>
#include <util/platform.h>

static char *getFriendlyMonitorName(const MONITORINFOEXW *info);

struct monitor_enum_set_monitor_data {
	window_follower_data_t *filter;
	const char *monitorName;
};

BOOL CALLBACK monitor_enum_set_monitor(
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

static BOOL CALLBACK monitor_enum_proplist_add(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT Arg3,
	LPARAM Arg4
) {
	obs_property_t *p = (obs_property_t *)Arg4;
	MONITORINFOEXW info;
	char *friendlyName, *devName;
	info.cbSize = sizeof(info);

	BOOL success = GetMonitorInfoW(Arg1, (LPMONITORINFO)&info);
	if(!success) return TRUE;

	os_wcs_to_utf8_ptr(info.szDevice, sizeof(info.szDevice)/sizeof(WCHAR), &devName);

	friendlyName=getFriendlyMonitorName(&info);
	if(!friendlyName) friendlyName = devName;

	obs_property_list_add_string(p, friendlyName, devName);

	bfree(friendlyName);
	if(friendlyName!=devName) bfree(devName);

	return TRUE;
}

void createMonitorProperty(window_follower_data_t *filter, obs_properties_t *props) {
	obs_property_t *p = obs_properties_add_list(props, "monitor", T_("Monitor"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	EnumDisplayMonitors(NULL, NULL, monitor_enum_proplist_add, (LPARAM)p);
	obs_property_set_long_description(p, T_("Monitor.LongDesc"));
	obs_property_set_modified_callback2(p, monitor_changed, filter);
}

static char *getFriendlyMonitorName(const MONITORINFOEXW *info) {
	UINT32 numPath, numMode;
	DISPLAYCONFIG_PATH_INFO *paths=NULL;
	DISPLAYCONFIG_MODE_INFO *modes=NULL;

	char *ret = NULL;

	if(GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPath, &numMode) != ERROR_SUCCESS) {
		return NULL;
	}

	paths = bmalloc(sizeof(DISPLAYCONFIG_PATH_INFO) * numPath);
	modes = bmalloc(sizeof(DISPLAYCONFIG_MODE_INFO) * numMode);

	if(QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPath,
		paths, &numMode, modes,
		NULL) != ERROR_SUCCESS) {

		goto freeBuffs;
	}

	for(size_t pathIndex = 0; pathIndex < numPath; ++pathIndex) {
		DISPLAYCONFIG_PATH_INFO *path = &paths[pathIndex];

		DISPLAYCONFIG_SOURCE_DEVICE_NAME source = {
			.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
			.header.size = sizeof(source),
			.header.adapterId = path->sourceInfo.adapterId,
			.header.id = path->sourceInfo.id
		};

		if(DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS) continue;
		if(wcscmp(info->szDevice,source.viewGdiDeviceName) != 0) continue;

		DISPLAYCONFIG_TARGET_DEVICE_NAME target = {
			.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
			.header.size = sizeof(target),
			.header.adapterId = path->sourceInfo.adapterId,
			.header.id = path->targetInfo.id
		};

		if(DisplayConfigGetDeviceInfo(&target.header) != ERROR_SUCCESS) continue;

		os_wcs_to_utf8_ptr(target.monitorFriendlyDeviceName, sizeof(target.monitorFriendlyDeviceName) / sizeof(WCHAR), &ret);
		break;
	}

freeBuffs:
	bfree(modes);
	bfree(paths);

	return ret;
}