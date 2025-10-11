// TODO: Implement sine ourselved
#include <math.h>

#include "handmade.cpp"
#include "handmade.h"

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
SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
SDL_Haptic *RumbleHandles[MAX_CONTROLLERS];

internal void
SDLInitAudio(int32 SamplesPerSecond, int32 BufferSize)
{
    SDL_AudioSpec AudioSettings = {0};

    AudioSettings.freq = SamplesPerSecond;
    AudioSettings.format = AUDIO_S16LSB;
    AudioSettings.channels = 2;
    AudioSettings.samples = BufferSize;

    SDL_OpenAudio(&AudioSettings, 0);

    if (AudioSettings.format != AUDIO_S16LSB)
    {
        printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
        SDL_CloseAudio();
    }
}

sdl_window_dimension
SDLGetWindowDimension(SDL_Window *Window)
{
    sdl_window_dimension Result;

    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);

    return (Result);
}

internal void
SDLResizeTexture(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height)
{
    int BytesPerPixel = 4;
    if (Buffer->Memory)
    {
        munmap(Buffer->Memory, Buffer->Width * Buffer->Height * BytesPerPixel);
    }
    if (Buffer->Texture)
    {
        SDL_DestroyTexture(Buffer->Texture);
    }
    Buffer->Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING, Width, Height);
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Width * BytesPerPixel;
    Buffer->Memory = mmap(0, Width * Height * BytesPerPixel, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

internal void
SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer, sdl_offscreen_buffer *Buffer)
{
    SDL_UpdateTexture(Buffer->Texture, 0, Buffer->Memory, Buffer->Pitch);

    SDL_RenderCopy(Renderer, Buffer->Texture, 0, 0);

    SDL_RenderPresent(Renderer);
}

bool
HandleEvent(SDL_Event *Event)
{
    bool ShouldQuit = false;

    switch (Event->type)
    {
        case SDL_QUIT:
        {
            printf("SDL_QUIT\n");
            ShouldQuit = true;
        }
        break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode KeyCode = Event->key.keysym.sym;
            bool IsDown = (Event->key.state == SDL_PRESSED);
            bool WasDown = false;
            if (Event->key.state == SDL_RELEASED)
            {
                WasDown = true;
            }
            else if (Event->key.repeat != 0)
            {
                WasDown = true;
            }

            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (Event->key.repeat == 0)
            {
                if (KeyCode == SDLK_w)
                {
                }
                else if (KeyCode == SDLK_a)
                {
                }
                else if (KeyCode == SDLK_s)
                {
                }
                else if (KeyCode == SDLK_d)
                {
                }
                else if (KeyCode == SDLK_q)
                {
                }
                else if (KeyCode == SDLK_e)
                {
                }
                else if (KeyCode == SDLK_UP)
                {
                }
                else if (KeyCode == SDLK_LEFT)
                {
                }
                else if (KeyCode == SDLK_DOWN)
                {
                }
                else if (KeyCode == SDLK_RIGHT)
                {
                }
                else if (KeyCode == SDLK_ESCAPE)
                {
                    printf("ESCAPE: ");
                    if (IsDown)
                    {
                        printf("IsDown ");
                    }
                    if (WasDown)
                    {
                        printf("WasDown");
                    }
                    printf("\n");
                }
                else if (KeyCode == SDLK_SPACE)
                {
                }
            }

            bool AltKeyWasDown = (Event->key.keysym.mod & KMOD_ALT);
            if (KeyCode == SDLK_F4 && AltKeyWasDown)
            {
                ShouldQuit = true;
            }
        }
        break;

        case SDL_WINDOWEVENT:
        {
            switch (Event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", Event->window.data1,
                           Event->window.data2);
                }
                break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                }
                break;

                case SDL_WINDOWEVENT_EXPOSED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    SDLUpdateWindow(Window, Renderer, &GlobalBackbuffer);
                }
                break;
            }
        }
        break;
    }

    return (ShouldQuit);
}

struct sdl_sound_output
{
    int SamplesPerSecond;
    int ToneHz;
    int16 ToneVolume;
    uint32 RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

internal void
SDLFillSoundBuffer(sdl_sound_output *SoundOutput,
                   int ByteToLock,
                   int BytesToWrite,
                   game_sound_output_buffer *SoundBuffer)
{
    SDL_QueueAudio(1, SoundBuffer->Samples, BytesToWrite);
}

internal void
SDLOpenGameControllers()
{
    int MaxJoysticks = SDL_NumJoysticks();
    int ControllerIndex = 0;
    for (int JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex)
    {
        if (!SDL_IsGameController(JoystickIndex))
        {
            continue;
        }
        if (ControllerIndex >= MAX_CONTROLLERS)
        {
            break;
        }
        ControllerHandles[ControllerIndex] = SDL_GameControllerOpen(JoystickIndex);
        SDL_Joystick *JoystickHandle =
            SDL_GameControllerGetJoystick(ControllerHandles[ControllerIndex]);
        RumbleHandles[ControllerIndex] = SDL_HapticOpenFromJoystick(JoystickHandle);
        if (SDL_HapticRumbleInit(RumbleHandles[ControllerIndex]) != 0)
        {
            SDL_HapticClose(RumbleHandles[ControllerIndex]);
            RumbleHandles[ControllerIndex] = 0;
        }

        ControllerIndex++;
    }
}

internal void
SDLCloseGameControllers()
{
    for (int ControllerIndex = 0; ControllerIndex < MAX_CONTROLLERS; ++ControllerIndex)
    {
        if (ControllerHandles[ControllerIndex])
        {
            if (RumbleHandles[ControllerIndex])
                SDL_HapticClose(RumbleHandles[ControllerIndex]);
            SDL_GameControllerClose(ControllerHandles[ControllerIndex]);
        }
    }
}

int
main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
    uint64 PerfCountFrequency = SDL_GetPerformanceFrequency();
    // Initialise our Game Controllers:
    SDLOpenGameControllers();
    // Create our window.
    SDL_Window *Window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
    if (Window)
    {
        // Create a "Renderer" for our window.
        SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_PRESENTVSYNC);
        if (Renderer)
        {
            bool Running = true;
            sdl_window_dimension Dimension = SDLGetWindowDimension(Window);
            SDLResizeTexture(&GlobalBackbuffer, Renderer, Dimension.Width, Dimension.Height);
            int XOffset = 0;
            int YOffset = 0;

            sdl_sound_output SoundOutput = {};

            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.ToneVolume = 3000;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            // Open our audio device:
            SDLInitAudio(SoundOutput.SamplesPerSecond,
                         SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / 60);
            // NOTE: calloc() allocates memory and clears it to zero. It accepts the number of
            // things being allocated and their size.
            int16 *Samples =
                (int16 *)calloc(SoundOutput.LatencySampleCount, SoundOutput.BytesPerSample);
            SDL_PauseAudio(0);

            uint64 LastCounter = SDL_GetPerformanceCounter();
            uint64 LastCycleCount = _rdtsc();
            while (Running)
            {
                SDL_Event Event;
                while (SDL_PollEvent(&Event))
                {
                    if (HandleEvent(&Event))
                    {
                        Running = false;
                    }
                }

                // Poll our controllers for input.
                for (int ControllerIndex = 0; ControllerIndex < MAX_CONTROLLERS; ++ControllerIndex)
                {
                    if (ControllerHandles[ControllerIndex] != 0 &&
                        SDL_GameControllerGetAttached(ControllerHandles[ControllerIndex]))
                    {
                        // NOTE: We have a controller with index ControllerIndex.
                        bool Up = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                              SDL_CONTROLLER_BUTTON_DPAD_UP);
                        bool Down = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                                SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                        bool Left = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                                SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                        bool Right = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                                 SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                        bool Start = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                                 SDL_CONTROLLER_BUTTON_START);
                        bool Back = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                                SDL_CONTROLLER_BUTTON_BACK);
                        bool LeftShoulder = SDL_GameControllerGetButton(
                            ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
                        bool RightShoulder =
                            SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
                                                        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
                        bool AButton = SDL_GameControllerGetButton(
                            ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_A);
                        bool BButton = SDL_GameControllerGetButton(
                            ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_B);
                        bool XButton = SDL_GameControllerGetButton(
                            ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_X);
                        bool YButton = SDL_GameControllerGetButton(
                            ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_Y);

                        int16 StickX = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex],
                                                                 SDL_CONTROLLER_AXIS_LEFTX);
                        int16 StickY = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex],
                                                                 SDL_CONTROLLER_AXIS_LEFTY);

                        XOffset += StickX / 4096;
                        YOffset += StickY / 4096;

                        SoundOutput.ToneHz = 512 + (int)(256.0f * ((real32)StickY / 30000.0f));
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
                    }
                    else
                    {
                        // TODO: This controller is not plugged in.
                    }
                }

                // Sound output test
                int TargetQueueBytes = SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample;
                int BytesToWrite = TargetQueueBytes - SDL_GetQueuedAudioSize(1);
                game_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;

                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch;
                GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);

                SDLFillSoundBuffer(&SoundOutput, 0, BytesToWrite, &SoundBuffer);

                SDLUpdateWindow(Window, Renderer, &GlobalBackbuffer);
                uint64 EndCycleCount = _rdtsc();
                uint64 EndCounter = SDL_GetPerformanceCounter();
                uint64 CounterElapsed = EndCounter - LastCounter;
                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;

                real64 MSPerFrame =
                    (((1000.0f * (real64)CounterElapsed) / (real64)PerfCountFrequency));
                real64 FPS = (real64)PerfCountFrequency / (real64)CounterElapsed;
                real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));

                printf("%.02fms/f, %.02f/s, %.02fmc/f\n", MSPerFrame, FPS, MCPF);

                LastCycleCount = EndCycleCount;
                LastCounter = EndCounter;
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

    SDLCloseGameControllers();
    SDL_Quit();
    return (0);
}
