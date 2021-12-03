#ifndef __have__stateMachine_h__
#define __have__stateMachine_h__

#include "uiElements.h"

#define STATE_MAIN_GUI 0
#define STATE_SLEEP_MODE 2
#define STATE_SETUP_COLOR_TEMPERATURE 3
#define STATE_SETUP_COLOR_BRIGHTNESS 4

uint8_t currentState = STATE_MAIN_GUI;
uint8_t newState = 255;

bool toSleepMode();
bool toSettingBrightness();
bool toSettingColorTemperature();
bool fromSettingsToMainGUI();
bool fromSleepToMainGUI();
void runGUI();
void runSleepMode();
void runSetupColorTemperature();
void runSetupBrightness();
void startManualTransition(int direction);
uint8_t updateByteSetting(uint8_t val, int amount, uint8_t maxVal);

void switchToState(uint8_t state)
{
  if (currentCycle != state) 
  {
    currentState = state;
    newState = state;
  }
}

void stateDetect()
{
#ifdef DEBUG_OUTPUT
  Serial.println(inSettingsTimer.elapsed());
#endif

  switch (currentState)
  {
  case STATE_MAIN_GUI:
    if (toSleepMode())
      return switchToState(STATE_SLEEP_MODE);
    if (toSettingBrightness())
      return switchToState(STATE_SETUP_COLOR_BRIGHTNESS);
    if (toSettingColorTemperature())
      return switchToState(STATE_SETUP_COLOR_TEMPERATURE);
    break;
  case STATE_SLEEP_MODE:
    if (fromSleepToMainGUI())
      return switchToState(STATE_MAIN_GUI);
    break;
  case STATE_SETUP_COLOR_TEMPERATURE:
    if (fromSettingsToMainGUI())
      return switchToState(STATE_MAIN_GUI);
    break;
  case STATE_SETUP_COLOR_BRIGHTNESS:
    if (fromSettingsToMainGUI())
      return switchToState(STATE_MAIN_GUI);
    break;
  }
}

void stateMachineRun()
{
  stateDetect();

  switch (currentState)
  {
  case STATE_MAIN_GUI:
    runGUI();
    break;
  case STATE_SLEEP_MODE:
    runSleepMode();
    break;
  case STATE_SETUP_COLOR_TEMPERATURE:
    runSetupColorTemperature();
    break;
  case STATE_SETUP_COLOR_BRIGHTNESS:
    runSetupBrightness();
    break;
  }
}

void runGUI()
{
  if (newState == currentState)
  {
    oled.setPower(true);
    inSettingsTimer.stop();
    sleepTimer.start();
    writeTemp();
    newState = -1;
  }

  if (nextBtn.click())
  {
    updateMode(mode + 1);
    startManualTransition(1);
    sleepTimer.start();
  }

  if (prevBtn.click())
  {
    updateMode(mode - 1);
    startManualTransition(-1);
    sleepTimer.start();
  }
  if (thermTimer.tick())
  {
    writeTemp();
  }
}

void runSleepMode()
{
  if (newState == currentState)
  {
    oled.setPower(false);
    sleepTimer.stop();
    newState = -1;
  }
  if (nextBtn.click() || prevBtn.click())
  {
    sleepTimer.start();
  }
}

void runSetupColorTemperature()
{
  if (newState == currentState)
  {
    inSettingsTimer.start();
    newState = -1;
  }

  if (nextBtn.click() || nextBtn.step())
  {
    colorTemperatureSetting = updateByteSetting(colorTemperatureSetting, 1, 9);
  }

  if (prevBtn.click() || prevBtn.step())
  {
    colorTemperatureSetting = updateByteSetting(colorTemperatureSetting, -1, 9);
  }

  write_eeprom();
  drawColorTemperatureSettings();
}

void runSetupBrightness()
{
  if (newState == currentState)
  {
    inSettingsTimer.start();
    newState = -1;
  }
  if (nextBtn.click())
  {
    colorBrightnessSetting = updateByteSetting(colorBrightnessSetting, 26, 255);
  }

  if (prevBtn.click())
  {
    colorBrightnessSetting = updateByteSetting(colorBrightnessSetting, -26, 255);
  }

  if (nextBtn.step())
  {
    colorBrightnessSetting = updateByteSetting(colorBrightnessSetting, 1, 255);
  }

  if (prevBtn.step())
  {
    colorBrightnessSetting = updateByteSetting(colorBrightnessSetting, -1, 255);
  }

  write_eeprom();
  drawBrightnessSettings();
}

bool toSleepMode()
{
  if (sleepTimer.elapsed())
    return true;
  return false;
}

bool toSettingBrightness()
{
  if (prevBtn.hold())
    return true;
  return false;
}

bool toSettingColorTemperature()
{
  if (nextBtn.hold())
    return true;
  return false;
}

bool fromSettingsToMainGUI()
{
  if (inSettingsTimer.elapsed())
    return true;
  return false;

}
bool fromSleepToMainGUI()
{
  if (!sleepTimer.elapsed())
    return true;
  return false;
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
  manualIncrement = (int)delta / transitionSteps;
  if (manualIncrement == 0)
  {
    manualIncrement = direction;
  }
}

uint8_t updateByteSetting(uint8_t val, int amount, uint8_t maxVal)
{
  uint8_t res;
  if (amount < 0 && amount > val)
  {
    res = 0;
  } else if (amount>0 && amount>(maxVal-val))
  {
    res = maxVal;
  } else {
    res = (uint8_t) val + amount;
  }
  inSettingsTimer.start();
}

#endif