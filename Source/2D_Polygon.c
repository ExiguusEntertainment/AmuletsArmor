#include <GENERAL.H>
#include <2D_Polygon.h>
#include <MEMORY.H>

#define EPSILON ((T_2DType)0.0001)

T_2DPolygon *PolygonCreate(void)
{
    T_2DPolygon *p;

    DebugRoutine("PolygoneCreate");
    p = MemAlloc(sizeof(T_2DPolygon));
    p->iFirst = p->iLast = 0;
    DebugEnd();

    return p;
}

void PolygonDestroy(T_2DPolygon *aPolygon)
{
    T_2DPolygonPoint *p;
    T_2DPolygonPoint *p_next;

    DebugRoutine("PolygonDestroy");

    if (aPolygon) {
        p = aPolygon->iFirst;
        while (p) {
            p_next = p->iNext;
            p->iPrevious = p->iNext = 0;
            MemFree(p);
            p = p_next;
        }
        aPolygon->iFirst = aPolygon->iLast = 0;
        MemFree(aPolygon);
    }

    DebugEnd();
}

void PolygonAddPoint(T_2DPolygon *aPolygon, T_2DCoord aCoord)
{
    T_2DPolygonPoint *p;

    DebugRoutine("PolygonAddPoint");

    p = MemAlloc(sizeof(T_2DPolygonPoint));
    DebugCheck(p != NULL);
    p->iCoord = aCoord;
    p->iNext = 0;
    p->iPrevious = aPolygon->iLast;
    if (aPolygon->iLast)
        aPolygon->iLast->iNext = p;
    aPolygon->iLast = p;
    if (!aPolygon->iFirst)
        aPolygon->iFirst = p;

    DebugEnd();
}

int IVectorIntersection(T_2DVector *aVector, T_2DCoord aFrom, T_2DCoord aTo, T_2DCoord *aIntersection)
{
    T_2DType x1, y1, x2, y2, x3, y3, x4, y4;
    T_2DType bx, by, dx, dy, cx, cy;
    double dot;
    double t;
    //double u;

    DebugRoutine("IVectorIntersection");

    x1 = aVector->iStart.iX;
    x2 = x1 + aVector->iDelta.iX;
    y1 = aVector->iStart.iY;
    y2 = y1 + aVector->iDelta.iY;
    x3 = aFrom.iX;
    y3 = aFrom.iY;
    x4 = aTo.iX;
    y4 = aTo.iY;

    // Calculate deltas
    bx = x2 - x1;
    by = y2 - y1;
    dx = x4 - x3;
    dy = y4 - y3;

    // Deteremine dot product of two segments
    dot = bx * dy - by * dx;
    if (dot == 0) {
        // Same/Parallel, end
        DebugEnd();
        return 0;
    }

    // Determine the dot product between the vector start
    // and the segment's first point.
    cx = x3 - x1;
    cy = y3 - y1;
    t = (cx * dy - cy * dx) / dot;
    //if (t < 0 || t > 1) {
    //    // Return if no intersection
    //    DebugEnd();
    //    return 0;
    //}

    // Now determine the dot product between the vector
    // and the segment's second point.
    //u = (cx * by - cy * bx) / dot;
    //if (u < 0 || u > 1) {
    //    // Return if no intersection
    //    DebugEnd();
    //    return 0;
    //}

    // Now that an intersection has been determined
    // to exist, determine where that point is
    aIntersection->iX = x1 + t * bx;
    aIntersection->iY = y1 + t * by;

    DebugEnd();

    return 1;
}

T_2DPolygon *PolygonClip(T_2DPolygon *aPolygon, T_2DVector *aVector)
{
    T_2DPolygonPoint *p_previous;
    T_2DType side1;
    T_2DType side2;
    T_2DPolygon *p_new;
    T_2DPolygonPoint *p;
    T_2DCoord prev;
    T_2DCoord at;
    T_2DCoord intersect;

    p_new = PolygonCreate();

    DebugRoutine("PolygonClip");
    if (aPolygon->iFirst) {
        p_previous = aPolygon->iLast;

        // Get previous point
        prev = p_previous->iCoord;
        // Calculate side of start position (last point of polygon)
        // using dot product against vector
        side1 = ((prev.iY - aVector->iStart.iY) * aVector->iDelta.iX)
                - ((prev.iX - aVector->iStart.iX) * aVector->iDelta.iY);

        // Walk the list of points in the current polygon
        p = aPolygon->iFirst;
        while (p) {
            // Fetch the vertex and determine the side to clipping vector
            at.iX = p->iCoord.iX;
            at.iY = p->iCoord.iY;

            // Calculate side of this new position to vector
            // using dot product
            side2 = ((at.iY - aVector->iStart.iY) * aVector->iDelta.iX)
                    - ((at.iX - aVector->iStart.iX) * aVector->iDelta.iY);

            // Are we on the front?
            if (side2 < -EPSILON) {
                // Does it cross over?
                if (side1 > EPSILON) {
                    // Split line with plane and insert the vertex
                     if (IVectorIntersection(aVector, prev, at, &intersect)) {
                        // If there is an intersection, add this point
                        PolygonAddPoint(p_new, intersect);
                     }
                } else {
                    // No, doesn't cross over
                }
                // In either case, the point is on the front side and must
                // be added.  Add the new point
                PolygonAddPoint(p_new, p->iCoord);
            } else if (side2 > EPSILON) {
                // Back side?  Do we cross over?
                if (side1 < -EPSILON) {
                    // Yes. Split line with plane and insert the vertex
                     if (IVectorIntersection(aVector, prev, at, &intersect)) {
                        // If there is an intersection, add this point
                        PolygonAddPoint(p_new, intersect);
                     }
                } else {
                    // No, not inside, skip this point
                }
            } else {
                // On the plane/vector
                PolygonAddPoint(p_new, at);
            }

            // Rotate around the polygon to the next segment/points
            p_previous = p;
            p = p->iNext;
            side1 = side2;
            prev = at;
        }
    }
    DebugEnd();

    return p_new;
}

T_2DPolygon *PolygonCopy(T_2DPolygon *aPolygon)
{
    T_2DPolygon *p_copy;
    T_2DPolygonPoint *p;

    DebugRoutine("PolygonCopy");
    p_copy = PolygonCreate();
    p = aPolygon->iFirst;
    while (p) {
        PolygonAddPoint(p_copy, p->iCoord);
        p = p->iNext;
    }
    DebugEnd();

    return p_copy;
}

unsigned int PolygonGetNumPoints(T_2DPolygon *aPolygon)
{
    T_2DPolygonPoint *p;
    unsigned int count = 0;

    DebugRoutine("PolygonGetNumPoints");
    p = aPolygon->iFirst;
    while (p) {
        count++;
        p = p->iNext;
    }
    DebugEnd();

    return count;
}
