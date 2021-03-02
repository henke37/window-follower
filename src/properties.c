#include "getHWND.h"
#include "window-follower.h"

#include <util/dstr.h>
#include <obs.h>

static bool source_enum_proplist_add(obs_scene_t *scene,
	obs_sceneitem_t *item, void *p) {
	obs_source_t *source = obs_sceneitem_get_source(item);
	if(!CanGetHWND(source)) return true;

	const char *name = obs_source_get_name(source);

	obs_property_list_add_string((obs_property_t *)p, name, name);
	UNUSED_PARAMETER(scene);
	return true;
}

void setupSceneItem(window_follower_data_t *filter, obs_data_t *settings) {

	const char *sourceName = obs_data_get_string(settings, "sourceId");

	if(filter->sceneItem) {
		obs_sceneitem_release(filter->sceneItem);
	}

	filter->sceneItem = obs_scene_find_source(filter->scene, sourceName);

	if(filter->sceneItem) {
		obs_sceneitem_addref(filter->sceneItem);
	}

	if(filter->mainSource) {
		obs_source_release(filter->mainSource);
		window_follower_signal_cleanup_mainsource(filter);
	}

	obs_source_t *source = obs_sceneitem_get_source(filter->sceneItem);

	if(source) {
		obs_source_addref(source);
		filter->mainSource = source;
		window_follower_signal_setup_mainsource(filter);
	}

	filter->window = GetHWND(source);
}

static bool source_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	setupSceneItem(filter, settings);

	return false;//no need to rebuild the properties
}

enum PosScaleMode parsePosScale(const char *posScaleName) {
	if(strcmp(posScaleName, "None") == 0) return PosScaleNone;
	if(strcmp(posScaleName, "MonitorToScene") == 0) return PosScaleMonitorToScene;
	if(strcmp(posScaleName, "DesktopToScene") == 0) return PosScaleDesktopToScene;

	return PosScaleNone;
}

bool posScaleUsesMonitor(enum PosScaleMode posScale) {
	switch(posScale) {
		case PosScaleMonitorToScene: return true;
		default: return false;
	}
}

bool updatePosScale(window_follower_data_t *filter, obs_data_t *settings) {
	enum PosScaleMode newPosScale = parsePosScale(obs_data_get_string(settings, "posScale"));

	enum PosScaleMode oldPosScale = filter->posScale;
	filter->posScale = newPosScale;

	filter->monitor = NULL;
	if(posScaleUsesMonitor(newPosScale)) {
		updateMonitor(filter, settings);
	}
	if(filter->monitor == NULL) {
		filter->baseWindowDisplayArea.left = GetSystemMetrics(SM_YVIRTUALSCREEN);
		filter->baseWindowDisplayArea.top = GetSystemMetrics(SM_XVIRTUALSCREEN);
		filter->baseWindowDisplayArea.right = filter->baseWindowDisplayArea.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
		filter->baseWindowDisplayArea.bottom = filter->baseWindowDisplayArea.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}

	//we need to rebuild the properties if we switch between posscale modes that use and doesn't use a monitor
	return posScaleUsesMonitor(newPosScale) != posScaleUsesMonitor(oldPosScale);
}

static bool posScale_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	return updatePosScale(filter, settings);
}

void updateBounds(window_follower_data_t *filter, obs_data_t *settings) {
	filter->sceneBoundsLeft = (int)obs_data_get_int(settings, "boundsLeft");
	filter->sceneBoundsTop = (int)obs_data_get_int(settings, "boundsTop");
	filter->sceneBoundsWidth = (int)obs_data_get_int(settings, "boundsWidth");
	filter->sceneBoundsHeight = (int)obs_data_get_int(settings, "boundsHeight");
}

bool bounds_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	updateBounds(filter, settings);

	return false;
}

void updateStayInBoundsField(window_follower_data_t *filter, obs_data_t *settings) {
	filter->stayInBounds = obs_data_get_bool(settings, "stayInBounds");
}

static bool stayInBounds_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	updateStayInBoundsField(filter, settings);

	return false;
}

void updateHideMinimizedField(window_follower_data_t *filter, obs_data_t *settings) {
	filter->hideMinimized = obs_data_get_bool(settings, "hideMinimized");
}

static bool hideMinimized_changed(void *data, obs_properties_t *props,
	obs_property_t *p, obs_data_t *settings) {
	window_follower_data_t *filter = data;

	updateHideMinimizedField(filter, settings);

	return false;
}

obs_properties_t *window_follower_properties(void *data) {
	window_follower_data_t *filter = data;
	obs_properties_t *props = obs_properties_create();

	struct obs_video_info vidInfo;
	obs_get_video_info(&vidInfo);

	if(!filter->scene) return props;

	{
		obs_property_t *p = obs_properties_add_list(props, "sourceId", T_("SourceId"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p, T_("SourceId.None"), "sourceIdNone");
		obs_property_set_long_description(p, T_("SourceId.LongDesc"));
		obs_property_set_modified_callback2(p, source_changed, filter);

		// A list of sources
		obs_scene_enum_items(filter->scene, source_enum_proplist_add, (void *)p);
		obs_property_set_modified_callback2(p, source_changed, filter);
	}

	{
		obs_property_t *p = obs_properties_add_list(props, "posScale", T_("ScalePos"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p, T_("ScalePos.None"), "None");
		obs_property_list_add_string(p, T_("ScalePos.MonitorToScene"), "MonitorToScene");
		obs_property_list_add_string(p, T_("ScalePos.DesktopToScene"), "DesktopToScene");
		obs_property_set_long_description(p, T_("ScalePos.LongDesc"));
		obs_property_set_modified_callback2(p, posScale_changed, filter);
	}

	if(posScaleUsesMonitor(filter->posScale)) {
		createMonitorProperty(filter, props);
	}

	{
		obs_properties_t *boundsProps = obs_properties_create();
		{
			obs_property_t *p = obs_properties_add_int(boundsProps, "boundsLeft", T_("Bounds.Left"), 0, vidInfo.base_width, 32);
			obs_property_set_long_description(p, T_("Bounds.Left.LongDesc"));
			obs_property_set_modified_callback2(p, bounds_changed, filter);
		}
		{
			obs_property_t *p = obs_properties_add_int(boundsProps, "boundsWidth", T_("Bounds.Width"), 0, vidInfo.base_width, 32);
			obs_property_set_long_description(p, T_("Bounds.Width.LongDesc"));
			obs_property_set_modified_callback2(p, bounds_changed, filter);
		}
		{
			obs_property_t *p = obs_properties_add_int(boundsProps, "boundsTop", T_("Bounds.Top"), 0, vidInfo.base_height, 32);
			obs_property_set_long_description(p, T_("Bounds.Top.LongDesc"));
			obs_property_set_modified_callback2(p, bounds_changed, filter);
		}
		{
			obs_property_t *p = obs_properties_add_int(boundsProps, "boundsHeight", T_("Bounds.Height"), 0, vidInfo.base_height, 32);
			obs_property_set_long_description(p, T_("Bounds.Height.LongDesc"));
			obs_property_set_modified_callback2(p, bounds_changed, filter);
		}

		{
			obs_property_t *p = obs_properties_add_bool(boundsProps, "stayInBounds", T_("StayInBounds"));
			obs_property_set_long_description(p, T_("StayInBounds.LongDesc"));
			obs_property_set_modified_callback2(p, stayInBounds_changed, filter);
		}

		obs_properties_add_group(props, "bounds", T_("Bounds.Group"), OBS_GROUP_NORMAL, boundsProps);
	}

	{
		obs_property_t *p = obs_properties_add_bool(props, "hideMinimized", T_("HideMinimized"));
		obs_property_set_modified_callback2(p, hideMinimized_changed, filter);
	}

	return props;
}

void window_follower_defaults(obs_data_t *settings) {
	struct obs_video_info vidInfo;
	obs_get_video_info(&vidInfo);

	obs_data_set_default_string(settings, "posScale", "DesktopToScene");
	obs_data_set_default_bool(settings, "stayInBounds", false);

	obs_data_set_default_int(settings, "boundsLeft", 0);
	obs_data_set_default_int(settings, "boundsWidth", vidInfo.base_width);
	obs_data_set_default_int(settings, "boundsTop", 0);
	obs_data_set_default_int(settings, "boundsHeight", vidInfo.base_height);

	obs_data_set_default_bool(settings, "hideMinimized", true);
}