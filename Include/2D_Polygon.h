#ifndef _2D_POLYGON_H_
#define _2D_POLYGON_H_

typedef double T_2DType;

typedef struct {
    T_2DType iX;
    T_2DType iY;
} T_2DCoord;

typedef struct _T_2DPolygonPoint {
    struct _T_2DPolygonPoint *iNext;
    struct _T_2DPolygonPoint *iPrevious;
    T_2DCoord iCoord;
} T_2DPolygonPoint;

typedef struct {
    T_2DPolygonPoint *iFirst;
    T_2DPolygonPoint *iLast;
} T_2DPolygon;

typedef struct {
    T_2DCoord iStart;
    T_2DCoord iDelta;
} T_2DVector;

T_2DPolygon *PolygonCreate(void);
void PolygonDestroy(T_2DPolygon *aPolygon);
void PolygonAddPoint(T_2DPolygon *aPolygon, T_2DCoord aCoord);
T_2DPolygon *PolygonCopy(T_2DPolygon *aPolygon);
T_2DPolygon *PolygonClip(T_2DPolygon *aPolygon, T_2DVector *aVector);
unsigned int PolygonGetNumPoints(T_2DPolygon *aPolygon);

#endif /* _2D_POLYGON_H_ */
