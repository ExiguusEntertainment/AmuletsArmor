#include "direct.h"
#include <time.h>
#include <Windows.h>
#include "DITALK.H"
#ifdef _DEBUG
   #include <crtdbg.h>
#endif
#include <SDL.h>

#define CAP_SPEED_TO_FPS       0 // 70 // 0

static int G_done = FALSE;
static SDL_Surface* screen;
static SDL_Surface* surface;
static SDL_Surface* largesurface;
static SDL_Rect srcrect = {
        0, 0,
        320, 240
    };
static SDL_Rect largesrcrect = {
        0, 0,
        640, 480
    };
static SDL_Rect destrect = {
        0, 0,
        640, 480
    };
extern T_void KeyboardUpdate(E_Boolean updateBuffers);

void SleepMS(T_word32 aMS)
{
    Sleep(aMS);
}

void WindowsUpdateMouse(void)
{
    int flags = 0 ;
    int x, y;
    Uint8 state;

    state = SDL_GetMouseState(&x, &y);
    DirectMouseSet(x, y);
    if (state & SDL_BUTTON_LMASK)
        flags |= MOUSE_BUTTON_LEFT;
    if (state & SDL_BUTTON_RMASK)
        flags |= MOUSE_BUTTON_RIGHT;
    if (state & SDL_BUTTON_MMASK)
        flags |= MOUSE_BUTTON_MIDDLE;
    DirectMouseSetButton(flags);
}

void WindowsUpdateEvents(void)
{
    int flags;
    SDL_Event event;
    static int altPressed = FALSE;

    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_QUIT:
                G_done = TRUE;
                break;
            case SDL_KEYDOWN:
                if ( event.key.keysym.sym == SDLK_ESCAPE )  {
                    G_done = TRUE; 
                } else if ((event.key.keysym.sym == SDLK_LALT) || (event.key.keysym.sym == SDLK_RALT)) {
                    // Left or right alt pressed?
                    altPressed = TRUE;
                } else if ((event.key.keysym.sym == SDLK_RETURN) && (altPressed)) {
                    // ALT-Enter toggles full screen
#if 1
                    flags = screen->flags; /* Save the current flags in case toggling fails */
                    screen = SDL_SetVideoMode(0, 0, 0, screen->flags ^ SDL_FULLSCREEN); /*Toggles FullScreen Mode */
                    if(screen == NULL) screen = SDL_SetVideoMode(0, 0, 0, flags); /* If toggle FullScreen failed, then switch back */
                    if(screen == NULL) exit(1); /* If you can't switch back for some reason, then epic fail */                    
#endif
                }
                break;
            case SDL_KEYUP:
                if ((event.key.keysym.sym == SDLK_LALT) || (event.key.keysym.sym == SDLK_RALT)) {
                    // Left or right alt released?
                    altPressed = FALSE;
                }
                break;
#if 0
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                DirectMouseSet(event.motion.x, event.motion.y);
                flags = 0;
                if (event.motion.state & SDL_BUTTON_LMASK)
                    flags |= MOUSE_BUTTON_LEFT;
                if (event.motion.state & SDL_BUTTON_RMASK)
                    flags |= MOUSE_BUTTON_RIGHT;
                if (event.motion.state & SDL_BUTTON_MMASK)
                    flags |= MOUSE_BUTTON_MIDDLE;
                DirectMouseSetButton(flags);
                break;
#endif
        }    
        SDL_GetKeyState(NULL);
        //keys = SDL_GetKeyState(NULL);
        
//        if ( keys[SDLK_UP] )    ypos -= 1;
//        if ( keys[SDLK_DOWN] )  ypos += 1;
//        if ( keys[SDLK_LEFT] )  xpos -= 1;
//        if ( keys[SDLK_RIGHT] ) xpos += 1;

//        DrawScene();
    }
}

#define Copy2x(aDest, aSrc) \
        *(aDest++) = *aSrc; \
        *(aDest++) = *(aSrc++);

#define oldCopy2x_4times(aDest, aSrc) \
    Copy2x(aDest, aSrc) \
    Copy2x(aDest, aSrc) \
    Copy2x(aDest, aSrc) \
    Copy2x(aDest, aSrc)

#define Copy2x_4times(aDest, aSrc) \
    v = *((T_word32 *)aSrc); \
    aSrc += 4; \
    aDest[0] = ((T_byte8 *)&v)[0]; \
    aDest[1] = ((T_byte8 *)&v)[0]; \
    aDest[2] = ((T_byte8 *)&v)[1]; \
    aDest[3] = ((T_byte8 *)&v)[1]; \
    aDest[4] = ((T_byte8 *)&v)[2]; \
    aDest[5] = ((T_byte8 *)&v)[2]; \
    aDest[6] = ((T_byte8 *)&v)[3]; \
    aDest[7] = ((T_byte8 *)&v)[3]; \
    aDest += 8;

#define Copy2x_20times(aDest, aSrc) \
    Copy2x_4times(aDest, aSrc) \
    Copy2x_4times(aDest, aSrc) \
    Copy2x_4times(aDest, aSrc) \
    Copy2x_4times(aDest, aSrc) \
    Copy2x_4times(aDest, aSrc)

#define Copy2x_100times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc)

#define Copy2x_320times(aDest, aSrc) \
    Copy2x_100times(aDest, aSrc) \
    Copy2x_100times(aDest, aSrc) \
    Copy2x_100times(aDest, aSrc) \
    Copy2x_20times(aDest, aSrc)

void WindowsUpdate(char *p_screen, unsigned char *palette)
{
    SDL_Color colors[256];
    int i;
    unsigned char *src = (char *)surface->pixels;
    unsigned char *dst = (char *)largesurface->pixels;
    unsigned char *line;
    static int lastFPS = 0;
    static int fps = 0;
    int y;
    T_word32 tick = clock();
    static T_word32 lastTick = 0xFFFFEEEE;
    static double movingAverage = 0;
    T_word32 v;
    T_word32 frac;

#if CAP_SPEED_TO_FPS
        if ((tick-lastTick)<(1000/CAP_SPEED_TO_FPS)) {
Sleep((1000/CAP_SPEED_TO_FPS) - (tick-lastTick));
        // 10 ms between frames (top out at 100 ms)
    } else
#endif
    {
        lastTick = tick;
//printf("Update: %d (%d)\n", clock(), TickerGet());

    // Setup the color palette for this update
    for (i=0; i<256; i++) {
        colors[i].r = ((((unsigned int)*(palette++))&0x3F)<<2);
        colors[i].g = ((((unsigned int)*(palette++))&0x3F)<<2);
        colors[i].b = ((((unsigned int)*(palette++))&0x3F)<<2);
    }
    //SDL_SetColors(surface, colors, 0, 256);
    SDL_SetColors(largesurface, colors, 0, 256);

    // Blit the current surface from 320x200 to 640x480
    line = src;
    for (y=0, frac=0; y<200; y++, line+=320) {
//        for (x=0; x<320; x++) {
//            *(dst++) = *src;
//            *(dst++) = *(src++);
//        }
        while (frac < 400) {
            src = line;
            Copy2x_320times(dst, src);
            frac += 200;
        }
        frac -= 400;
//        for (x=0; x<320; x++) {
//            *(dst++) = *src;
//            *(dst++) = *(src++);
//        }
//        Copy2x_320times(dst, src);
    }

    if (SDL_BlitSurface(largesurface, &largesrcrect, screen, &destrect)) {
        printf("Failed blit: %s\n", SDL_GetError());
    }
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    fps++;

    if ((tick-lastFPS) >= 1000) {
        if (movingAverage < 1.0)
            movingAverage = fps;
        movingAverage = ((double)fps)*0.05+movingAverage*0.95;
        lastFPS += 1000;
        //printf("%02d:%02d:%02d.%03d FPS: %d, %f\n", tick/3600000, (tick/60000) % 60, (tick/1000) % 60, tick%1000, fps, movingAverage);
        fps = 0;
    }
    WindowsUpdateEvents();
    WindowsUpdateMouse();
    KeyboardUpdate(TRUE) ;
#if CAP_SPEED_TO_100_FPS
    Sleep(1);
#endif
    }
}


extern T_void game_main(T_word16 argc, char *argv[]);

int SDL_main(int argc, char *argv[])
{
    char *pixels;
    int x, y;
    SDL_Color black = { 0, 0, 0, 0 };
    SDL_Color white = { 255, 255, 255, 0 };

    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
    {
          printf ("Could not initialize SDL: %s\n",SDL_GetError());
          return 1;
    }

    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(640, 400, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Amulets & Armor", "Amulets & Armor");
    SDL_ShowCursor( SDL_DISABLE ); 

    if(screen == NULL)
    {
          printf("Could not set video mode: %s\n",SDL_GetError());
          return 1;
    }

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 8, 0, 0, 0, 0);
    if (surface == NULL) {
        printf("Could not create overlay: %s\n", SDL_GetError());
        return 1;
    }
    largesurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 8, 0, 0, 0, 0);
    if (largesurface == NULL) {
        printf("Could not create overlay: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetColors(surface, &black, 0, 1);
    SDL_SetColors(surface, &white, 255, 1);
    pixels = (char *)surface->pixels;
    GRAPHICS_ACTUAL_SCREEN = (void *)pixels;
    for (y=0; y<240; y++) {
        for (x=0; x<320; x++, pixels++) {
            if ((x == 0) || (x == 319) || (y == 0) || (y == 239))
                *pixels = 255;
            else
                *pixels = 0;
        }
    }

    {
#ifndef NDEBUG
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag( tmpFlag );
#endif

        game_main(argc, argv);
    }

    return 0;
}

