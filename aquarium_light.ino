#define OLED_SPI_SPEED 4000000ul

#include <GyverOLED.h>
#include <EncButton.h>
#include <GyverNTC.h>
#include <TimerMs.h>

#include "grobals.h"
#include "uiElements.h"

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 2, 3, 4> oled;
//GyverOLED<SSD1306_128x64, OLED_NO_BUFFER, OLED_SPI, 2, 3, 4> oled;

EncButton<EB_TICK, 6> prevBtn;
EncButton<EB_TICK, 5> nextBtn;

GyverNTC therm(2, 10000, 3950, 25, 8440);

TimerMs thermTimer(2000, 1, 0);
TimerMs sleepTimer(60000, 0, 1);

void setup()
{
  oled.init();
  //  Wire.setClock(400000L);

  oled.clear(); 
 
  for (int i = 0; i < 4; i++)
  {
    oled.drawBitmap(ICON_SIZE * i, TOP_OFFSET, icon_table[i], ICON_SIZE, ICON_SIZE, BITMAP_INVERT);
  }
  overlayRect(1);
  writeTemp();
  oled.update();
  sleepTimer.start();
  //  Serial.begin(9600);
}

void loop()
{
  nextBtn.tick();
  prevBtn.tick();
  if (nextBtn.press())
  {
    if (!inSleepMode)
    {
      updateMode(mode + 1);
    }
    sleepTimer.start();
  }
  if (prevBtn.press())
  {
    if (!inSleepMode)
    {
      updateMode(mode - 1);
    }
    sleepTimer.start();
  }
  if (thermTimer.tick())
  {
    writeTemp();
  }
  sleepMode();
}

void sleepMode()
{
  if (inSleepMode)
  {
    if (!sleepTimer.elapsed())
    {
      oled.setPower(true);
      inSleepMode = false;
    }
  }
  else
  {
    if (sleepTimer.elapsed())
    {
      oled.setPower(false);
      inSleepMode = true;
    }
  }
}

void updateMode(int newMode)
{
  oled.drawBitmap(ICON_SIZE * mode, TOP_OFFSET, icon_table[mode], ICON_SIZE, ICON_SIZE, BITMAP_INVERT);
  overlayRect(0);
  oled.update(ICON_SIZE * mode, TOP_OFFSET, ICON_SIZE * (mode + 1), (ICON_SIZE + TOP_OFFSET));
  if (newMode > 3)
  {
    mode = 0;
  }
  else if (newMode < 0)
  {
    mode = 3;
  }
  else
  {
    mode = newMode;
  }
  overlayRect(1);
  oled.drawBitmap(ICON_SIZE * mode, TOP_OFFSET, icon_table[mode], ICON_SIZE, ICON_SIZE, BITMAP_INVERT, BUF_ADD);
  oled.update(ICON_SIZE * mode, TOP_OFFSET, ICON_SIZE * (mode + 1), (ICON_SIZE + TOP_OFFSET));
}

void overlayRect(int fill)
{
  oled.fastLineH(TOP_OFFSET, ICON_SIZE * mode, ICON_SIZE * (mode + 1) - 1, fill);
  oled.fastLineH((ICON_SIZE + TOP_OFFSET) - 1, ICON_SIZE * mode, ICON_SIZE * (mode + 1) - 1, fill);
  oled.fastLineV(ICON_SIZE * mode, TOP_OFFSET, (ICON_SIZE + TOP_OFFSET) - 1, fill);
  oled.fastLineV(ICON_SIZE * (mode + 1) - 1, TOP_OFFSET, (ICON_SIZE + TOP_OFFSET) - 1, fill);
}

void writeTemp()
{
  oled.clear(8 * 6, TOP_OFFSET * 2 + ICON_SIZE, 8 * 12, TOP_OFFSET * 2 + ICON_SIZE + 8);
  oled.setCursorXY(0, TOP_OFFSET * 2 + ICON_SIZE);
  oled.print("Темп: ");
  oled.print(therm.getTempAverage(), 1);
  drawIcon(0);
  oled.update(8 * 6, TOP_OFFSET * 2 + ICON_SIZE, 8 * 12, TOP_OFFSET * 2 + ICON_SIZE + 8);
}

void drawIcon(byte index)
{
  size_t s = sizeof icons[index];
  for (unsigned int i = 0; i < s; i++)
  {
    oled.drawByte(pgm_read_byte(&(icons[index][i])));
  }
}
