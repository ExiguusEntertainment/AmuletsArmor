#ifndef AA_SDL_NET_SHIM_H
#define AA_SDL_NET_SHIM_H

static inline int SDLNet_Init(void)
{
    return 0;
}

static inline const char *SDLNet_GetError(void)
{
    return "SDL_net disabled";
}

#endif
