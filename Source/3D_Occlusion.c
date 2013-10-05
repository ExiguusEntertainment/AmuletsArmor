#include <general.h>
#include <memory.h>
#include <Windows.h>
#include <GL/gl.h>
#include <3D_Occlusion.h>

#define DEBUG_OCCLUSION_PRINTF      0
#define OCCLUDE_ROUNDING        ((GLdouble)0.1)

typedef struct _T_occlusion {
        struct _T_occlusion *iNext;
        struct _T_occlusion *iPrevious;
        T_occludeValue iLeft;
        T_occludeValue iRight;
} T_occlusion;
static T_occlusion *G_occlusions = 0;

void OcclusionInit(void)
{
    T_occlusion *p = G_occlusions;
    T_occlusion *p_next;

#if DEBUG_OCCLUSION_PRINTF
    printf("---------------\n");
#endif
    while (p) {
        p_next = p->iNext;
        MemFree(p);
        p = p_next;
    }
    G_occlusions = 0;
}

#if DEBUG_OCCLUSION_PRINTF
void OcclusionDump(void)
{
    T_occlusion *p = G_occlusions;

    printf("OD: ");
    while (p) {
        printf("(%g, %g) ", p->iLeft, p->iRight);
        p = p->iNext;
    }
    printf("\n");
}
#endif


void OcclusionAdd(T_occludeValue aLeft, T_occludeValue aRight)
{
    // First, see if the left is inside one of the occlusions
    // or before another occlusion
    T_occlusion *p = G_occlusions;
    T_occlusion *p_new = 0;
    T_occlusion *p_next;
    T_occlusion *p_combine;
    T_occlusion *p_last = 0;
    int isInside = 0;

    aLeft -= OCCLUDE_ROUNDING;
    aRight += OCCLUDE_ROUNDING;
#if DEBUG_OCCLUSION_PRINTF
    printf("OcclusionAdd(%g, %g)\n", aLeft, aRight);
#endif

    while (p) {
        p_next = p->iNext;
        if ((aLeft >= p->iLeft) && (aLeft <= p->iRight)) {
            // left edge IS inside an existing occlusion
            // Extend the occlusion (if bigger)
            if (p->iRight < aRight)
                p->iRight = aRight;
            break;
        } else if (aLeft < p->iLeft) {
            // We are too far to the left of this occlusion.
            // A new occlusion is needed, link it in here
//printf("+A+");
            p_new = (T_occlusion *)MemAlloc(sizeof(T_occlusion));
            p_new->iPrevious = p->iPrevious;
            if (p->iPrevious) {
                p->iPrevious->iNext = p_new;
            } else {
                G_occlusions = p_new;
            }
            p_new->iNext = p;
            p->iPrevious = p_new;
            p_new->iLeft = aLeft;
            p_new->iRight = aRight;
            p_last = p;
            p = p_new;
            break;
        }
        p_last = p;
        p = p_next;
    }
    // If we got here and didn't find one, we need to add
    // a new one to end of the list
    if (!p) {
//printf("+B+");
        p_new = (T_occlusion *)MemAlloc(sizeof(T_occlusion));
        p_new->iLeft = aLeft;
        p_new->iRight = aRight;
        if (p_last) {
            p_last->iNext = p_new;
            p_new->iPrevious = p_last;
        } else {
            G_occlusions = p_new;
            p_new->iPrevious = 0;
        }
        p_new->iNext = 0;
        p = p_new;
    }

    // Now that we a range in p at aLeft, let's
    // combine it with the ranges to the right (if any).
    // This will drop/combine ranges that overlap with this occlusion range.
    p_combine = p;
    p = p->iNext;
    while (p) {
        p_next = p->iNext;
        if (p->iLeft <= p_combine->iRight) {
            // Left edge is inside right edge.
            // Is right edge also inside?
            if (p->iRight <= p_combine->iRight) {
                // Fully inside!
            } else {
                // Partially inside!
                // Extend the right edge
                p_combine->iRight = p->iRight;
            }
            // Since the old occlusion is now in the combining range,
            // (it was either fully inside or touching)
            // delete the old occlusion range, it is no longer needed
            // now that the combining occlusion range is over both.
            p_combine->iNext = p->iNext;
            if (p->iNext)
                p->iNext->iPrevious = p_combine;
            MemFree(p);
//printf("-C-");
        } else {
//printf("=D=");
            // Left edge is not inside the right combine edge.
            // Let's stop here.  Nothing more to combine.
            break;
        }
        p = p_next;
    }
#if DEBUG_OCCLUSION_PRINTF
    OcclusionDump();
#endif
}

int OcclusionIsVisible(T_occludeValue aLeft, T_occludeValue aRight)
{
    T_occlusion *p = G_occlusions;

#if DEBUG_OCCLUSION_PRINTF
    printf("OcclusionIsVisible(%g, %g): ", aLeft, aRight);
#endif
    while (p) {
        // Is the range fully hidden by an occlusion range?
        if ((aLeft >= p->iLeft) && (aRight <= p->iRight)) {
            // Yes, fully hidden.  Not visible.
#if DEBUG_OCCLUSION_PRINTF
    printf("Fully hidden\n");
#endif
            return 0;
        }
        p = p->iNext;
    }

#if DEBUG_OCCLUSION_PRINTF
    printf("Visible\n");
#endif
    // Never fully hidden, so, yes, visible (at least partially)
    return 1;
}
