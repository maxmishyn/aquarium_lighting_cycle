#ifndef __have__lampGlobals_h__
#define __have__lampGlobals_h__

#define ICON_SIZE 32
#define TOP_OFFSET 10

#define MAX_MODE 3
#define MAX_CYCLE 14399

#define MANUAL_TRANSITION_TIME 5000
#define MANUAL_TRANSITION_STEPS_PER_SECOND 100

#define WHITE_LIGHT_PIN 9

#include "skyWorks.h"

int mode = 0;
bool inSleepMode = false;
bool inManualTransition = false;
bool inSettings = false;

int currentCycle = 0;
int manualTargetCycle = 0;
int manualIncrement = 0;


const int modeCycles[4] = {
    0,
    100,
    7200,
    7300
};

#endif