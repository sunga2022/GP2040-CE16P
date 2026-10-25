#ifndef _PS_TOUCHPAD_H_
#define _PS_TOUCHPAD_H_

#include "gpaddon.h"

#ifndef PS_TOUCHPAD_ENABLED
#define PS_TOUCHPAD_ENABLED 0
#endif

#define PSTouchpadName "PS Touchpad"

// DualShock 4 / DualSense style touchpad for Pico16N.
// PS4 / PS5 / PS5-general only:
//   TOUCH (A2) tap            = touchpad click
//   TOUCH (A2) + d-pad        = finger swipe
//   TOUCH (A2) + SELECT (S1)  = touchpad click (Select is not sent)
class PSTouchpadAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void process();
	virtual void preprocess() {}
	virtual void postprocess(bool) {}
	virtual void reinit() {}
	virtual std::string name() { return PSTouchpadName; }

private:
	void resetFinger();
	void suppressDpad(Gamepad * gamepad);

	bool held = false;
	bool usedSwipe = false;
	uint16_t posX = 0;
	uint16_t posY = 0;
	int32_t fracX = 0;
	int32_t fracY = 0;
	uint32_t lastUs = 0;
};

#endif
