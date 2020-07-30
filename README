Dreamroq by Mike Melanson (mike -at- multimedia.cx)
R2 update by Josh Pearson (ph3nom.dcmc@gmail.com)
Library version by lerabot

## Introduction
Dreamroq is a RoQ playback library designed for the Sega Dreamcast video
game console.

RoQ is a relatively simple video file format developed for video-heavy
CD-ROM games. Read more about the format here: http://wiki.multimedia.cx/index.php?title=RoQ

The Dreamroq library includes a player component that is designed to run
under the KallistiOS (KOS) open source operating system. Read more about
KOS at: http://gamedev.allusion.net/softprj/kos/

The library also includes a sample testing utility that can be built
and executed on Unix systems. This utility is useful for debugging and
validation.

RoQ sample files can be found at: http://samples.mplayerhq.hu/game-formats/idroq/

RoQ files can also be created using the Switchblade encoder: http://icculus.org/~riot/

A version of Switchblade is also included in FFmpeg and many derivative
programs: http://ffmpeg.org/


## License
Dreamroq is meant to be license-compatible with the rest of the KallistiOS
operating system, which is a BSD-style open source license. You can read
the specific text in LICENSE.KOS.


## Building the library
Make sure you clone this this directory
 $(KOS_BASE)/addons

Then, inside the libdreamroq folder, simply type
make defaultall

You'll be able to link the library using
 -ldreamroq

There is a very simple exemple in the samples folder.

## Encoding video using FFMPEG
You can encode your video using this handy ffmpeg command

 ffmpeg -i input_video.mp4 -ar 22050 output_video.roq

The -ar argument is recommended if you're planning to have sound in your video.

Additional options :
-s : Size
-b:v : Video Bitrate
-b:a : Audio Bitrate
-vf scale=640:436,pad=640:448:0:6

## Bugs, Issues, and Future Development
:R2 Release by PH3NOM:

I have updated the dreamroq library to support decoding of DPCM
audio samples encoded in the RoQ stream.

The K:OS Dreamcast player has been updated to support streaming
of the decoded samples, directly using the Dreamcast's AICA
audio hardware, running in a seperate thread.

Furthermore, a frame-rate timer has been implemented, so the video
should be played back at correct speed.  Also, the allocated PVR
memory is now freed on exit of the player.
