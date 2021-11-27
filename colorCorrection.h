#ifndef __have__colorCorrection_h__
#define __have__colorCorrection_h__

static sData adjustColor(sData input, const CRGB &colorCorrection, const CRGB &colorTemperature)
{
  uint8_t adjustment[3] = {255, 255, 255};

  for (int i = 0; i <= 3; i++)
  {
    uint8_t cc = colorCorrection.raw[i];
    uint8_t ct = colorTemperature.raw[i];
    if (cc > 0 && ct > 0)
    {
      uint32_t work = (((uint32_t)cc) + 1) * (((uint32_t)ct) + 1) * 255;
      work /= 0x10000L;
      adjustment[i] = work & 0xFF;
    }
  }
#ifdef DEBUG_OUTPUT
  Serial.print(input.r);
  Serial.print(",");
  Serial.print(input.g);
  Serial.print(",");
  Serial.print(input.b);
  Serial.print(" >> ");
  Serial.print(adjustment[0]);
  Serial.print(",");
  Serial.print(adjustment[1]);
  Serial.print(",");
  Serial.print(adjustment[2]);
  Serial.print(" >> ");
  Serial.print(scale8(input.r, adjustment[0]));
  Serial.print(",");
  Serial.print(scale8(input.g, adjustment[1]));
  Serial.print(",");
  Serial.println(scale8(input.b, adjustment[2]));

#endif

  return {
      scale8_video(input.r, adjustment[0]),
      scale8_video(input.g, adjustment[1]),
      scale8_video(input.b, adjustment[2]),
      input.a};
}

struct colorTemperature
{
  CRGB value;
  char *name;
};

colorTemperature colorTemps[10] = {
    {UncorrectedTemperature, "нет"},
    {Candle, "1900"},
    {Tungsten40W, "2600"},
    {Tungsten100W, "2850"},
    {Halogen, "3200"},
    {CarbonArc, "5200"},
    {HighNoonSun, "5400"},
    {DirectSunlight, "6000"},
    {OvercastSky, "7000"},
    {ClearBlueSky, "20000"}
  };

uint8_t colotTemperatureSetting = 0;

#endif