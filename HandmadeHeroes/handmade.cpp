#include "handmade.h"
#include "def.h"
#include <math.h>

internal void
game_output_sound(game_sound_output_buffer *sound_buffer, int tone_hz)
{
    local_persist float32 t_sine;
    int16 tone_volume = 3000;
    int wave_period = sound_buffer->samples_per_second / tone_hz;

    int16 *sample_out = sound_buffer->samples;
    for (int sample_index = 0; sample_index < sound_buffer->sample_count; ++sample_index)
    {
        // TODO(casey): Draw this out for people
        float32 sine_value = sinf(t_sine);
        int16 sample_value = (int16)(sine_value * tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        t_sine += 2.0f * Pi32 * 1.0f / (float32)wave_period;
    }
}

internal void
render_weird_gradient(game_offscreen_buffer *buffer, int blue_offset, int green_offset)
{
    // TODO(casey): Let's see what the optimizer does

    uint8 *row = (uint8 *)buffer->memory;
    for (int y = 0; y < buffer->height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer->width; ++x)
        {
            uint8 blue = (x + blue_offset);
            uint8 green = (y + green_offset);

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

internal void
game_update_and_render(game_offscreen_buffer *buffer,
                       int blue_offset,
                       int green_offset,
                       game_sound_output_buffer *sound_buffer,
                       int tone_hz)
{
    // TODO(casey): Allow sample offsets here for more robust platform options
    game_output_sound(sound_buffer, tone_hz);
    render_weird_gradient(buffer, blue_offset, green_offset);
}
