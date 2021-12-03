#ifndef __have__lampGlobals_h__
#define __have__lampGlobals_h__

#define ICON_SIZE 32
#define TOP_OFFSET 10

#define MAX_MODE 3
#define MAX_CYCLE 14399

#define MANUAL_TRANSITION_TIME 5000
#define MANUAL_TRANSITION_STEPS_PER_SECOND 100

#define WHITE_LIGHT_PIN  9 // 10bit
#define BLUE_LIGHT_PIN  10 // 10bit 
#define RED_LIGHT_PIN    5 
#define GREEN_LIGHT_PIN  6

#include "skyWorks.h"

int mode = 0;
bool inManualTransition = false;

int currentCycle = 0;
int manualTargetCycle = 0;
int manualIncrement = 0;

const int modeCycles[4] = {
    0,
    100,
    7200,
    7300
};


EncButton<EB_TICK, 7> prevBtn;
EncButton<EB_TICK, 12> nextBtn;

GyverNTC therm(2, 10000, 3950, 25, 8440);

TimerMs thermTimer(2000, 1, 0);
TimerMs sleepTimer(60000, 0, 1);
TimerMs normalCycleTimer(6000, 1, 0); //6000
TimerMs inSettingsTimer(4000, 0, 1);


#endif