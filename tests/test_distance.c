#include <assert.h>
#include <stdio.h>
#include "../Include/GENERAL.H"

static T_word16 IQuickSquareRoot(T_word32 value)
{
    T_sword16 i;
    T_word16 result, tmp;
    T_word32 low, high;

    if (value <= 1L)
        return (T_word16)value;

    low = value;
    high = 0L;
    result = 0;

    for (i = 0; i < 16; i++) {
        result += result;
        high = (high << 2) | ((low >> 30) & 0x3);
        low <<= 2;

        tmp = result + result + 1;
        if (high >= tmp) {
            result++;
            high -= tmp;
        }
    }

    return result;
}

T_word16 CalculateDistance(
             T_sword32 x1,
             T_sword32 y1,
             T_sword32 x2,
             T_sword32 y2)
{
    T_sword32 dx;
    T_sword32 dy;
    T_word16 shift = 0;

    dx = x1 - x2;
    if (dx < 0)
        dx = -dx;

    dy = y2 - y1;
    if (dy < 0)
        dy = -dy;

    while ((dx & 0xFFFF0000) || (dy & 0xFFFF0000)) {
        dx >>= 2;
        dy >>= 2;
        shift += 2;
    }

    if (dx == 0)
        return (dy << shift);
    if (dy == 0)
        return (dx << shift);

    return (IQuickSquareRoot((dx * dx) + (dy * dy)) << shift);
}

T_word16 CalculateEstimateDistance(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2)
{
    x1 -= x2;
    if (x1 < 0)
        x1 = -x1;

    y1 -= y2;
    if (y1 < 0)
        y1 = -y1;

    if (x1 > y1)
        return x1;

    return y1;
}

int main(void)
{
    /* CalculateDistance symmetry */
    assert(CalculateDistance(0, 0, 3, 4) == 5);
    assert(CalculateDistance(3, 4, 0, 0) == 5);

    /* CalculateEstimateDistance symmetry */
    assert(CalculateEstimateDistance(0, 0, 3, 4) == 4);
    assert(CalculateEstimateDistance(3, 4, 0, 0) == 4);

    printf("All distance tests passed.\n");
    return 0;
}
