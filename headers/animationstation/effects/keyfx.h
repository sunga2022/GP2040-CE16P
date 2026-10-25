#ifndef KEYFX_H_
#define KEYFX_H_

#include "animation.h"
#include <math.h>
#include <stdint.h>

// Packed into the high nibble of each customTheme* idle color.
// 0 solid, 1 breath, 2 jump, 3 fast flash, 4 slow flash, 5 heartbeat.
#define KEY_FX_SOLID 0
#define KEY_FX_BREATH 1
#define KEY_FX_JUMP 2
#define KEY_FX_FAST 3
#define KEY_FX_SLOW 4
#define KEY_FX_HEART 5
#define KEY_FX_COUNT 6

static inline uint8_t keyFxId(uint32_t packed) {
	const uint8_t id = (uint8_t)((packed >> 24) & 0x0F);
	return id < KEY_FX_COUNT ? id : KEY_FX_SOLID;
}

static inline float keyFxLevel(uint8_t fx, float seconds) {
	const float t = seconds < 0.0f ? 0.0f : seconds;
	switch (fx) {
	case KEY_FX_BREATH: {
		const float s = (sinf(t * 2.2f) + 1.0f) * 0.5f;
		const float e = s * s * (3.0f - 2.0f * s);
		return 0.08f + 0.92f * e;
	}
	case KEY_FX_JUMP: {
		const float ph = fmodf(t * 2.4f, 1.0f);
		if (ph < 0.12f)
			return 1.0f;
		if (ph < 0.42f)
			return 0.12f + 0.88f * (0.42f - ph) / 0.30f;
		return 0.06f;
	}
	case KEY_FX_FAST:
		return fmodf(t * 8.0f, 1.0f) < 0.42f ? 1.0f : 0.04f;
	case KEY_FX_SLOW:
		return fmodf(t * 1.15f, 1.0f) < 0.48f ? 1.0f : 0.05f;
	case KEY_FX_HEART: {
		const float ph = fmodf(t * 1.15f, 1.0f);
		const float a = expf(-((ph - 0.10f) * (ph - 0.10f)) / 0.0016f);
		const float b = expf(-((ph - 0.26f) * (ph - 0.26f)) / 0.0020f);
		float v = a + 0.72f * b;
		if (v > 1.0f)
			v = 1.0f;
		return 0.05f + 0.95f * v;
	}
	default:
		return 1.0f;
	}
}

static inline RGB keyFxScale(RGB color, float level) {
	if (level < 0.0f)
		level = 0.0f;
	if (level > 1.0f)
		level = 1.0f;
	return RGB(
		(uint8_t)(color.r * level + 0.5f),
		(uint8_t)(color.g * level + 0.5f),
		(uint8_t)(color.b * level + 0.5f));
}

#endif
