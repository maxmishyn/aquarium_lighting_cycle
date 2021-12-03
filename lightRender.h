#ifndef __have__rendered_h__
#define __have__rendered_h__

sData getColor(int _step);

//microLED<0, 8, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_OFF> strip;

void initLeds()
{
	//  strip.setBrightness(200);
	// strip.clear();
	pinMode(WHITE_LIGHT_PIN, OUTPUT);
	pinMode(RED_LIGHT_PIN, OUTPUT);
	pinMode(GREEN_LIGHT_PIN, OUTPUT);
	pinMode(BLUE_LIGHT_PIN, OUTPUT);
}

int getModeByCycle(int cycle)
{
	for (int i = MAX_MODE; i >= 0; i--)
	{
		if (cycle > modeCycles[i])
			return i;
	}
}

void lightRendering()
{
	sData color = getColor(currentCycle);
	analogWrite(WHITE_LIGHT_PIN, (int)color.a<<2);
	analogWrite(RED_LIGHT_PIN,   color.r);
	analogWrite(GREEN_LIGHT_PIN, color.g);
	analogWrite(BLUE_LIGHT_PIN,  (int)color.b<<2);
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
		colorTemps[colorTemperatureSetting].value);
}

#endif