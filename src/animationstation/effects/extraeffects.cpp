#include "extraeffects.h"
#include "fxutil.h"
#include "keyfx.h"
#include "storagemanager.h"
#include <math.h>
#include <vector>

#define EXTRA_CYCLE_INCREMENT 10
#define EXTRA_CYCLE_MAX       (INT16_MAX - EXTRA_CYCLE_INCREMENT)
#define EXTRA_CYCLE_MIN       (1 + EXTRA_CYCLE_INCREMENT)

static const RGB kPadColors[8] = {
	RGB(255, 16, 16),
	RGB(255, 214, 0),
	RGB(28, 72, 255),
	RGB(0, 230, 56),
	RGB(0, 224, 232),
	RGB(255, 112, 0),
	RGB(156, 32, 255),
	RGB(255, 72, 168),
};

static RGB lerpRgb(RGB a, RGB b, float t) {
	t = fxClamp(t);
	return RGB(
		(uint8_t)(a.r + (b.r - a.r) * t + 0.5f),
		(uint8_t)(a.g + (b.g - a.g) * t + 0.5f),
		(uint8_t)(a.b + (b.b - a.b) * t + 0.5f));
}

static RGB paletteAt(float pos) {
	pos = fmodf(pos, 8.0f);
	if (pos < 0.0f)
		pos += 8.0f;
	const int i = (int)pos;
	const float f = fxSmooth01(pos - (float)i);
	return lerpRgb(kPadColors[i], kPadColors[(i + 1) & 7], f);
}

static float hash01(int n) {
	n = (n << 13) ^ n;
	n = n * (n * n * 15731 + 789221) + 1376312589;
	return (float)(n & 0x7fffffff) / 2147483647.0f;
}

ExtraEffect::ExtraEffect(PixelMatrix &matrix, uint8_t effectId)
	: Animation(matrix), effectId(effectId) {
}

bool ExtraEffect::Animate(RGB (&frame)[100]) {
	if (!time_reached(this->nextRunTime))
		return false;

	UpdateTime();
	UpdatePresses(frame);

	std::vector<Pixel> leds;
	for (auto &col : matrix->pixels) {
		for (auto &pixel : col) {
			if (pixel.index == NO_PIXEL.index)
				continue;
			DecrementFadeCounter(pixel.index);
			leds.push_back(pixel);
		}
	}

	AnimationOptions &animationOptions = Storage::getInstance().getAnimationOptions();
	const float tt = fxAnimTime(animationOptions.rainbowCycleTime);
	const float sec = getMillis() * 0.001f;
	const int n = (int)leds.size();

	auto paint = [&](const Pixel &pixel, RGB color) {
		for (auto &pos : pixel.positions)
			frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
	};

	const uint8_t id = effectId < EXTRA_EFFECT_COUNT ? effectId : 0;

	for (int i = 0; i < n; i++) {
		const Pixel &pixel = leds[i];
		float x, y;
		const uint8_t idx = pixel.positions.empty() ? 0 : pixel.positions[0];
		fxLedXY(idx, n, x, y);
		RGB color = kPadColors[0];

		switch (id) {
		case 0: color = kPadColors[0]; break;
		case 1: color = kPadColors[1]; break;
		case 2: color = kPadColors[2]; break;
		case 3: color = kPadColors[3]; break;
		case 4: color = kPadColors[4]; break;
		case 5: color = kPadColors[5]; break;
		case 6: color = kPadColors[6]; break;
		case 7: color = kPadColors[7]; break;
		case 8: color = RGB(255, 214, 168); break;
		case 9:
			color = keyFxScale(kPadColors[0], keyFxLevel(KEY_FX_BREATH, sec));
			break;
		case 10:
			color = keyFxScale(fxHsv(fmodf(tt * 0.15f, 1.0f), 1.0f, 1.0f), keyFxLevel(KEY_FX_BREATH, sec));
			break;
		case 11:
			color = keyFxScale(paletteAt(x * 7.0f + tt * 0.85f), 0.25f + 0.75f * keyFxLevel(KEY_FX_BREATH, sec * 0.6f));
			break;
		case 12:
			color = keyFxScale(paletteAt(y * 7.0f + tt * 0.75f), 0.25f + 0.75f * keyFxLevel(KEY_FX_BREATH, sec * 0.55f));
			break;
		case 13:
			color = keyFxScale(paletteAt((float)i * (8.0f / (n > 0 ? n : 1)) + tt * 0.9f), keyFxLevel(KEY_FX_BREATH, sec * 0.8f));
			break;
		case 14: {
			const float d = hypotf(x - 0.52f, y - 0.48f);
			color = keyFxScale(paletteAt(d * 10.0f + tt * 0.7f), keyFxLevel(KEY_FX_BREATH, sec * 0.7f));
			break;
		}
		case 15: {
			const float head = fmodf(tt * 0.35f, 1.2f);
			const float band = expf(-((x - head) * (x - head)) / 0.02f);
			color = keyFxScale(paletteAt(x * 6.0f + tt), 0.08f + 0.92f * band);
			break;
		}
		case 16: {
			const float slide = (sinf(sec * 1.4f) + 1.0f) * 0.5f;
			color = lerpRgb(kPadColors[0], kPadColors[2], fxSmooth01(fxClamp((x - slide + 0.35f) * 1.6f)));
			break;
		}
		case 17: {
			const int slot = (int)(sec * 5.0f);
			const int pick = n > 0 ? (int)(hash01(slot * 17 + 3) * n) % n : 0;
			const float pop = keyFxLevel(KEY_FX_JUMP, sec);
			color = (i == pick) ? keyFxScale(kPadColors[1], pop) : RGB(18, 8, 4);
			break;
		}
		case 18:
			color = keyFxScale(kPadColors[4], keyFxLevel(KEY_FX_FAST, sec));
			break;
		case 19:
			color = keyFxScale(kPadColors[6], keyFxLevel(KEY_FX_SLOW, sec));
			break;
		case 20:
			color = keyFxScale(RGB(255, 24, 48), keyFxLevel(KEY_FX_HEART, sec));
			break;
		case 21: {
			const float head = fmodf(sec * 0.45f, 1.15f);
			const float band = expf(-((x - head) * (x - head)) / 0.012f);
			color = keyFxScale(RGB(255, 255, 255), 0.04f + 0.96f * band);
			break;
		}
		case 22: {
			const float drop = fmodf(y * 0.15f + sec * 0.55f, 1.0f);
			const float band = expf(-((drop - 0.15f) * (drop - 0.15f)) / 0.004f);
			color = keyFxScale(kPadColors[2], 0.05f + 0.95f * band);
			break;
		}
		case 23: {
			const float headx = fmodf(sec * 0.35f, 1.3f) - 0.1f;
			const float heady = 0.85f - headx * 0.55f;
			const float streak = fxStreak(x, y, headx, heady, 0.72f, -0.40f, 0.42f, 0.08f);
			color = keyFxScale(fxFirePalette(0.55f + streak * 0.45f), 0.05f + streak);
			break;
		}
		case 24:
			color = fxHsv(0.45f + 0.18f * sinf(x * 4.0f + sec * 0.6f) + y * 0.08f, 0.75f, 0.35f + 0.65f * keyFxLevel(KEY_FX_BREATH, sec + x));
			break;
		case 25:
			color = keyFxScale(kPadColors[i & 7], 0.35f + 0.65f * keyFxLevel(KEY_FX_BREATH, sec + i * 0.35f));
			break;
		case 26: {
			const float mix = (sinf(sec * 0.7f) + 1.0f) * 0.5f;
			color = lerpRgb(RGB(255, 140, 32), RGB(40, 120, 255), mix);
			color = keyFxScale(color, 0.45f + 0.55f * keyFxLevel(KEY_FX_BREATH, sec));
			break;
		}
		case 27: {
			const RGB triad[3] = { kPadColors[0], kPadColors[1], kPadColors[2] };
			const int shift = ((int)(sec * 1.5f) + i) % 3;
			color = triad[shift];
			break;
		}
		case 28: {
			const int slot = (int)(sec * 0.8f);
			const float tw = hash01(i * 13 + slot);
			color = tw > 0.72f ? keyFxScale(RGB(210, 220, 255), keyFxLevel(KEY_FX_SLOW, sec + i)) : RGB(6, 8, 16);
			break;
		}
		default: {
			const float flick = 0.55f + 0.45f * hash01(i * 9 + (int)(sec * 12.0f));
			color = keyFxScale(fxFirePalette(0.35f + 0.5f * y), flick * keyFxLevel(KEY_FX_BREATH, sec * 1.3f + x));
			break;
		}
		}

		paint(pixel, color);
	}

	this->nextRunTime = make_timeout_time_ms(16);
	return true;
}

void ExtraEffect::ParameterUp() {
	AnimationOptions &animationOptions = Storage::getInstance().getAnimationOptions();
	if (animationOptions.rainbowCycleTime < EXTRA_CYCLE_MAX)
		animationOptions.rainbowCycleTime += EXTRA_CYCLE_INCREMENT;
	else
		animationOptions.rainbowCycleTime = INT16_MAX;
}

void ExtraEffect::ParameterDown() {
	AnimationOptions &animationOptions = Storage::getInstance().getAnimationOptions();
	if (animationOptions.rainbowCycleTime > EXTRA_CYCLE_MIN)
		animationOptions.rainbowCycleTime -= EXTRA_CYCLE_INCREMENT;
	else
		animationOptions.rainbowCycleTime = 1;
}
