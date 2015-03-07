/****************************************************************************/
/*    FILE:  PICS.H                                                         */
/****************************************************************************/
#ifndef _PICS_H_
#define _PICS_H_

#include "GENERAL.H"
#include "GRAPHICS.H"
#include "RESOURCE.H"

#define PICTURE_RESOURCE_FILENAME "PICS.RES"

T_void PicturesInitialize(T_void) ;

T_void PicturesFinish(T_void) ;

T_byte8 *PictureLock(T_byte8 *name, T_resource *res) ;

T_byte8 *PictureLockData(const char *name, T_resource *res) ;

//T_byte8 *PictureLockPNG(const T_byte8 *name, T_resource *res);

T_void PictureUnlock(T_resource res) ;

#define PictureUnlockData(res)  ResourceUnlock(res)

E_Boolean PictureExist(const char *name) ;

T_void PictureGetXYSize(T_void *p_picture, T_word16 *sizeX, T_word16 *sizeY) ;

T_resource PictureFind(const char *name) ;

T_void PictureUnfind(T_resource res) ;

T_void PictureUnlockAndUnfind(T_resource res) ;

T_byte8 *PictureLockDataQuick(T_resource res) ;

T_byte8 *PictureLockByResource(T_resource res) ;

T_bitmap *PictureToBitmap(T_byte8 *pic) ;

T_word16 PictureGetWidth(T_void *p_picture) ;

T_word16 PictureGetHeight(T_void *p_picture) ;

T_byte8 *PictureGetName(T_void *p_picture) ;

#ifndef NDEBUG
T_void PicturePrint(FILE *fp, T_void *p_pic) ;
T_void PicturesDump(T_void) ;
T_void PictureCheck(T_void *p_picture) ;
#else
#define PicturePrint(fp, p_pic)
#define PicturesDump()
#define PictureCheck(p_pic)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  PICS.H                                                  */
/****************************************************************************/
