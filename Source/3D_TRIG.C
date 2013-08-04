/*-------------------------------------------------------------------------*
 * File:  3D_TRIG.C
 *-------------------------------------------------------------------------*/
/**
 * All of the 3D Math goes here.
 *
 * @addtogroup _3D_TRIG
 * @brief 3D Math routines
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_TRIG.H"
#include "FILE.H"

/** I want my own constants for pi and pi/2. **/
#ifdef M_PI
#  undef M_PI
#endif

#ifdef M_PI_2
#  undef M_PI_2
#endif

#define M_PI        (3.14159265358979323846)
#define M_PI_2      (1.57079632679489661923)

/* Lookup tables for previous calculated information. */
/*
T_sword32 *G_sinTable ;
T_sword32 *G_tanTable ;
T_sword32 *G_cosTable ;
T_sword32 *G_invCosTable ;
T_word16 *G_arcTanTable ;
T_word16 *G_distanceTable ;
*/
//T_byte8 *G_palIndex ;

T_sword32 G_sinTable[MATH_MAX_ANGLE];
T_sword32 G_tanTable[MATH_MAX_ANGLE];
T_sword32 G_cosTable[MATH_MAX_ANGLE];
T_sword32 G_invCosTable[MATH_MAX_ANGLE];
T_word16 G_arcTanTable[256][256] ;
T_byte8 G_translucentTable[256][256] ;

//TAKE OUT         T_word16 G_distanceTable[256][256] ;

T_sword32 G_viewTanTable[MAX_VIEW3D_WIDTH] ;
T_sword32 G_invDistTable[MATH_MAX_DISTANCE];
T_byte8 G_power2table[257] ;

T_void IMathPower2Init(T_void) ;
T_void ISetupViewTable(T_void) ;

//extern T_byte8 *P_shadeIndex ;

/*-------------------------------------------------------------------------*
 * Routine:  MathCosine
 *-------------------------------------------------------------------------*/
/**
 *  Calculates 32768 * cos(angle) and returns in integer format.
 *
 *  NOTE:
 *  This routine is only called during the trig
 *  initialization process.
 *
 *  @param angle -- Degrees of angle to report sine of
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 MathCosine(T_word16 angle)
{
/*
    T_float64 radians = (((T_float64)angle) * M_PI) / (T_float64)32768.0 ;

    return (T_sword32)(cos(radians) * (T_float64)65536.0) ;
*/
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathSine
 *-------------------------------------------------------------------------*/
/**
 *  Calculates 32768 * sin(angle) and returns in integer format.
 *
 *  NOTE:
 *  This routine is only called during the trig
 *  initialization process.
 *
 *  @param angle -- Degrees of angle to report sine of
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 MathSine(T_word16 angle)
{
/*
    T_float64 radians = (((T_float64)angle) * M_PI) / (T_float64)32768.0 ;

    return (T_sword32)(sin(radians) * (T_float64)65536.0) ;
*/
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathTangent
 *-------------------------------------------------------------------------*/
/**
 *  Calculates 32768 * tan(angle) and returns in integer format.
 *
 *  NOTE:
 *  This routine is only called during the trig
 *  initialization process.
 *
 *  @param angle -- Degrees of angle to report sine of
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 MathTangent(T_word16 angle)
{
/*
    T_float64 radians = (((T_float64)angle) * M_PI) / (T_float64)32768.0 ;

    return (T_sword32)(tan(radians) * (T_float64)65536.0) ;
*/
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathInitializeInvDistTable
 *-------------------------------------------------------------------------*/
/**
 *  MathInitializeInvDistTable calculates the inverse distance table
 *  based on the given width of the screen.  All perspective calculations
 *  are based on this table instead of using division during the rendering
 *  process.  Call this routine at any time the width of the view is
 *  changed.
 *
 *  @param screenWidth -- Width of the screen
 *
 *<!-----------------------------------------------------------------------*/
T_void MathInitializeInvDistTable(T_word32 screenWidth)
{
    T_word32 numerator = (screenWidth/2)<<16 ;
    T_word32 distance ;

    for (distance=0;  distance<MATH_MAX_DISTANCE; distance++)  {
        /* The one is added to dist in the denomintator */
        /* to avoid divide by zero ... and it doesn't affect */
        /* the calculations much. */
        G_invDistTable[distance] = numerator / (distance + 1) ;
    }
}

T_void MathInitializeOld(T_word32 screenWidth)
{
    FILE *fp ;

    DebugRoutine("MathInitialize") ;

    MathInitializeInvDistTable(screenWidth) ;

    /* Calculate each one for each angle. */
/*
    for (angle=0; angle<MATH_MAX_ANGLE; angle++)  {
        G_sinTable[angle] = MathSine(angle * MATH_ANGLE_ACCURACY) ;
        G_cosTable[angle] = MathCosine(angle * MATH_ANGLE_ACCURACY) ;
        G_tanTable[angle] = MathTangent(angle * MATH_ANGLE_ACCURACY) ;
        G_invCosTable[angle] = MathInvCosine(angle * MATH_ANGLE_ACCURACY) ;
    }

    fp = fopen("cosine.dat", "wb") ;
    fwrite(G_cosTable, sizeof(G_cosTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("sine.dat", "wb") ;
    fwrite(G_sinTable, sizeof(G_sinTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("tangent.dat", "wb") ;
    fwrite(G_tanTable, sizeof(G_tanTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("invcos.dat", "wb") ;
    fwrite(G_invCosTable, sizeof(G_invCosTable), 1, fp) ;
    fclose(fp) ;
*/

    fp = fopen("cosine.dat", "rb") ;
DebugCheck(fp != NULL) ;
    fread(G_cosTable, sizeof(G_cosTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("sine.dat", "rb") ;
DebugCheck(fp != NULL) ;
    fread(G_sinTable, sizeof(G_sinTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("tangent.dat", "rb") ;
DebugCheck(fp != NULL) ;
    fread(G_tanTable, sizeof(G_tanTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("arctan.dat", "rb") ;
DebugCheck(fp != NULL) ;
    fread(G_arcTanTable, sizeof(G_arcTanTable), 1, fp) ;
    fclose(fp) ;

    fp = fopen("invcos.dat", "rb") ;
DebugCheck(fp != NULL) ;
    fread(G_invCosTable, sizeof(G_invCosTable), 1, fp) ;
    fclose(fp) ;
    ISetupViewTable() ;

    IMathPower2Init() ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathInvCosine
 *-------------------------------------------------------------------------*/
/**
 *  Calculates 1/(65536 * cos(angle)) and returns in integer format.
 *
 *  NOTE:
 *  This routine is only called during the trig
 *  initialization process.
 *
 *  @param angle -- Degrees of angle to report sine of
 *
 *  @return 1/(65536 * cos(angle))
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 MathInvCosine(T_word16 angle)
{
/*
    T_float64 radians = (((T_float64)angle) * M_PI) / (T_float64)32768.0 ;

    if (cos(radians) == 0)
        return((T_sword32)65536) ;

    return((T_sword32) (((T_sword32)65536) / cos(radians))) ;
*/
    return 0 ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathXTimesCosAngle
 *-------------------------------------------------------------------------*/
/**
 *  Computes x * cos(angle)
 *
 *  @param x -- Multiplier
 *  @param angle -- Angle
 *
 *  @return x * cos(angle)
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 MathXTimesCosAngle(
              T_sword16 x,
              T_word16 angle)
{
    T_sword32 tx = x ;

    return ((T_sword16) ((tx * MathCosineLookup(angle)) >> 16)) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathXTimesSinAngle
 *-------------------------------------------------------------------------*/
/**
 *  Computes x * sin(angle)
 *
 *  @param x -- Multiplier
 *  @param angle -- Angle
 *
 *  @return x * sin(angle)
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 MathXTimesSinAngle(
              T_sword16 x,
              T_word16 angle)
{
    T_sword32 tx = x ;

    return((T_sword16) ((tx * MathSineLookup(angle)) >> 16)) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ISetupViewTable
 *-------------------------------------------------------------------------*/
/**
 *  ISetupViewTable
 *
 *<!-----------------------------------------------------------------------*/
T_void ISetupViewTable(T_void)
{
    T_sword32 i ;

/*
    for (i=0; i<VIEW3D_WIDTH; i++)  {
        angle = ((i-VIEW3D_HALF_WIDTH)*MATH_ANGLE_45)/VIEW3D_HALF_WIDTH ;
        if (angle < 0)
            angle += MATH_ANGLE_360 ;
        G_viewTanTable[i] = -G_tanTable[angle] ;
    }
*/
    for (i=-VIEW3D_HALF_WIDTH; i<VIEW3D_HALF_WIDTH; i++)  {
        G_viewTanTable[i+VIEW3D_HALF_WIDTH] = -(i * 65536)/VIEW3D_HALF_WIDTH ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  MathArcTangent
 *-------------------------------------------------------------------------*/
/**
 *  MathArcTangent takes the given x and y deltas and determines
 *  the atan (using this system's theta values).
 *
 *  @param y -- Numerator of tangent fraction
 *  @param x -- Denomator of tangent fraction
 *
 *  @return Angle result
 *
 *<!-----------------------------------------------------------------------*/
T_word16 MathArcTangent(T_sword32 y, T_sword32 x)
{
    /* Now both x and y must be less than 128 for table lookup. */
    while ((x > 127) || (x < -127) || (y > 127) || (y < -127))  {
        x >>= 1 ;
        y >>= 1 ;
    }

    /* Return that angle. */
    return (G_arcTanTable[y+128][x+128]) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathArcTangent32
 *-------------------------------------------------------------------------*/
/**
 *  MathArcTangent32 is a 32 bit version of MathArcTangent
 *
 *  @param y -- Numerator of tangent fraction
 *  @param x -- Denomator of tangent fraction
 *
 *  @return Angle result
 *
 *<!-----------------------------------------------------------------------*/
T_word16 MathArcTangent32(T_sword32 y, T_sword32 x)
{
    /* Now both x and y must be less than 128 for table lookup. */
    while ((x > 127) || (x < -127) || (y > 127) || (y < -127))  {
        x >>= 1 ;
        y >>= 1 ;
    }

    /* Return that angle. */
    return (G_arcTanTable[y+128][x+128]) ;
}

T_word16 MathArcTangentOld(T_sword16 y, T_sword16 x)
{
    T_sword16 sign ;
    T_word16 angle ;

    /* Determine the sign of the fraction. */
    if (((y < 0) && (x >=0)) || ((x < 0) && (y >= 0)))  {
        sign = -1 ;
    } else {
        sign = 1 ;
    }

    /* Positives only, please. */
    if (x < 0)
        x = -x ;
    if (y < 0)
        y = -y ;

    /* Now both x and y must be less than 128 for table lookup. */
    while ((x & 0xFF80) || (y & 0xFF80))  {
        x >>= 1 ;
        y >>= 1 ;
    }

    /* Get the angular value. */
    angle = G_arcTanTable[y][x] ;

    /* If the sign is negative, flip the angle over the 360 mark. */
    if (sign == -1)
        if (angle != 0)
            angle = MATH_MAX_ANGLE-angle ;

    /* Return that angle. */
    return (angle<<6) ;
}

T_void IMathPower2Init(T_void)
{
    T_sword16 i, power, next ;

    G_power2table[0] = 0 ;
    for (i=1, power=-1, next=1; i<257; i++)  {
        if (next == i)  {
            power++ ;
            next <<= 1 ;
        }
        G_power2table[i] = (T_byte8)power ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  MathInitialize
 *-------------------------------------------------------------------------*/
/**
 *  MathInitialize creates all the lookup tables used by the math
 *  routines.  Some of the tables (well, one) needs the width of the
 *  screen as well.  Pass this in to the routine.
 *  This routine MUST be called before all other routines are used.
 *
 *  @param screenWidth -- Width of view screen.
 *
 *<!-----------------------------------------------------------------------*/
T_void MathInitialize(T_word32 screenWidth)
{
    T_file fh ;

    DebugRoutine("MathInitialize") ;

    /* Set up an inverse distance table (pretty much a 1/Z table). */
    MathInitializeInvDistTable(screenWidth) ;

//    G_arcTanTable = FileLoad("mdat.res", &size) ;
    fh = FileOpen("mdat.res", FILE_MODE_READ) ;
    if (fh == FILE_BAD)  {
        puts("Cannot load MDAT.RES!") ;
//        DebugRoutine(FALSE) ;
        exit(1) ;
    } else {
        /* Reindex all the pointers off of the first pointer. */
        FileRead(fh, G_arcTanTable, sizeof(G_arcTanTable)) ;
        FileRead(fh, G_cosTable, sizeof(G_cosTable)) ;
        FileRead(fh, G_invCosTable, sizeof(G_invCosTable)) ;
        FileRead(fh, P_shadeIndex, sizeof(P_shadeIndex)) ;
        FileRead(fh, G_sinTable, sizeof(G_sinTable)) ;
        FileRead(fh, G_tanTable, sizeof(G_tanTable)) ;
        FileRead(fh, G_translucentTable, sizeof(G_translucentTable)) ;
//TAKE OUT        FileRead(fh, G_distanceTable, sizeof(G_distanceTable)) ;
        FileClose(fh) ;
/*
        G_cosTable = (T_sword32 *)(((T_byte8 *)G_arcTanTable) +
                      sizeof(T_word16) * 256 * 256) ;
        G_invCosTable = (T_sword32 *)(((T_byte8 *)G_cosTable) +
                      sizeof(T_sword32) * MATH_MAX_ANGLE) ;
        P_shadeIndex = ((T_byte8 *)G_invCosTable) +
                       sizeof(T_sword32) * MATH_MAX_ANGLE ;
        G_sinTable = (T_sword32 *)(P_shadeIndex + 256 * 64) ;
        G_distanceTable = (T_word16 *)(((T_byte8 *)G_sinTable) +
                          sizeof(T_sword32) * MATH_MAX_ANGLE) ;
        G_tanTable = (T_sword32 *)(((T_byte8 *)G_distanceTable) +
                     sizeof(T_word16) * 256 * 256) ;
*/

        /* Setup the angular values for the view table. */
        ISetupViewTable() ;

        /* Setup the power of 2 table. */
        IMathPower2Init() ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MathFinish
 *-------------------------------------------------------------------------*/
/**
 *  MathFinish is called when the lookup tables are no longer needed.
 *  They are all unloaded.
 *
 *<!-----------------------------------------------------------------------*/
T_void MathFinish(T_void)
{
    DebugRoutine("MathFinish") ;

    /* All we have to do is free up the memory used by Math. */
//    MemFree(G_arcTanTable) ;

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  3D_TRIG.C
 *-------------------------------------------------------------------------*/
