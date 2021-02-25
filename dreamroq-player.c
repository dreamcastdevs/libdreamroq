/*
 * Dreamroq by Mike Melanson
 * Updated by Josh Pearson to add audio support
 *
 * This is the sample Dreamcast player app, designed to be run under
 * the KallistiOS operating system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "include/dreamroqlib.h"

#include "dc_timer.h"
#include "snddrv.h"

/* Audio Global variables */
#define PCM_BUF_SIZE 1024*1024
static unsigned char *pcm_buf = NULL;
static int pcm_size = 0;
static int audio_init = 0;
static mutex_t * pcm_mut;

/* Video Global variables */
static pvr_ptr_t  textures[2];
static int        current_frame = 0;
static int        graphics_initialized = 0;
static float      video_delay;
static int        frame=0;
static int        vid_width = 640;
static int        vid_height = 480;
static const float VIDEO_RATE = 30.0f; /* Video FPS */

static void snd_thd(){
    do
    {
        /* Wait for AICA Driver to request some samples */
        while( snddrv.buf_status != SNDDRV_STATUS_NEEDBUF )
            thd_pass();

        /* Wait for RoQ Decoder to produce enough samples */
        while( pcm_size < snddrv.pcm_needed )
        {
            if( snddrv.dec_status == SNDDEC_STATUS_DONE )
                goto done;
            thd_pass();
        }

        /* Copy the Requested PCM Samples to the AICA Driver */
        mutex_lock( pcm_mut );
        memcpy( snddrv.pcm_buffer, pcm_buf, snddrv.pcm_needed );

        /* Shift the Remaining PCM Samples Back */
        pcm_size -= snddrv.pcm_needed;
        memmove( pcm_buf, pcm_buf+snddrv.pcm_needed, pcm_size );
        mutex_unlock( pcm_mut );

        /* Let the AICA Driver know the PCM samples are ready */
        snddrv.buf_status = SNDDRV_STATUS_HAVEBUF;

    } while( snddrv.dec_status == SNDDEC_STATUS_STREAMING );
    done:
    snddrv.dec_status = SNDDEC_STATUS_NULL;
}

int roq_set_size(int width, int height) {
  vid_width = width;
  vid_height = height;
}

 int roq_render_cb(unsigned short *buf, int width, int height, int stride,
    int texture_height){
    pvr_poly_cxt_t cxt;
    static pvr_poly_hdr_t hdr[2];
    static pvr_vertex_t vert[4];

    float ratio;
    /* screen coordinates of upper left and bottom right corners */
    static int ul_x, ul_y, br_x, br_y;

    /* on first call, initialize textures and drawing coordinates */
    if (!graphics_initialized)
    {
        textures[0] = pvr_mem_malloc(stride * texture_height * 2);
        textures[1] = pvr_mem_malloc(stride * texture_height * 2);
        if (!textures[0] || !textures[1])
        {
            return ROQ_RENDER_PROBLEM;
        }

        /* Precompile the poly headers */
        pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, stride, texture_height, textures[0], PVR_FILTER_TRILINEAR2);
        pvr_poly_compile(&hdr[0], &cxt);
        pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, stride, texture_height, textures[1], PVR_FILTER_TRILINEAR2);
        pvr_poly_compile(&hdr[1], &cxt);

        /* this only works if width ratio <= height ratio */
        ratio = 640 / width;
        ul_x = 0;
        br_x = (ratio * stride);
        ul_y = ((480 - ratio * 480) / 2);
        br_y = ul_y + ratio * texture_height;

        ul_x = 0;
        ul_y = 0;
        br_x = 640;
        br_y = 480;



        /* Things common to vertices */
        vert[0].z     = vert[1].z     = vert[2].z     = vert[3].z     = 1.0f;
        vert[0].argb  = vert[1].argb  = vert[2].argb  = vert[3].argb  = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
        vert[0].oargb = vert[1].oargb = vert[2].oargb = vert[3].oargb = 0;
        vert[0].flags = vert[1].flags = vert[2].flags = PVR_CMD_VERTEX;
        vert[3].flags = PVR_CMD_VERTEX_EOL;

        vert[0].x = ul_x;
        vert[0].y = ul_y;
        vert[0].u = 0.0;
        vert[0].v = 0.0;

        vert[1].x = br_x;
        vert[1].y = ul_y;
        vert[1].u = 1.0;
        vert[1].v = 0.0;

        vert[2].x = ul_x;
        vert[2].y = br_y;
        vert[2].u = 0.0;
        vert[2].v = 1.0;

        vert[3].x = br_x;
        vert[3].y = br_y;
        vert[3].u = 1.0;
        vert[3].v = 1.0;

        /* Get current hardware timing */
        video_delay = (float)dc_get_time();

        graphics_initialized = 1;
    }

    /* send the video frame as a texture over to video RAM */
    pvr_txr_load(buf, textures[current_frame], stride * texture_height * 2);

    /* Delay the frame to match Frame Rate */
    frame_delay( VIDEO_RATE, video_delay, ++frame );

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);

    pvr_prim(&hdr[current_frame], sizeof(pvr_poly_hdr_t));
    pvr_prim(&vert[0], sizeof(pvr_vertex_t));
    pvr_prim(&vert[1], sizeof(pvr_vertex_t));
    pvr_prim(&vert[2], sizeof(pvr_vertex_t));
    pvr_prim(&vert[3], sizeof(pvr_vertex_t));

    pvr_list_finish();
    pvr_scene_finish();

    if (current_frame)
        current_frame = 0;
    else
        current_frame = 1;

    return ROQ_SUCCESS;
}

 int roq_audio_cb( unsigned char *buf, int size, int channels){
    if(!audio_init)
    {
        /* allocate PCM buffer */
        pcm_buf = malloc(PCM_BUF_SIZE);
        if( pcm_buf == NULL )
            return ROQ_NO_MEMORY;

        /* Start AICA Driver */
        snddrv_start( 22050, channels );
        snddrv.dec_status = SNDDEC_STATUS_STREAMING;

        /* Create a thread to stream the samples to the AICA */
        thd_create(0, snd_thd, NULL );

        /* Create a mutex to handle the double-threaded buffer */
        pcm_mut = mutex_create();

        audio_init=1;
    }

    /* Copy the decoded PCM samples to our local PCM buffer */
    mutex_lock( pcm_mut );
    memcpy(  pcm_buf+pcm_size, buf, size);
    pcm_size += size;
    mutex_unlock( pcm_mut );

    return ROQ_SUCCESS;
}

int roq_quit_cb(){
  maple_device_t *cont;
  cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

  cont_state_t *state;
  state = maple_dev_status(cont);
  return (state->buttons & CONT_START);
}

int roq_free_texture(){
  pvr_mem_free(textures[0]);
  pvr_mem_free(textures[1]);
}

int free_variables() {
  current_frame = 0;
  graphics_initialized = 0;
  video_delay;
  frame=0;
  vid_width = 640;
  vid_height = 480;	
}

int roq_free_audio() {
  if(audio_init)
  {
    snddrv.dec_status = SNDDEC_STATUS_DONE;  /* Singal audio thread to stop */
    while( snddrv.dec_status != SNDDEC_STATUS_NULL )
       thd_pass();
    free( pcm_buf );
    pcm_buf = NULL;
    pcm_size = 0;
    mutex_destroy(pcm_mut);                  /* Destroy the PCM mutex */
    snddrv_exit();                           /* Exit the AICA Driver */
    audio_init = 0;
  }
  return(1);
}
