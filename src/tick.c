#include "window-follower.h"

#include <util/dstr.h>
#include <obs.h>

void realTick(window_follower_data_t *filter);

void window_follower_tick(void *data, float seconds) {
	window_follower_data_t *filter = data;

	if(!filter->lateInitializationDone) {
		window_follower_lateInit(filter);
	}

	if(filter->sceneItem) {
		//filter->pos.x += 0.1f;
		//if (filter->pos.x > 400) filter->pos.x -= 400;

		if(filter->hwndPtr) {
			if(IsWindow(*filter->hwndPtr)) {
				BOOL iconic = IsIconic(*filter->hwndPtr);
				if(!iconic) {
					realTick(filter);
				}
				if(filter->hideMinimized) {
					obs_sceneitem_set_visible(filter->sceneItem, !iconic);
				}
			}
		}

		obs_sceneitem_set_pos(filter->sceneItem, &filter->pos);
	}
}

void realTick(window_follower_data_t *filter) {
	RECT wndPos;
	GetWindowRect(*filter->hwndPtr, &wndPos);

	float itemWidth = (float)obs_source_get_width(filter->mainSource);
	float itemHeight = (float)obs_source_get_height(filter->mainSource);

	struct vec2 scale;
	obs_sceneitem_get_scale(filter->sceneItem, &scale);
	itemWidth *= scale.x;
	itemHeight *= scale.y;

	if(filter->posScale == PosScaleNone) {
		filter->pos.x = (float)wndPos.left;
		filter->pos.y = (float)wndPos.top;
	} else {
		float adjustedLeft = (float)wndPos.left - filter->baseWindowDisplayArea.left;
		float adjustedTop = (float)wndPos.top - filter->baseWindowDisplayArea.top;

		float xScaler = (float)filter->sceneBoundsWidth / (float)(filter->baseWindowDisplayArea.right - filter->baseWindowDisplayArea.left);
		float yScaler = (float)filter->sceneBoundsHeight / (float)(filter->baseWindowDisplayArea.bottom - filter->baseWindowDisplayArea.top);

		filter->pos.x = adjustedLeft * xScaler + filter->sceneBoundsLeft;
		filter->pos.y = adjustedTop * yScaler + filter->sceneBoundsTop;
	}

	if(filter->stayInBounds) {
		int sceneBoundsRight = filter->sceneBoundsLeft + filter->sceneBoundsWidth;
		int sceneBoundsBottom = filter->sceneBoundsTop + filter->sceneBoundsHeight;

		if(filter->pos.x < 0) filter->pos.x = 0;
		if(filter->pos.x + itemWidth > sceneBoundsRight) filter->pos.x = sceneBoundsRight - itemWidth;

		if(filter->pos.y < 0) filter->pos.y = 0;
		if(filter->pos.y + itemHeight > sceneBoundsBottom) filter->pos.y = sceneBoundsBottom - itemHeight;
	}
}