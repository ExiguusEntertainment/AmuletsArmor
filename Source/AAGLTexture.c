/*
 * AAOpenGL.c
 *
 *  Created on: Sep 28, 2013
 *      Author: lshields
 */

#include <AAGL/AAGLTexture.h>
#include <DEBUG.H>
#include <GRAPHICS.H>
#include <MEMORY.H>
#include <PICS.H>

typedef struct _T_AAGLTexture {
    struct _T_AAGLTexture *p_next;
    T_byte8 *p_pic;
    T_word16 width;
    T_word16 height;
    GLuint textureID;
} T_AAGLTexture;
static T_AAGLTexture *G_AAGLTextures = 0;

// Search the existing list for a matching structure
GLuint AAGLFindTexture(T_byte8 *aPicture)
{
    T_AAGLTexture *p = G_AAGLTextures;

    // Walk the list of loaded textures looking for a match
    while (p) {
        // If there is a match, return the ID
        if (p->p_pic == aPicture)
            return p->textureID;
        p = p->p_next;
    }

    // None found? return 0
    return 0;
}

GLuint AAGLTextureCreateMipMapFromPIC(T_byte8 *aPicture)
{
    T_word16 sizeX, sizeY;
    GLuint textureID;
    T_palette palette;
    T_byte8 *rgba;
    T_word32 x, y;
    T_byte8 *p;
    T_byte8 *p_index;
    T_byte8 c;
    T_AAGLTexture *p_texture;

    DebugRoutine("AAGLTextureCreateMipMapFromPIC");

    // First, determine if this texture already exists
    textureID = AAGLFindTexture(aPicture);
    // If found, just return that one, it already exists
    if (textureID) {
        DebugEnd();
        return textureID;
    }

    // Determine how big this picture is
    PictureGetXYSize(aPicture, &sizeX, &sizeY);

    // Get the game palette
    GrGetPalette(0, 256, palette);

    // Create a working structure for the texture
    rgba = (T_byte8 *)MemAlloc((T_word32)sizeX * (T_word32)sizeY * 4UL);

    // Convert the indexed color picture into a full 24+alpha channel texture
    p = rgba;
    p_index = aPicture;
        for (y=0; y<sizeX; y++) {   
    for (x=0; x<sizeY; x++,  p_index++) {
#if 1
            p = rgba+(x*sizeX+y)*4;
            c = *p_index;
            // red
            p[0] = (palette[c][0]<<2);
            // green
            p[1] = (palette[c][1]<<2);
            // blue
            p[2] = (palette[c][2]<<2);
            // alpha
            p[3] = 255; // solid
#else
            // TEST PATTERN texture
            p = rgba+(x*sizeY+y)*4;
            if ((x==0) || (y==0) || (x==sizeX-1) || (y==sizeY-1)) {
                c = 0;
            } else {
                c = 255;
            }
                p[0]=p[1]=p[2] = c;
            p[3]=255;
#endif
        }
    }

    // Use the unpacked RGBA texture and create in the GL system
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, &textureID);
    glBindTexture (GL_TEXTURE_2D, textureID);
    gluBuild2DMipmaps (GL_TEXTURE_2D, 4, sizeY, sizeX, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    // Done with the working structure, release it
    MemFree(rgba);

    // Create another structure for this data and put in the list
    p_texture = (T_AAGLTexture *)MemAlloc(sizeof(T_AAGLTexture));
    DebugCheck(p_texture != NULL);
    p_texture->p_pic = aPicture;
    p_texture->textureID = textureID;
    p_texture->width = sizeY;
    p_texture->height = sizeX;
    p_texture->p_next = G_AAGLTextures;
    G_AAGLTextures = p_texture;

    DebugEnd();

    // If anyone cares, return the structure
    return textureID;
}

void AAGLTextureGetSize(GLuint textureID, T_sword16 *sizeX, T_sword16 *sizeY)
{
    T_AAGLTexture *p = G_AAGLTextures;
    DebugRoutine("AAGLTextureGetSize");

    *sizeX = 0;
    *sizeY = 0;

    // Walk the list of loaded textures looking for a match
    while (p) {
        // If there is a match, return the ID
        if (p->textureID == textureID) {
            *sizeX = p->width;
            *sizeY = p->height;
            break;
        }
        p = p->p_next;
    }

    DebugEnd();
}
