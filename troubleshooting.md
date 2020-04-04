# Troubleshooting


## I'm not seeing any filter settings

You added the filter to the source. Add it to the scene instead.
This is because filters on sources apply to all instances of the source on all scenes.


## I updated OBS and the plugin no longer loads

That's by design. Due to the black magic used in this plugin, changes in OBS may require adjusting the code in this plugin to match.


## I can't move the scene item

This plugin moves the scene item for you. Use the boundrary settings to control where it can position the source.


## The scene item left the set boundraries

That's by design. If the window leaves the monitor, so will the scene item.

Use the "keep in bounds" setting if you dislike this.


## The scene item doesn't move when the window moves.

Check that the boundrary settings doesn't have a width or height of zero.


## Strange behavior on multi monitor setups

Ensure that you have selected the correct scale mode and monitor settings.


## The monitor setting shows/doesn't show when it should.

This is a defect in the OBS settings system. Reopen the filter settings and it should correct itself.