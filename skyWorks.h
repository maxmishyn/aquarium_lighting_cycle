#ifndef __have__skyLanpWorks_h__
#define __have__skyLanpWorks_h__
#define SKY_INLINE __attribute__((always_inline))
#include "FastLED.h"

struct sData
{
  uint8_t r, g, b, a;
  inline sData() SKY_INLINE {}
  inline sData(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) SKY_INLINE 
      : r(_r), g(_g), b(_b), a(_a) {}
  inline sData(uint32_t colorcode, uint8_t _a) SKY_INLINE
      : r(((uint32_t)colorcode >> 16) & 0xFF),
        g(((uint32_t)colorcode >> 8) & 0xFF),
        b(colorcode & 0xFF),
        a(_a) {}
  inline sData(uint32_t colorcode)  SKY_INLINE
    : r(((uint32_t)colorcode >> 24) & 0xFF), g(((uint32_t)colorcode >> 16) & 0xFF), b(colorcode >> 8), a(colorcode & 0xFF){}	
  inline sData& operator= (const uint32_t colorcode) SKY_INLINE {
        r = ((uint32_t)colorcode >> 24) & 0xFF;
        g = ((uint32_t)colorcode >> 16) & 0xFF;
        b = ((uint32_t)colorcode >>  8) & 0xFF;
        a = colorcode & 0xFF;
        return *this;
    }		
};
inline SKY_INLINE bool operator== (const sData& lhs, const sData& rhs) {
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) && (lhs.a == rhs.a);
}
inline SKY_INLINE bool operator!= (const sData& lhs, const sData& rhs) {
    return (lhs.r != rhs.r) || (lhs.g != rhs.g) || (lhs.b != rhs.b) || (lhs.a != rhs.a);
}

sData makeRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) 
{
  return {r,g,b,a};
}
sData makeHEX(uint32_t color, uint8_t a)
{
  return {color, a};
}

sData makeHSV(uint8_t h, uint8_t s, uint8_t v, uint8_t a)
{
  float r, g, b;

  float H = h / 255.0;
  float S = s / 255.0;
  float V = v / 255.0;

  int i = int(H * 6);
  float f = H * 6 - i;
  float p = V * (1 - S);
  float q = V * (1 - f * S);
  float t = V * (1 - (1 - f) * S);

  switch (i % 6)
  {
  case 0:
    r = V, g = t, b = p;
    break;
  case 1:
    r = q, g = V, b = p;
    break;
  case 2:
    r = p, g = V, b = t;
    break;
  case 3:
    r = p, g = q, b = V;
    break;
  case 4:
    r = t, g = p, b = V;
    break;
  case 5:
    r = V, g = p, b = q;
    break;
  }
  r *= 255.0;
  g *= 255.0;
  b *= 255.0;
  return makeRGB((uint8_t)r, (uint8_t)g, (uint8_t)b, a);
}

sData makeTemperature(int kelvin, uint8_t a)
{
  float tmpKelvin, tmpCalc;
  uint8_t _r, _g, _b;

  kelvin = constrain(kelvin, 1000, 40000);
  tmpKelvin = kelvin / 100;

  if (tmpKelvin <= 66)
    _r = 255;
  else
  {
    tmpCalc = tmpKelvin - 60;
    tmpCalc = (float)pow(tmpCalc, -0.1332047592);
    tmpCalc *= (float)329.698727446;
    tmpCalc = constrain(tmpCalc, 0, 255);
    _r = tmpCalc;
  }

  // green
  if (tmpKelvin <= 66)
  {
    tmpCalc = tmpKelvin;
    tmpCalc = (float)99.4708025861 * log(tmpCalc) - 161.1195681661;
    tmpCalc = constrain(tmpCalc, 0, 255);
    _g = tmpCalc;
  }
  else
  {
    tmpCalc = tmpKelvin - 60;
    tmpCalc = (float)pow(tmpCalc, -0.0755148492);
    tmpCalc *= (float)288.1221695283;
    tmpCalc = constrain(tmpCalc, 0, 255);
    _g = tmpCalc;
  }

  // blue
  if (tmpKelvin >= 66)
    _b = 255;
  else if (tmpKelvin <= 19)
    _b = 0;
  else
  {
    tmpCalc = tmpKelvin - 10;
    tmpCalc = (float)138.5177312231 * log(tmpCalc) - 305.0447927307;
    tmpCalc = constrain(tmpCalc, 0, 255);
    _b = tmpCalc;
  }
  return makeRGB(_r, _g, _b, a);
}

template <int size>
struct skyPalette {
    sData colors[size];
    sData get(int x, int amount) { 
      uint32_t sectorSize = (amount + size - 2) / (size - 1); // (x+y-1)/y
      uint32_t sector = x / sectorSize;
      uint32_t xc = x - sector * sectorSize;
      uint32_t amount8 = 255;
      if (xc < sectorSize)
      {
        amount8 = (xc * 255) / (sectorSize - 1); ///sectorSize*255;
      }
      sData dot = {
          blend8(colors[sector].r, colors[sector + 1].r, amount8),
          blend8(colors[sector].g, colors[sector + 1].g, amount8),
          blend8(colors[sector].b, colors[sector + 1].b, amount8),
          blend8(colors[sector].a, colors[sector + 1].a, amount8)
        };
      return dot;
    }
};

skyPalette<6> skyCycle[4] = {
    {makeRGB(110, 113, 216, 0),
     makeRGB(170, 104, 173, 0),
     makeRGB(255, 180, 58, 0),
     makeRGB(255, 218, 95, 0),
     makeRGB(255, 255, 130, 0),
     makeHEX(0xf8f7d5, 32)},
    {
        makeHEX(0xf8f7d5, 32),
        makeHEX(0xfff1b2, 255),
        makeHEX(0xfff1b2, 255),
        makeHEX(0xfff1b2, 255),
        makeHEX(0xfff1b2, 255),
        makeHEX(0xf0ffff, 32),
    },
    {
        makeRGB(240, 255, 255, 32),
        makeRGB(254, 242, 13, 0),
        makeRGB(251, 163, 48, 0),
        makeRGB(235, 80, 38, 0),
        makeRGB(70, 120, 156, 0),
        makeHEX(0x1d304d, 0),
    },
    {
        makeHEX(0x1d304d, 0),
        makeRGB(0, 32, 127, 0),
        makeRGB(0, 0, 64, 0),
        makeRGB(0, 0, 64, 0),
        makeRGB(0, 0, 127, 0),
        makeRGB(110, 113, 216, 0),
    }};

#endif