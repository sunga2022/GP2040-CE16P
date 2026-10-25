#ifndef _TURBOSHOT_H_
#define _TURBOSHOT_H_

#include "storagemanager.h"

#ifndef TURBO_SHOT_MIN
#define TURBO_SHOT_MIN 2
#endif
#ifndef TURBO_SHOT_MAX
#define TURBO_SHOT_MAX 30
#endif

extern uint64_t getMicro();

// T30 = 30 flashes/sec, T10 = 10 flashes/sec. Independent of OLED/LED sample rate.
static inline bool turboShotPulseOn() {
	uint8_t shotCount = Storage::getInstance().getAddonOptions().turboOptions.shotCount;
	if (shotCount < TURBO_SHOT_MIN)
		shotCount = TURBO_SHOT_MIN;
	if (shotCount > TURBO_SHOT_MAX)
		shotCount = TURBO_SHOT_MAX;
	const uint32_t periodUs = 1000000u / static_cast<uint32_t>(shotCount);
	return (getMicro() % periodUs) < (periodUs / 2);
}

#endif
