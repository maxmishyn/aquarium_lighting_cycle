#define OLED_SPI_SPEED 4000000ul
//#define DEBUG_OUTPUT

#include <GyverOLED.h>
#include <EncButton.h>
#include <GyverNTC.h>
#include <TimerMs.h>
#include <math.h>
#include <microLED.h>

#include "globals.h"
#include "uiElements.h"
#include "skyWorks.h"
#include "colorCorrection.h"

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 2, 3, 4> oled;
//GyverOLED<SSD1306_128x64, OLED_NO_BUFFER, OLED_SPI, 2, 3, 4> oled;

EncButton<EB_TICK, 6> prevBtn;
EncButton<EB_TICK, 5> nextBtn;

GyverNTC therm(2, 10000, 3950, 25, 8440);

TimerMs thermTimer(2000, 1, 0);
TimerMs sleepTimer(60000, 0, 1);
TimerMs normalCycleTimer(6000, 1, 0);//6000
//TimerMs fastCycleTimer(MANUAL_TRANSITION_STEPS_PER_SECOND, 0, 0);

microLED<0, 8, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_AVER> strip;

void setup()
{
  initSkies();
//  strip.setBrightness(200);
  strip.clear();
  oled.init();
  //  Wire.setClock(400000L);

  oled.clear();
  delay(2);
  for (int i = 0; i < 4; i++)
  {
    oled.drawBitmap(ICON_SIZE * i, TOP_OFFSET, icon_table[i], ICON_SIZE, ICON_SIZE, BITMAP_INVERT);
  }
  overlayRect(1);
  oled.update();
  delay(2);
  writeTemp();
  sleepTimer.start();
#ifdef DEBUG_OUTPUT
  Serial.begin(9600);
#endif
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
      startManualTransition(1);
    }
    sleepTimer.start();
  }
  if (prevBtn.press())
  {
    if (!inSleepMode)
    {
      updateMode(mode - 1);
      startManualTransition(-1);
    }
    sleepTimer.start();
  }

  if (thermTimer.tick())
  {
    writeTemp();
  }

  if (normalCycleTimer.tick())
  {
    cycleTask();
  }

  if (inManualTransition)
  {
    fastCycleTask();
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
  if (newMode > MAX_MODE)
  {
    mode = 0;
  }
  else if (newMode < 0)
  {
    mode = MAX_MODE;
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
  oled.clear(0, TOP_OFFSET * 2 + ICON_SIZE, 8 * 12, TOP_OFFSET * 2 + ICON_SIZE + 8);
  oled.setCursorXY(0, TOP_OFFSET * 2 + ICON_SIZE);
  oled.print("Темп: ");
  oled.print(therm.getTempAverage(), 1);
  drawIcon(0);
  oled.update(0, TOP_OFFSET * 2 + ICON_SIZE, 8 * 12, TOP_OFFSET * 2 + ICON_SIZE + 8);
}

void drawIcon(byte index)
{
  size_t s = sizeof icons[index];
  for (unsigned int i = 0; i < s; i++)
  {
    oled.drawByte(pgm_read_byte(&(icons[index][i])));
  }
}

void cycleTask()
{
  currentCycle++;
  if (mode == MAX_MODE)
  {
    if (currentCycle>MAX_CYCLE) 
    {
      currentCycle = 0;
      updateMode(0);
    } 
  }
  else 
  {
    if (currentCycle>=modeCycles[(mode+1)]) {
      updateMode(mode + 1);
    }
  }
  lightRendering();
}

void startManualTransition(int direction)
{
  if (!inManualTransition)
  {
    normalCycleTimer.stop();
    inManualTransition = true;
  }
  int delta = modeCycles[mode] - currentCycle;
  if (direction > 0 && delta < 0)
    delta = delta + MAX_CYCLE;
  if (direction < 0 && delta > 0)
    delta = delta - MAX_CYCLE;
  int transitionSteps = 200; //(int)MANUAL_TRANSITION_TIME / MANUAL_TRANSITION_STEPS_PER_SECOND;
  manualIncrement = (int) delta / transitionSteps;
  if (manualIncrement==0) 
  {
    manualIncrement = direction;
  }
}

void fastCycleTask()
{
  currentCycle = currentCycle + manualIncrement;

  if (currentCycle > MAX_CYCLE)
    currentCycle = 0;

  if (currentCycle < 0)
    currentCycle = MAX_CYCLE;

  lightRendering();
  delay(25);
  
  if (abs(modeCycles[mode] - currentCycle) < abs(manualIncrement*2)) {
    inManualTransition = false;
    normalCycleTimer.start();
  }
}

void lightRendering()
{
  mData color = getColor(currentCycle);
  strip.begin();
  strip.send(color);
  strip.end();
}

mData getColor(int _step)
{
  int cyclesPerMode;
  int cMode = getModeByCycle(_step);
  if (cMode == MAX_MODE) 
  {
    cyclesPerMode = MAX_CYCLE - modeCycles[MAX_MODE];
  }
  else 
  {
    cyclesPerMode = modeCycles[cMode + 1] - modeCycles[cMode];
  }

  return adjustColor(
      skyCycle[cMode]
          .get(_step - modeCycles[cMode], cyclesPerMode),
      TypicalSMD5050,
      UncorrectedTemperature
      );
}

void fadeIn()
{
}

int getModeByCycle(int cycle)
{
  for(int i=MAX_MODE; i>=0; i--)
  {
    if (cycle>modeCycles[i]) 
      return i;
  }
}

void initSkies()
{
  skyCycle[0].colors[0] = mRGB(110, 113, 216);
  skyCycle[0].colors[1] = mRGB(170, 104, 173);
  skyCycle[0].colors[2] = mRGB(255, 180, 58);
  skyCycle[0].colors[3] = mRGB(255, 218, 95);
  skyCycle[0].colors[4] = mRGB(255, 255, 130);
  skyCycle[0].colors[5] = mRGB(240, 255, 255);

  skyCycle[1].colors[0] = mRGB(240, 255, 255);
  skyCycle[1].colors[1] = mRGB(255, 255, 255);
  skyCycle[1].colors[2] = mRGB(255, 255, 255);
  skyCycle[1].colors[3] = mRGB(255, 255, 255);
  skyCycle[1].colors[4] = mRGB(255, 255, 255);
  skyCycle[1].colors[5] = mRGB(240, 255, 255);

  skyCycle[2].colors[0] = mRGB(240, 255, 255);
  skyCycle[2].colors[1] = mRGB(254, 242, 13);
  skyCycle[2].colors[2] = mRGB(251, 163, 48);
  skyCycle[2].colors[3] = mRGB(235, 80, 38);
  skyCycle[2].colors[4] = mRGB(70, 120, 156);
  skyCycle[2].colors[5] = mRGB(48, 76, 117);

  skyCycle[3].colors[0] = mRGB(48, 76, 117);
  skyCycle[3].colors[1] = mRGB(0, 32, 127);
  skyCycle[3].colors[2] = mRGB(0, 0, 64);
  skyCycle[3].colors[3] = mRGB(0, 0, 64);
  skyCycle[3].colors[4] = mRGB(0, 0, 127);
  skyCycle[3].colors[5] = mRGB(110, 113, 216);
}