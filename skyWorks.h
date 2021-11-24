#ifndef __have__skyLanpWorks_h__
#define __have__skyLanpWorks_h__

#include "FastLED.h"

template <int size>
struct skyPalette
{
    mData colors[size];
    mData get(int x, int amount)
    {
        long sectorSize = (amount + size - 2) / (size - 1); // (x+y-1)/y
        long sector = x / sectorSize;
        long xc = x-sector*sectorSize;
        long  amount8 = 255;
        if (xc<sectorSize) 
        {
          amount8 = (xc*255)/(sectorSize-1);///sectorSize*255;
        }
        mData dot = mergeRGBraw(
          blend8(colors[sector].r, colors[sector + 1].r, amount8),
          blend8(colors[sector].g, colors[sector + 1].g, amount8),
          blend8(colors[sector].b, colors[sector + 1].b, amount8)
        );
        

        return dot;
    }
};

#endif