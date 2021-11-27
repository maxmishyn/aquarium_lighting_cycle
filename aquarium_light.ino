#define OLED_SPI_SPEED 4000000ul
#define EB_HOLD 2000

//#define DEBUG_OUTPUT

#include <GyverOLED.h>
#include <EncButton.h>
#include <GyverNTC.h>
#include <TimerMs.h>
#include <math.h>
#include <microLED.h>
#include <EEPROMex.h>

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
TimerMs inSettingsTimer(4000, 0, 0);

//TimerMs fastCycleTimer(MANUAL_TRANSITION_STEPS_PER_SECOND, 0, 0);

microLED<0, 8, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_OFF> strip;

void setup()
{
//  strip.setBrightness(200);
  strip.clear();
  oled.init();
  //  Wire.setClock(400000L);

  read_eeprom();

  sleepTimer.start();

  pinMode(WHITE_LIGHT_PIN, OUTPUT);

  oled.clear();
  for (int i = 0; i < 4; i++)
  {
    oled.drawBitmap(ICON_SIZE * i, TOP_OFFSET, icon_table[i], ICON_SIZE, ICON_SIZE, BITMAP_INVERT);
  }
  overlayRect(1);
  oled.update();
  writeTemp();

#ifdef DEBUG_OUTPUT
  Serial.begin(9600);
#endif
}

void loop()
{
  nextBtn.tick();
  prevBtn.tick();

  if (nextBtn.hold()||prevBtn.hold())
  {
    inSettings = true;
    inSettingsTimer.start();
  }

  if (inSettings) 
  {
    if (nextBtn.click())
    {
      if (colotTemperatureSetting < 9) {}
        colotTemperatureSetting++;
      sleepTimer.start();
      inSettingsTimer.start();
      write_eeprom();
    }

    if (prevBtn.click())
    {
      if (colotTemperatureSetting > 0)
        colotTemperatureSetting--;
      sleepTimer.start();
      inSettingsTimer.start();
      write_eeprom();
    }

    drawSettings();

    if (inSettingsTimer.elapsed())
    {
      oled.clear(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
      oled.update(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
      inSettings = false;
    }
  }
  else 
  {
    if (nextBtn.click())
    {
      if (!inSleepMode)
      {
        updateMode(mode + 1);
        startManualTransition(1);
      }
      sleepTimer.start();
    }

    if (prevBtn.click())
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
  oled.clear(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
  oled.setCursorXY(0, TOP_OFFSET * 2 + ICON_SIZE);
  oled.print("Темп: ");
  oled.print(therm.getTempAverage(), 1);
  drawIcon(0);
  oled.update(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
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
  sData color = getColor(currentCycle);
  mData outColor = { color.r, color.g, color.b };
  analogWrite(WHITE_LIGHT_PIN, color.a);
  strip.begin();
  for (int i=0;i<6;i++)
  {
    strip.send(outColor);
  }
  strip.end();
}

sData getColor(int _step)
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
      colorTemps[colotTemperatureSetting].value);
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

void write_eeprom()
{
  EEPROM.updateByte(10, colotTemperatureSetting);
  EEPROM.updateByte(11, 253);
}

void read_eeprom()
{
  if (EEPROM.read(11) != 253)
  {
    colotTemperatureSetting = 0;
  }
  else 
  {
    colotTemperatureSetting = EEPROM.read(10);
    if (colotTemperatureSetting < 0)
      colotTemperatureSetting = 0;
    if (colotTemperatureSetting > 9)
      colotTemperatureSetting = 9;
  }
}

void drawSettings()
{
  oled.clear(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
  oled.setCursorXY(0, TOP_OFFSET * 2 + ICON_SIZE);
  oled.print("Цвет. темп: ");
  oled.print(colorTemps[colotTemperatureSetting].name);
  if (colotTemperatureSetting>0) 
    drawIcon(1);
  oled.update(0, TOP_OFFSET * 2 + ICON_SIZE, 127, TOP_OFFSET * 2 + ICON_SIZE + 8);
}
