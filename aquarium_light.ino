#define TIMER_MULTIPLIER 8
#define micros() (micros() >> TIMER_MULTIPLIER)
#define millis() (millis() >> TIMER_MULTIPLIER)

#define OLED_SPI_SPEED 4000000ul
#define EB_HOLD 2000
#define EB_STEP 250

#define DEBUG_OUTPUT

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
#include "stateMachine.h"
#include "lightRender.h"

void setup()
{
//  TCCR2B = 0b00000010; // x8  D3, D11
//  TCCR2A = 0b00000011; // fast pwm
TCCR1A = 0b00000011; // 10bit  
TCCR1B = 0b00001001; // x1 fast pwm
TCCR0B = 0b00000010; // x8
TCCR0A = 0b00000011; // fast pwm  D5, D6
read_eeprom();
initScreen();
initLeds();
  
#ifdef DEBUG_OUTPUT
  Serial.begin(9600);
#endif
}

void loop()
{
  nextBtn.tick();
  prevBtn.tick();


  stateMachineRun();

   if (normalCycleTimer.tick())
   {
     cycleTask();
   }

   if (inManualTransition)
   {
     fastCycleTask();
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




