/*
* Dreamroq by Mike Melanson
* Updated by Josh Pearson to add audio support
* Made into a library by lerabot in 2020

  The most basic way to play a video from the library.
  This uses the default video/audio callback.
  Feel free to check the library code and come up with your own.

*/

#include <stdio.h>
#include <dreamroq/dreamroqlib.h>

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

int main() {
  printf("DreamROQ Demo.\n");

  vid_set_mode(DM_640x480, PM_RGB565);
  pvr_init_defaults();

  roq_play( "/rd/video.roq",   // path to .roq file
            0,                // 1 = loop, 0 = no loop
            roq_render_cb,    // default video callback.
            roq_audio_cb,     // default audio callback
            roq_quit_cb      // default quit callback
          );

  printf("DreamROQ video done.\n");
}
