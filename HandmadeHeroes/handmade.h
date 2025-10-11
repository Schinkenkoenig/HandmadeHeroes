/*
  TODO(casey): Services that the platform layer provides to the game
*/

/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!

#include "def.h"
#if !defined(HANDMADE_H)

struct game_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *memory;
    int width;
    int height;
    int pitch;
};

struct game_sound_output_buffer
{
    int samples_per_second;
    int sample_count;
    int16 *samples;
};

internal void
game_update_and_render(game_offscreen_buffer *offscreen_buffer,
                       int blue_offset,
                       int green_offset,
                       game_sound_output_buffer *sound_buffer);

#define HANDMADE_H
#endif
