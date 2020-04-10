# window-follower
OBS plugin  that makes scene items follow the movement of the window the source is capturing from

## Usage
Add the plugin to the scene with the scene item that you want to work with. Set the scene item to work with.

## Additional options
* Stay in bounds: Ensures that the scene item doesn't leave the scene if the window moves out of bounds.
* Scale mode: How to translate window coordinates to scene coordinates. Only really matters in multimonitor setups.
* Monitor: The monitor to use when in monitor to scene scale mode.

## Limitations
* The scene item should not be rotated or skewed.

## Build
### Windows
First follow build procedures for [obs-studio](https://github.com/obsproject/obs-studio/wiki/install-instructions#windows-build-directions).

- Building obs-studio will produce an `obs.lib` file, generated inside the build directories - e.g. `obs-studio/build/libobs/debug/obs.lib`

- Assuming you have cmake, prior to first configure, add the following entries:

| Entry name         | Type     | Value (e.g.)                                         |
|--------------------|----------|------------------------------------------------------|
| LIBOBS_LIB         | FILEPATH | /obs-studio/path/to/obs.lib                          |
| LIBOBS_INCLUDE_DIR | PATH     | /obs-studio/libobs                                   |
| OBS_FRONTEND_LIB   | FILEPATH | /obs-studio/UI/obs-frontend-api/obs-frontend-api.lib |

- Click 'Configure', which will run
- Click 'Generate'

This should produce the desired development environment, which after building, shall produce the plugin dll file.
