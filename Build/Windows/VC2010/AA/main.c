#include "direct.h"
#include <time.h>
#include <Windows.h>
#include "DITALK.H"
#ifdef _DEBUG
   #include <crtdbg.h>
#endif
#include <SDL.h>
#include "resource.h"
#include <GL/GL.h>
#include <gl/GLU.h>

extern int test_main( int argc, char* argv[] );

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
        WINDOW_WIDTH, 480
    };
static SDL_Rect destrect = {
        0, 0,
        WINDOW_WIDTH, 480
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
                    flags = screen->flags; /* Save the current flags in case toggling fails */
                    screen = SDL_SetVideoMode(0, 0, 0, screen->flags ^ SDL_FULLSCREEN); /*Toggles FullScreen Mode */
                    if(screen == NULL) screen = SDL_SetVideoMode(0, 0, 0, flags); /* If toggle FullScreen failed, then switch back */
                    if(screen == NULL) exit(1); /* If you can't switch back for some reason, then epic fail */                    
                }
                break;
            case SDL_KEYUP:
                if ((event.key.keysym.sym == SDLK_LALT) || (event.key.keysym.sym == SDLK_RALT)) {
                    // Left or right alt released?
                    altPressed = FALSE;
                }
                break;
        }    
        SDL_GetKeyState(NULL);
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

void WindowsRenderSDL(char *p_screen, unsigned char *palette)
{
    SDL_Color colors[256];
    unsigned char *src = (char *)surface->pixels;
    unsigned char *dst = (char *)largesurface->pixels;
    unsigned char *line;
    int i;
    T_word32 frac;
    int y;
    T_word32 v;

    // Setup the color palette for this update
    for (i=0; i<256; i++) {
        colors[i].r = ((((unsigned int)*(palette++))&0x3F)<<2);
        colors[i].g = ((((unsigned int)*(palette++))&0x3F)<<2);
        colors[i].b = ((((unsigned int)*(palette++))&0x3F)<<2);
    }
    //SDL_SetColors(surface, colors, 0, 256);
    SDL_SetColors(largesurface, colors, 0, 256);

    // Blit the current surface from 320x200 to WINDOW_WIDTHx480
    line = src;
    for (y=0, frac=0; y<200; y++, line+=320) {
        while (frac < WINDOW_HEIGHT) {
            src = line;
            Copy2x_320times(dst, src);
            frac += 200;
        }
        frac -= WINDOW_HEIGHT;
    }

    if (SDL_BlitSurface(largesurface, &largesrcrect, screen, &destrect)) {
        printf("Failed blit: %s\n", SDL_GetError());
    }
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

#define TEXTURE_WIDTH   320
#define TEXTURE_HEIGHT  200

static unsigned char G_bytes32[TEXTURE_HEIGHT][TEXTURE_WIDTH][4];
static void *create_texture_32bit(void)
{
    unsigned char *p_bytes = G_bytes32[0][0];
    unsigned char *p = p_bytes;
    int x, y;

    for (y=0; y<TEXTURE_HEIGHT; y++) {
        for (x=0; x<TEXTURE_WIDTH; x++, p+=4) {
            if ((x==0) || (x==(TEXTURE_WIDTH-1)) || (y==0) || (y==(TEXTURE_HEIGHT-1))) {
                //edge of square
                // red
                p[0] = 255;
                p[1] = 255;
                p[2] = 255;
                p[3] = 255;
            } else {
                // internal of square
                // black
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
                p[3] = 255;
            }
        }
    }
    G_bytes32[0][0][1] = 0;
    G_bytes32[0][0][2] = 0;
    G_bytes32[1][0][1] = 0;
    G_bytes32[1][0][2] = 0;
    G_bytes32[0][1][1] = 0;
    G_bytes32[0][1][2] = 0;

    G_bytes32[0][31][1] = 0;
    G_bytes32[0][31][2] = 0;

    G_bytes32[TEXTURE_HEIGHT-1][TEXTURE_WIDTH-1][1] = 0;
    G_bytes32[TEXTURE_HEIGHT-1][TEXTURE_WIDTH-1][2] = 255;

    return p_bytes;
}

static GLuint G_texture;
static void create_texture(void)
{
    void *p_texture = create_texture_32bit();

    // Create an unpacked alignment 1 structure
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, &G_texture);
    glBindTexture (GL_TEXTURE_2D, G_texture);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#if 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST /* GL_NEAREST */);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST /* GL_NEAREST */);
#elif 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR /* GL_NEAREST */);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR /* GL_NEAREST */);
#elif 0
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR /* GL_NEAREST */);
#elif 1
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR /* GL_NEAREST */);
#else
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

//    gluBuild2DMipmaps (GL_TEXTURE_2D, 4, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, p_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, p_texture);
    glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static void setup_opengl( int width, int height )
{
    float ratio = (float) width / (float) height;

    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Culling. */
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );

    /* Set the clear color. */
    glClearColor( 0xFF, 0xFF, 0xFF, 0 );

    create_texture();
    /* Setup our viewport. */
    glViewport( 0, 0, width, height );

    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    /*
     * EXERCISE:
     * Replace this with a call to glFrustum.
     */
    gluPerspective( 90.0, ratio, 1.0, 1024.0 );
}


unsigned char G_render1_pixels[200][320][4];
static void Render1(unsigned char *p_screen, unsigned char *palette)
{
    int x, y;
    unsigned int v;

    for (y=0; y<200; y++) {
        for (x=0; x<320; x++, p_screen++) {
            v = *p_screen;
            v *= 3;
            G_render1_pixels[y][x][0] = ((palette[v+0]&0x3F)<<2);
            G_render1_pixels[y][x][1] = ((palette[v+1]&0x3F)<<2);
            G_render1_pixels[y][x][2] = ((palette[v+2]&0x3F)<<2);
            if (v == 255*3)
                G_render1_pixels[y][x][3] = 0;
            else
                G_render1_pixels[y][x][3] = 255;

        }
    }
#if 0
    for (y=0; y<200; y++) {
        p_screen = G_render1_pixels[y][0];
        memset(p_screen, y, 320*4);
    }
//memset(G_render1_pixels, 0x80, sizeof(G_render1_pixels));
#endif
}

#if RENDER_OPENGL
void WindowsRenderOpenGL(char *p_screen, unsigned char *palette)
{
#if 1
    GLuint colorsR[256];
    GLuint colorsG[256];
    GLuint colorsB[256];
    GLuint colorsA[256];
    int i;
    GLuint v;
#endif
    static GLfloat rect3D[4][3] = {
        { 0, 0, 0 },
        { 0, WINDOW_HEIGHT, 0 },
        { WINDOW_WIDTH, WINDOW_HEIGHT, 0 },
        { WINDOW_WIDTH, 0, 0 }
    };

    // Setup the color palette for this update
#if 1
    for (i=0; i<256; i++) {
#if 0
        colorsR[i] = ((((unsigned int)*(palette++))&0x3F)<<2);
        colorsG[i] = ((((unsigned int)*(palette++))&0x3F)<<2);
        colorsB[i] = ((((unsigned int)*(palette++))&0x3F)<<2);
        colorsA[i] = 0xFF;
#else
        v = i;
        colorsR[i] =(v << 8) | v;
        colorsG[i] = (v << 8) | v;
        colorsB[i] = (v << 8) | v;
        colorsA[i] = i|0x8080;
#endif
    }
#endif

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_TEXTURE_2D);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -100 ,100);
    glDisable (GL_DEPTH_TEST);

    glBindTexture (GL_TEXTURE_2D, G_texture);
    glEnable( GL_TEXTURE_2D );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
#if 1
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
#else
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif
    // Setup pixel mappings
#if 0
    glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
    glPixelMapuiv( GL_PIXEL_MAP_I_TO_R, 256, colorsR );
    glPixelMapuiv( GL_PIXEL_MAP_I_TO_G, 256, colorsG );
    glPixelMapuiv( GL_PIXEL_MAP_I_TO_B, 256, colorsB );
    glPixelMapuiv( GL_PIXEL_MAP_I_TO_A, 256, colorsA );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0,
                   GL_COLOR_INDEX, GL_UNSIGNED_BYTE, (void *)p_screen );
    //glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
#else
    Render1(p_screen, palette);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, &G_render1_pixels[0][0][0]);
#endif
glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex3fv( rect3D[0] );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex3fv( rect3D[1] );
    glTexCoord2f( 1.0f, 1.0f);
    glVertex3fv( rect3D[2] );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex3fv( rect3D[3] );
    glEnd( );
    glDisable( GL_TEXTURE_2D );

     //printf( "error: [%s]\n", SDL_GetError( ) );
    glFlush ();
    SDL_GL_SwapBuffers( );

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
#endif

void WindowsUpdate(char *p_screen, unsigned char *palette)
{
    static int lastFPS = 0;
    static int fps = 0;
    T_word32 tick = clock();
    static T_word32 lastTick = 0xFFFFEEEE;
    static double movingAverage = 0;

#if CAP_SPEED_TO_FPS
        if ((tick-lastTick)<(1000/CAP_SPEED_TO_FPS)) {
Sleep((1000/CAP_SPEED_TO_FPS) - (tick-lastTick));
        // 10 ms between frames (top out at 100 ms)
    } else
#endif
    {
        lastTick = tick;

#if RENDER_OPENGL
        WindowsRenderOpenGL(p_screen, palette);
#else
        WindowsRenderSDL(p_screen, palette);
#endif
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
    int flags;
    SDL_Color black = { 0, 0, 0, 0 };
    SDL_Color white = { 255, 255, 255, 0 };
    //SDL_Surface* icon;

    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
    {
          printf ("Could not initialize SDL: %s\n",SDL_GetError());
          return 1;
    }

    atexit(SDL_Quit);

#if AA_OPENGL
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
#endif

    flags = SDL_HWSURFACE|SDL_DOUBLEBUF;
#ifdef AA_OPENGL
    flags |= SDL_OPENGL;
#else
#ifdef NDEBUG
    flags |= SDL_FULLSCREEN;
#endif
#endif
    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, flags);
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
    largesurface = SDL_CreateRGBSurface(SDL_SWSURFACE, WINDOW_WIDTH, WINDOW_HEIGHT, 8, 0, 0, 0, 0);
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
#if AA_OPENGL
    setup_opengl(WINDOW_WIDTH, WINDOW_HEIGHT);
#endif
        game_main(argc, argv);
//    test_main(argc, argv);
    }

    return 0;
}

