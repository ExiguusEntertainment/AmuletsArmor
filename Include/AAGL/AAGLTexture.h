#ifndef AAGLTEXTURE_H_
#define AAGLTEXTURE_H_

#include <general.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

GLuint AAGLTextureCreateMipMapFromPIC(T_byte8 *aPIC);
GLuint AAGLFindTexture(T_byte8 *aPicture);
void AAGLTextureGetSize(GLuint textureID, T_sword16 *sizeX, T_sword16 *sizeY);

#endif /* AAGLTEXTURE_H_ */
