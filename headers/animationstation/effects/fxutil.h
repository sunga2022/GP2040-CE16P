#ifndef FXUTIL_H_
#define FXUTIL_H_

#include "animation.h"
#include <math.h>
#include <stdint.h>

extern uint32_t getMillis();

static inline float fxClamp(float x, float a = 0.0f, float b = 1.0f) {
	return x < a ? a : (x > b ? b : x);
}

static inline float fxSmooth01(float x) {
	x = fxClamp(x);
	return x * x * (3.0f - 2.0f * x);
}

static inline RGB fxHsv(float h, float s, float v) {
	h = h - floorf(h);
	if (h < 0.0f)
		h += 1.0f;
	s = fxClamp(s);
	v = fxClamp(v);
	const float c = v * s;
	const float xf = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
	const float m = v - c;
	float r = 0, g = 0, b = 0;
	switch ((int)floorf(h * 6.0f) % 6) {
	case 0: r = c; g = xf; break;
	case 1: r = xf; g = c; break;
	case 2: g = c; b = xf; break;
	case 3: g = xf; b = c; break;
	case 4: r = xf; b = c; break;
	default: r = c; b = xf; break;
	}
	return RGB(
		(uint8_t)((r + m) * 255.0f + 0.5f),
		(uint8_t)((g + m) * 255.0f + 0.5f),
		(uint8_t)((b + m) * 255.0f + 0.5f));
}

static inline RGB fxFirePalette(float t) {
	t = fxClamp(t);
	if (t < 0.18f)
		return fxHsv(0.02f, 1.0f, t / 0.18f * 0.25f);
	if (t < 0.45f)
		return fxHsv(0.00f + (t - 0.18f) * 0.08f, 1.0f, 0.25f + (t - 0.18f) * 2.2f);
	if (t < 0.72f)
		return fxHsv(0.045f + (t - 0.45f) * 0.16f, 1.0f, 1.0f);
	if (t < 0.88f)
		return fxHsv(0.10f, 0.75f - (t - 0.72f), 1.0f);
	return fxHsv(0.12f, 0.22f, 1.0f);
}

// Pico16N 14-LED physical map (left stick cluster + right 3x4 + L3/R3/Up).
static inline void fxLedXY(uint8_t ledIndex, int total, float &x, float &y) {
	static const float map[14][2] = {
		{0.08f, 0.38f}, // LEFT
		{0.22f, 0.38f}, // DOWN
		{0.36f, 0.46f}, // RIGHT
		{0.42f, 0.82f}, // UP
		{0.50f, 0.40f}, // B3 / X
		{0.64f, 0.30f}, // B4 / Y
		{0.78f, 0.32f}, // R1 / RB
		{0.92f, 0.34f}, // L1 / LB
		{0.49f, 0.58f}, // B1 / A
		{0.63f, 0.50f}, // B2 / B
		{0.77f, 0.52f}, // R2 / RT
		{0.91f, 0.54f}, // L2 / LT
		{0.57f, 0.70f}, // R3
		{0.50f, 0.20f}, // L3
	};
	if (ledIndex < 14) {
		x = map[ledIndex][0];
		y = map[ledIndex][1];
		return;
	}
	x = total > 1 ? (float)ledIndex / (float)(total - 1) : 0.5f;
	y = 0.5f;
}

static inline float fxAnimTime(int cycleMs) {
	const float spd = 40.0f / (float)(cycleMs < 10 ? 10 : cycleMs);
	return getMillis() * 0.001f * spd;
}

// Soft meteor/comet: head at (hx,hy), flying along (dx,dy), tail behind.
static inline float fxStreak(float px, float py, float hx, float hy,
		float dx, float dy, float length, float width) {
	const float vx = px - hx;
	const float vy = py - hy;
	const float along = vx * (-dx) + vy * (-dy);
	const float across = vx * (-dy) - vy * (-dx);
	if (along < -0.04f || along > length)
		return 0.0f;
	const float tail = 1.0f - along / length;
	return expf(-(across * across) / (width * width)) * tail * tail;
}

static inline RGB fxAdd(RGB a, RGB b) {
	int r = (int)a.r + (int)b.r;
	int g = (int)a.g + (int)b.g;
	int bl = (int)a.b + (int)b.b;
	return RGB(
		(uint8_t)(r > 255 ? 255 : r),
		(uint8_t)(g > 255 ? 255 : g),
		(uint8_t)(bl > 255 ? 255 : bl));
}

#endif
