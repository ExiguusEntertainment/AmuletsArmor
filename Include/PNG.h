/*
 * PNG.h
 *
 *  Created on: Mar 4, 2015
 *      Author: lshields
 */

#ifndef PNG_H_
#define PNG_H_

#include "GENERAL.H"
#include "GRAPHICS.H"

typedef void *T_png;
#define PNG_BAD     0

void PNGInit(void);
void PNGFinish(void);
T_png PNGLock(const char *aName);
void PNGUnlock(T_png aPNG);
void PNGRelease(T_png aPNG);
void PNGReleaseUnlocked(void);
void PNGGetSize(T_png aPNG, T_word16 *sizeX, T_word16 *sizeY);
T_bitmap *PNGGetBitmap(T_png aPNG);

#endif /* PNG_H_ */
