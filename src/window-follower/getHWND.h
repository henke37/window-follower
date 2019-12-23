
#ifndef GET_HWND_H
#define GET_HWND_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdbool.h>
#include <obs-module.h>

HWND* GetHWND(obs_source_t*);

bool CanGetHWND(obs_source_t*);

#endif