#ifndef WINDOW_FOLLOWER_H
#define WINDOW_FOLLOWER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct window_follower_data {
	HWND* hwndPtr;
};

typedef struct window_follower_data window_follower_data_t;


#define T_(v)               obs_module_text(v)

#endif