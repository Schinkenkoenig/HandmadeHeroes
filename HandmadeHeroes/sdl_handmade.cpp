// TODO: Implement sine ourselved
#include <math.h>

#include "SDL_render.h"
#include "handmade.cpp"
#include "handmade.h"

#include "def.h"
#include <SDL.h>
#include <stdio.h>
#include <sys/mman.h>
#include <x86intrin.h>

// NOTE: MAP_ANONYMOUS is not defined on Mac OS X and some other UNIX systems.
// On the vast majority of those systems, one can use MAP_ANON instead.
// Huge thanks to Adam Rosenfield for investigating this, and suggesting this
// workaround:
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

struct sdl_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *texture;
    void *memory;
    int width;
    int height;
    int pitch;
};

struct sdl_window_dimension
{
    int width;
    int height;
};

global_variable sdl_offscreen_buffer global_backbuffer;

#define MAX_CONTROLLERS 4
SDL_GameController *controller_handles[MAX_CONTROLLERS];
SDL_Haptic *rumble_handles[MAX_CONTROLLERS];

struct sdl_audio_ring_buffer
{
    int size;
    int write_cursor;
    int play_cursor;
    void *data;
};

sdl_audio_ring_buffer audio_ring_buffer;

internal void
sdl_audio_callback(void *user_data, Uint8 *audio_data, int length)
{
    sdl_audio_ring_buffer *ring_buffer = (sdl_audio_ring_buffer *)user_data;
    int region1_size = length;
    int region2_size = 0;

    if (ring_buffer->play_cursor + length > ring_buffer->size)
    {
        region1_size = ring_buffer->size - ring_buffer->play_cursor;
        region2_size = length - region1_size;
    }
    memcpy(audio_data, (uint8 *)(ring_buffer->data) + ring_buffer->play_cursor, region1_size);
    memcpy(&audio_data[region1_size], ring_buffer->data, region2_size);
    ring_buffer->play_cursor = (ring_buffer->play_cursor + length) % ring_buffer->size;
    ring_buffer->write_cursor = (ring_buffer->play_cursor + length) % ring_buffer->size;
}

internal void
sdl_init_audio(int32 samples_per_second, int32 buffer_size)
{
    SDL_AudioSpec audio_settings = {0};

    audio_settings.freq = samples_per_second;
    audio_settings.format = AUDIO_S16LSB;
    audio_settings.channels = 2;
    audio_settings.samples = 512;
    audio_settings.callback = &sdl_audio_callback;
    audio_settings.userdata = &audio_ring_buffer;

    audio_ring_buffer.size = buffer_size;
    audio_ring_buffer.data = calloc(buffer_size, 1);
    audio_ring_buffer.play_cursor = audio_ring_buffer.write_cursor = 0;

    SDL_OpenAudio(&audio_settings, 0);

    printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer size %d\n",
           audio_settings.freq, audio_settings.channels, audio_settings.size);

    if (audio_settings.format != AUDIO_S16LSB)
    {
        printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
        SDL_CloseAudio();
    }
}

sdl_window_dimension
sdl_get_window_dimension(SDL_Window *window)
{
    sdl_window_dimension result;

    SDL_GetWindowSize(window, &result.width, &result.height);

    return result;
}

internal void
SDLResizeTexture(sdl_offscreen_buffer *buffer, SDL_Renderer *renderer, int width, int height)
{
    int bytes_per_pixel = 4;

    if (buffer->memory)
    {
        munmap(buffer->memory, buffer->width * buffer->height * bytes_per_pixel);
    }

    if (buffer->texture)
    {
        SDL_DestroyTexture(buffer->texture);
    }
    buffer->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING, width, height);
    buffer->width = width;
    buffer->height = height;
    buffer->pitch = width * bytes_per_pixel;
    buffer->memory = mmap(0, width * height * bytes_per_pixel, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

internal void
sdl_update_window(SDL_Window *window, SDL_Renderer *renderer, sdl_offscreen_buffer *buffer)
{
    SDL_UpdateTexture(buffer->texture, 0, buffer->memory, buffer->pitch);

    SDL_RenderCopy(renderer, buffer->texture, 0, 0);

    SDL_RenderPresent(renderer);
}

bool
handle_event(SDL_Event *event)
{
    bool should_quit = false;

    switch (event->type)
    {
        case SDL_QUIT:
        {
            printf("SDL_QUIT\n");
            should_quit = true;
        }
        break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode key_code = event->key.keysym.sym;
            bool is_down = (event->key.state == SDL_PRESSED);
            bool was_down = false;

            if (event->key.state == SDL_RELEASED)
            {
                was_down = true;
            }
            else if (event->key.repeat != 0)
            {
                was_down = true;
            }

            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (event->key.repeat == 0)
            {
                if (key_code == SDLK_w)
                {
                }
                else if (key_code == SDLK_a)
                {
                }
                else if (key_code == SDLK_s)
                {
                }
                else if (key_code == SDLK_d)
                {
                }
                else if (key_code == SDLK_q)
                {
                }
                else if (key_code == SDLK_e)
                {
                }
                else if (key_code == SDLK_UP)
                {
                }
                else if (key_code == SDLK_LEFT)
                {
                }
                else if (key_code == SDLK_DOWN)
                {
                }
                else if (key_code == SDLK_RIGHT)
                {
                }
                else if (key_code == SDLK_ESCAPE)
                {
                    printf("ESCAPE: ");

                    if (is_down)
                    {
                        printf("IsDown ");
                    }

                    if (was_down)
                    {
                        printf("WasDown");
                    }
                    printf("\n");
                }
                else if (key_code == SDLK_SPACE)
                {
                }
            }

            bool alt_key_was_down = (event->key.keysym.mod & KMOD_ALT);

            if (key_code == SDLK_F4 && alt_key_was_down)
            {
                should_quit = true;
            }
        }
        break;

        case SDL_WINDOWEVENT:
        {
            switch (event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
                    SDL_Renderer *renderer = SDL_GetRenderer(window);
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", event->window.data1,
                           event->window.data2);
                }
                break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                }
                break;

                case SDL_WINDOWEVENT_EXPOSED:
                {
                    SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
                    SDL_Renderer *renderer = SDL_GetRenderer(window);
                    sdl_update_window(window, renderer, &global_backbuffer);
                }
                break;
            }
        }
        break;
    }

    return (should_quit);
}

struct sdl_sound_output
{
    int samples_per_second;
    int tone_hz;
    int16 tone_volume;
    uint32 running_sample_index;
    int wave_period;
    int bytes_per_sample;
    int secondary_buffer_size;
    float32 t_sine;
    int latency_sample_count;
};

internal void
sdl_fill_sound_buffer(sdl_sound_output *sound_output,
                      int byte_to_lock,
                      int bytes_to_write,
                      game_sound_output_buffer *sound_buffer)
{
    int16_t *samples = sound_buffer->samples;
    void *region1 = (uint8 *)audio_ring_buffer.data + byte_to_lock;
    int region1_size = bytes_to_write;

    if (region1_size + byte_to_lock > sound_output->secondary_buffer_size)
    {
        region1_size = sound_output->secondary_buffer_size - byte_to_lock;
    }

    void *region2 = audio_ring_buffer.data;
    int region2_size = bytes_to_write - region1_size;
    int region1_sample_count = region1_size / sound_output->bytes_per_sample;
    int16 *sample_out = (int16 *)region1;

    for (int SampleIndex = 0; SampleIndex < region1_sample_count; ++SampleIndex)
    {
        *sample_out++ = *samples++;
        *sample_out++ = *samples++;

        ++sound_output->running_sample_index;
    }

    int region2_sample_count = region2_size / sound_output->bytes_per_sample;
    sample_out = (int16 *)region2;
    for (int sample_index = 0; sample_index < region2_sample_count; ++sample_index)
    {
        *sample_out++ = *samples++;
        *sample_out++ = *samples++;
        ++sound_output->running_sample_index;
    }
}

internal void
sdl_open_game_controllers()
{
    int max_joysticks = SDL_NumJoysticks();
    int controller_index = 0;
    for (int joystick_index = 0; joystick_index < max_joysticks; ++joystick_index)
    {
        if (!SDL_IsGameController(joystick_index))
        {
            continue;
        }
        if (controller_index >= MAX_CONTROLLERS)
        {
            break;
        }

        controller_handles[controller_index] = SDL_GameControllerOpen(joystick_index);
        SDL_Joystick *joystick_handle =
            SDL_GameControllerGetJoystick(controller_handles[controller_index]);
        rumble_handles[controller_index] = SDL_HapticOpenFromJoystick(joystick_handle);

        if (SDL_HapticRumbleInit(rumble_handles[controller_index]) != 0)
        {
            SDL_HapticClose(rumble_handles[controller_index]);
            rumble_handles[controller_index] = 0;
        }

        controller_index++;
    }
}

internal void
sdl_close_game_controllers()
{
    for (int controller_index = 0; controller_index < MAX_CONTROLLERS; ++controller_index)
    {
        if (controller_handles[controller_index])
        {
            if (rumble_handles[controller_index])
                SDL_HapticClose(rumble_handles[controller_index]);
            SDL_GameControllerClose(controller_handles[controller_index]);
        }
    }
}

int
main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
    uint64 perf_count_frequency = SDL_GetPerformanceFrequency();
    // Initialise our Game Controllers:
    sdl_open_game_controllers();
    // Create our window.
    SDL_Window *window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
    if (window)
    {
        // Create a "Renderer" for our window.
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
        if (renderer)
        {
            bool running = true;
            sdl_window_dimension dimension = sdl_get_window_dimension(window);
            SDLResizeTexture(&global_backbuffer, renderer, dimension.width, dimension.height);
            int x_offset = 0;
            int y_offset = 0;

            sdl_sound_output sound_output = {0};
            sound_output.samples_per_second = 48000;
            sound_output.tone_hz = 256;
            sound_output.tone_volume = 3000;
            sound_output.running_sample_index = 0;
            sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
            sound_output.bytes_per_sample = sizeof(int16) * 2;
            sound_output.secondary_buffer_size =
                sound_output.samples_per_second * sound_output.bytes_per_sample;
            sound_output.t_sine = 0.0f;
            sound_output.latency_sample_count = sound_output.samples_per_second / 15;

            // Open our audio device:
            sdl_init_audio(48000, sound_output.secondary_buffer_size);

            // NOTE: calloc() allocates memory and clears it to zero. It accepts the number of
            // things being allocated and their size.
            int16 *samples =
                (int16 *)calloc(sound_output.samples_per_second, sound_output.bytes_per_sample);
            SDL_PauseAudio(0);

            uint64 last_counter = SDL_GetPerformanceCounter();
            uint64 last_cycle_counter = _rdtsc();
            while (running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    if (handle_event(&event))
                    {
                        running = false;
                    }
                }

                // Poll our controllers for input.
                for (int controller_index = 0; controller_index < MAX_CONTROLLERS;
                     ++controller_index)
                {
                    if (controller_handles[controller_index] != 0 &&
                        SDL_GameControllerGetAttached(controller_handles[controller_index]))
                    {
                        // NOTE: We have a controller with index ControllerIndex.
                        bool up = SDL_GameControllerGetButton(controller_handles[controller_index],
                                                              SDL_CONTROLLER_BUTTON_DPAD_UP);
                        bool down = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                        bool left = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                        bool right = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                        bool start = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_START);
                        bool back = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_BACK);
                        bool left_shoulder =
                            SDL_GameControllerGetButton(controller_handles[controller_index],
                                                        SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
                        bool right_shoulder =
                            SDL_GameControllerGetButton(controller_handles[controller_index],
                                                        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
                        bool a_button = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_A);
                        bool b_button = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_B);
                        bool x_button = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_X);
                        bool y_button = SDL_GameControllerGetButton(
                            controller_handles[controller_index], SDL_CONTROLLER_BUTTON_Y);

                        int16 stick_x = SDL_GameControllerGetAxis(
                            controller_handles[controller_index], SDL_CONTROLLER_AXIS_LEFTX);
                        int16 stick_y = SDL_GameControllerGetAxis(
                            controller_handles[controller_index], SDL_CONTROLLER_AXIS_LEFTY);

                        if (a_button)
                        {
                            y_offset += 2;
                        }
                        if (b_button)
                        {
                            if (rumble_handles[controller_index])
                            {
                                SDL_HapticRumblePlay(rumble_handles[controller_index], 0.5f, 2000);
                            }
                        }

                        x_offset += stick_x / 4096;
                        y_offset += stick_y / 4096;

                        sound_output.tone_hz = 512 + (int)(256.0f * ((float32)stick_y / 30000.0f));
                        sound_output.wave_period =
                            sound_output.samples_per_second / sound_output.tone_hz;
                    }
                    else
                    {
                        // TODO: This controller is not plugged in.
                    }
                }

                // Sound output test
                SDL_LockAudio();
                int byte_to_lock =
                    (sound_output.running_sample_index * sound_output.bytes_per_sample) %
                    sound_output.secondary_buffer_size;
                int target_cursor =
                    ((audio_ring_buffer.play_cursor +
                      (sound_output.latency_sample_count * sound_output.bytes_per_sample)) %
                     sound_output.secondary_buffer_size);
                int bytes_to_write;

                if (byte_to_lock > target_cursor)
                {
                    bytes_to_write = (sound_output.secondary_buffer_size - byte_to_lock);
                    bytes_to_write += target_cursor;
                }
                else
                {
                    bytes_to_write = target_cursor - byte_to_lock;
                }

                SDL_UnlockAudio();

                game_sound_output_buffer sound_buffer = {};
                sound_buffer.samples_per_second = sound_output.samples_per_second;
                sound_buffer.sample_count = bytes_to_write / sound_output.bytes_per_sample;
                sound_buffer.samples = samples;

                game_offscreen_buffer buffer = {};
                buffer.memory = global_backbuffer.memory;
                buffer.width = global_backbuffer.width;
                buffer.height = global_backbuffer.height;
                buffer.pitch = global_backbuffer.pitch;

                game_update_and_render(&buffer, x_offset, y_offset, &sound_buffer,
                                       sound_output.tone_hz);

                sdl_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
                sdl_update_window(window, renderer, &global_backbuffer);

                uint64 end_cycle_counter = _rdtsc();
                uint64 end_counter = SDL_GetPerformanceCounter();
                uint64 counter_elapsed = end_counter - last_counter;
                uint64 cycles_elapsed = end_cycle_counter - last_cycle_counter;

                float64 ms_per_frame =
                    (((1000.0f * (float64)counter_elapsed) / (float64)perf_count_frequency));
                float64 FPS = (float64)perf_count_frequency / (float64)counter_elapsed;
                float64 MCPF = ((float64)cycles_elapsed / (1000.0f * 1000.0f));

                printf("%.02fms/f, %.02f/s, %.02fmc/f\n", ms_per_frame, FPS, MCPF);

                last_cycle_counter = end_cycle_counter;
                last_counter = end_counter;
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    else
    {
        // TODO(casey): Logging
    }

    sdl_close_game_controllers();
    SDL_Quit();
    return (0);
}
