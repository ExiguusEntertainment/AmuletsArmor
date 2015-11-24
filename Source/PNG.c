/*
 * PNG.c
 *
 *  Created on: Mar 4, 2015
 *      Author: lshields
 */
#include <stdlib.h>
#include "PNG.h"
#include "MEMORY.h"
#include "RESOURCE.h"
#include "PICS.h"
#include <lodepng.h>

typedef struct _T_pngStruct {
    struct _T_pngStruct *iNext; //!< Link to next loaded PNG in list
    struct _T_pngStruct *iPrevious; //!< Link to previously loaded PNG in list
    char *iName;                //!< Look up of name
    T_resource iResource;       //!< Resource handle to compressed PNG image
    void *iData;                //!< Compressed PNG image (stored elsewhere)
    unsigned char *iPNG;        //!< 32-bit raw image (RGBA format)
    void *iBitmap;              //!< Converted to indexed color with bitmap header
    unsigned int iWidth;        //!< Width of image in pixels
    unsigned int iHeight;       //!< Height of image in pixels
    T_word32 iLockCount;        //!< Number of locks on this PNG
} T_pngStruct;

static T_pngStruct *G_pngList = 0;
static int G_paletteInt[256][3];

/*-------------------------------------------------------------------------*
 * Routine:  IPNGAlloc
 *-------------------------------------------------------------------------*/
/**
 *  Allocate a PNG internal structure and put on the list of PNGs.
 *
 *  @param aName -- Name in PICS resource file
 *  @param res -- Resource handle for this PNG
 *  @param aData -- Handle to data
 *
 *  @return Pointer to internal png structure
 *
 *<!-----------------------------------------------------------------------*/
static T_pngStruct *IPNGAlloc(const char *aName, T_resource res, void *aData)
{
    T_pngStruct *p;

    DebugRoutine("IPNGAlloc");
    DebugCheck(aName != 0);
    DebugCheck(res != RESOURCE_BAD);

    // Allocate and initialize the data
    p = (T_pngStruct *)MemAlloc(sizeof(T_pngStruct));
    DebugCheck(p != 0);
    memset(p, 0, sizeof(T_pngStruct));
    p->iName = MemAlloc(strlen(aName)+1);
    DebugCheck(p->iName != 0);
    strcpy(p->iName, aName);
    p->iResource = res;
    p->iLockCount = 0;
    p->iData = aData;

    // Insert at the front of the list
    p->iNext = G_pngList;
    if (G_pngList)
        G_pngList->iPrevious = p;
    G_pngList = p;

    DebugEnd();

    return p;
}

/*-------------------------------------------------------------------------*
 * Routine:  IPNGFree
 *-------------------------------------------------------------------------*/
/**
 *  Free the PNG from memory
 *
 *  @param p -- PNG structure and data to free
 *
 *<!-----------------------------------------------------------------------*/
static void IPNGFree(T_pngStruct *p)
{
    DebugRoutine("IPNGFree");
    DebugCheck(p != 0);

    MemFree(p->iName);
    p->iName = 0;
    if (p->iBitmap) {
        MemFree(p->iBitmap);
        p->iBitmap = 0;
    }

    PictureUnlockData(p->iResource);
    p->iData = 0;
    p->iResource = RESOURCE_BAD;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  IPNGFindClosestColor
 *-------------------------------------------------------------------------*/
/**
 *  Convert a 32-bit RGBA (4 8-bit values) into an index palette entry.
 *
 *  @param p -- PNG to convert from uncompressed 32-bit to 8-bit indexed.
 *
 *<!-----------------------------------------------------------------------*/
static unsigned char IPNGFindClosestColor(unsigned char *aRGBA)
{
    int dr, dg, db;
    unsigned int distance;
    unsigned char closest = 0;
    unsigned int closestDistance = 256*256+256*256+256*256;
    unsigned int i;

    // If no alpha, return index 0 automatically
    if (aRGBA[3] == 0)
        return 0;

    // We'll measure the distance to each color and determine
    // the closest color.
    // NOTE: We're excluding the last 32 colors since they were animated
    // colors.
    for (i=0; i<256-32; i++) {
        int *p_rgb = G_paletteInt[i];
        dr = ((int)aRGBA[0]) - p_rgb[0];
        dr *= dr;
        dg = ((int)aRGBA[1]) - p_rgb[1];
        dg *= dg;
        db = ((int)aRGBA[2]) - p_rgb[2];
        db *= db;
        distance = dr+dg+db;
        if (distance < closestDistance) {
            closestDistance = distance;
            closest = i;
        }
    }

    return closest;
}

/*-------------------------------------------------------------------------*
 * Routine:  IPNGCreateIndexedColorBitmap
 *-------------------------------------------------------------------------*/
/**
 *  For now, we're going to downgrade the PNGs until the rest of the
 *  system is up and running.  This code converts a 32-bit raw PNG into
 *  an indexed 8-bit version.
 *
 *  @param p -- PNG to convert from uncompressed 32-bit to 8-bit indexed.
 *
 *<!-----------------------------------------------------------------------*/
static void IPNGCreateIndexedColorBitmap(T_pngStruct *p)
{
    T_palette palette;
    T_bitmap *p_bitmap;
    unsigned char *p_pixel;
    unsigned int x;
    unsigned int y;
    unsigned char *p_rgb;

    DebugRoutine("IPNGCreateBitmap");
    DebugCheck(p != 0); // Good pointer?
    DebugCheck(p->iBitmap == 0);  // No bitmap yet?
    DebugCheck(p->iPNG != 0);  // Have PNG?
    DebugCheck(p->iWidth < 65536); // Too wide?
    DebugCheck(p->iHeight < 65536); // Too tall?

    if ((p->iBitmap==0) && (p->iPNG != 0)) {
        // Now convert the indexed color bitmap

        // Get the graphics palette (colors in this palette are 0-63 rgb)
        GrGetPalette(0, 256, palette);

        // Allocate memory for the full bitmap
        p_bitmap = (T_bitmap *)MemAlloc(sizeof(T_word16)*2 + (p->iWidth * p->iHeight));
        DebugCheck(p_bitmap != 0);
        if (p_bitmap) {
            p->iBitmap = p_bitmap;
            p_bitmap->sizex = (T_word16)p->iWidth;
            p_bitmap->sizey = (T_word16)p->iHeight;

            // Now let's recreate the data.  For this type of bitmap,
            // there are no alpha channels, but if we get a blank cell,
            // we'll still skip it and use the default of index of 0
            // for those.
            p_pixel = p_bitmap->data;
            p_rgb = p->iPNG;

            for (y=0; y<p->iHeight; y++) {
                for (x=0; x<p->iWidth; x++) {
                    *p_pixel = IPNGFindClosestColor(p_rgb);

                    // Next pixel
                    p_pixel++;
                    p_rgb += 4;
                }
            }
        }
    }

    DebugEnd();
}
/*-------------------------------------------------------------------------*
 * Routine:  IPNGFind
 *-------------------------------------------------------------------------*/
/**
 *  Walk the list of PNGs and find a match
 *
 *  @param aName -- Name of PNG to find in Pics database  (from root directory, eg.
 *      Textures/DRK42.png).  Case sensitive.
 *
 *  @return PNG handle
 *<!-----------------------------------------------------------------------*/
static T_pngStruct *IPNGFind(const char *aName)
{
    T_pngStruct *p;

    DebugRoutine("IPNGFind");
    DebugCheck(aName != 0);

    for (p=G_pngList; p; p=p->iNext) {
        if (strcmp(p->iName, aName) == 0)
            break;
    }

    DebugEnd();

    return p;
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGLock
 *-------------------------------------------------------------------------*/
/**
 *  Lock a PNG from the PICS database into memory.
 *
 *  @param aName -- Name of PNG to lock in Pics database (from root directory, eg.
 *      Textures/DRK42.png).  Case sensitive.
 *
 *  @return PNG handle
 *<!-----------------------------------------------------------------------*/
T_png PNGLock(const char *aName)
{
    T_pngStruct *p;
    void *p_data;
    unsigned char *p_pixels;

    DebugRoutine("PNGLock");
    DebugCheck(aName != 0);

printf("PNGLock: %s\n", aName);
    // Already locked?  If so, return that instead
    p = IPNGFind(aName);
    if (!p) {
        // PNG is not already locked, let's see if there is even a resource
        // out there for it.
        T_resource res;
        p_data = PictureLockData(aName, &res);
        if (p_data) {
            // Create the new entry (and we'll return that)
            p = IPNGAlloc(aName, res, p_data);
            DebugCheck(p != 0);

            // Now convert the image from PNG to raw 32-bit data
            if (p) {
                unsigned error = lodepng_decode32(&p_pixels, &p->iWidth,
                        &p->iHeight, (const unsigned char *)p_data,
                        ResourceGetSize(res));
                if (error == 0) {
                    p->iPNG = p_pixels;

                    // Now create an indexed color bitmap out of the PNG
                    IPNGCreateIndexedColorBitmap(p);
                } else {
                    const char *errorString = lodepng_error_text(error);
                    printf("PNG Error: name='%s': error=%s\n", aName, errorString);
                    DebugCheck(error == 0);
                }
            }
        } else {
            // Nothing found, return a bad handle
            p = PNG_BAD;
            // This should never happen?
            DebugCheck(p != PNG_BAD);
        }
    }

    // Now increment the lock count
    p->iLockCount++;

    DebugEnd();

    return (T_png)p;

}

/*-------------------------------------------------------------------------*
 * Routine:  PNGUnlock
 *-------------------------------------------------------------------------*/
/**
 *  Unlock a PNG so it can be freed later.
 *
 *  @param aPNG -- PNG to unlock
 *<!-----------------------------------------------------------------------*/
void PNGUnlock(T_png aPNG)
{
    T_pngStruct *p = (T_pngStruct *)aPNG;
    DebugRoutine("PNGUnlock");
    DebugCheck(aPNG != PNG_BAD);
printf("PNGUnlock: %s\n", p->iName);
    DebugCheck(p->iLockCount > 0);
    if (p->iLockCount > 0) {
        p->iLockCount--;
        if (p->iLockCount == 0) {
            // Do anything here?
        }
    }
    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGRelease
 *-------------------------------------------------------------------------*/
/**
 *  Release a PNG from memory (if it's not locked).
 *
 *  @param aPNG -- PNG to release
 *<!-----------------------------------------------------------------------*/
void PNGRelease(T_png aPNG)
{
    T_pngStruct *p = (T_pngStruct *)aPNG;

    DebugRoutine("PNGRelease");
    DebugCheck(aPNG != PNG_BAD);
    DebugCheck(p->iLockCount == 0);
    if (p->iLockCount == 0) {
        // Unlink this entry from the double linked list
        if (p->iPrevious)
            p->iPrevious->iNext = p->iNext;
        else
            G_pngList = p->iNext;
        if (p->iNext)
            p->iNext->iPrevious = p->iPrevious;

        // Now free the memory
        IPNGFree(p);
    }
    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGReleaseUnlocked
 *-------------------------------------------------------------------------*/
/**
 *  Go through all PNGs and release all the ones that are unlocked.
 *  This helps the system free graphics from memory that are not being
 *  used.
 *<!-----------------------------------------------------------------------*/
void PNGReleaseUnlocked(void)
{
    T_pngStruct *p;
    T_pngStruct *p_next;

    DebugRoutine("PNGReleaseUnlocked");

    // Just walk through all the PNGs and for each unlocked
    // entry, release it
    for (p=G_pngList; p; p=p_next) {
        // First, make sure we get the next one in the list before
        // this one possibly disappears
        p_next = p->iNext;

        // Is this one unlocked?  If so, we'll free it
        if (p->iLockCount == 0)
            PNGRelease(p);
    }
    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGInit
 *-------------------------------------------------------------------------*/
/**
 *  Initialize any settings needed to handled PNG files.
 *
 *<!-----------------------------------------------------------------------*/
void PNGInit(void)
{
    int i;
    unsigned char *p_rgb;
    T_palette *palette = 0;
    T_resource res = RESOURCE_BAD;

    DebugRoutine("PNGInit");
    palette = (T_palette *)PictureLockData("Palettes/VIEW00.PAL", &res);
    DebugCheck(palette != 0);
    if (palette) {
        for (i=0; i<256; i++) {
            p_rgb = (*palette)[i];
            G_paletteInt[i][0] = (p_rgb[0] & 63)<<2;
            G_paletteInt[i][1] = (p_rgb[1] & 63)<<2;
            G_paletteInt[i][2] = (p_rgb[2] & 63)<<2;
        }
        PictureUnlock(res);
        res = RESOURCE_BAD;
        palette = 0;
    } else {
        printf("Cannot find VIEW00.PAL!");
        exit(1);
    }

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGFinish
 *-------------------------------------------------------------------------*/
/**
 *  Release all resources by the PNG system.  Should only be called
 *  when all PNG resources have been unlocked.
 *
 *<!-----------------------------------------------------------------------*/
void PNGFinish(void)
{
    DebugRoutine("PNGFinish");

    // Release all PNGs
    PNGReleaseUnlocked();
    DebugCheck(G_pngList == 0); // All PNGs should now be unloaded

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGGetBitmap
 *-------------------------------------------------------------------------*/
/**
 *  Get the bitmap associated with this PNG.
 *
 *<!-----------------------------------------------------------------------*/
T_bitmap *PNGGetBitmap(T_png aPNG)
{
    T_bitmap *p_bitmap = 0;
    T_pngStruct *p = (T_pngStruct *)aPNG;

    DebugRoutine("PNGGetBitmap");
    DebugCheck(aPNG != PNG_BAD);

    // Get previously converted bitmap
    p_bitmap = p->iBitmap;
    DebugCheck(p_bitmap != 0);

    DebugEnd();

    return p_bitmap;
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGGetSize
 *-------------------------------------------------------------------------*/
/**
 *  Return the size in pixels of the given PNG
 *
 *  @param [in] aPNG -- PNG to get size of
 *  @param [out] sizeX -- Pointer to receive width, NULL if no pointer
 *  @param [out] sizeY -- Pointer to receive height, NULL if no pointer
 *
 *<!-----------------------------------------------------------------------*/
void PNGGetSize(T_png aPNG, T_word16 *sizeX, T_word16 *sizeY)
{
    T_pngStruct *p = (T_pngStruct *)aPNG;

    DebugRoutine("PNGGetSize");
    DebugCheck(aPNG != PNG_BAD);

    if (sizeX)
        *sizeX = (T_word16)p->iWidth;
    if (sizeY)
        *sizeY = (T_word16)p->iHeight;

    DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  PNGGetName
 *-------------------------------------------------------------------------*/
/**
 *  Return the stored name of a given PNG for "(PNG_BAD)" for none.
 *
 *  @param [in] aPNG -- PNG to get size of
 *
 *<!-----------------------------------------------------------------------*/
const char *PNGGetName(T_png aPNG)
{
    T_pngStruct *p = (T_pngStruct *)aPNG;
    const char *p_name;

    DebugRoutine("PNGGetSize");
    if (aPNG == PNG_BAD)
        p_name = "(PNG_BAD)";
    else
        p_name = p->iName;

    DebugEnd();

    return p_name;
}

