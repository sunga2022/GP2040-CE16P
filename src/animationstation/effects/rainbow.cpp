#include "rainbow.h"
#include "fxutil.h"
#include "storagemanager.h"
#include <algorithm>
#include <math.h>

Rainbow::Rainbow(PixelMatrix &matrix) : Animation(matrix) {
}

bool Rainbow::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return false;
  }

  UpdateTime();
  UpdatePresses(frame);

  // Slow seven-color gradient + breathing. Hue crawl is very slow so the
  // pad stays a rainbow, not a racing wave.
  const float t = getMillis() * 0.001f;
  const float breath = fxSmooth01((sinf(t * 0.70f) + 1.0f) * 0.5f);
  const float bri = 0.16f + 0.84f * breath;
  const float hueDrift = t * 0.018f;

  int total = 0;
  for (auto &col : matrix->pixels)
    for (auto &pixel : col)
      if (pixel.index != NO_PIXEL.index)
        total++;

  for (auto &col : matrix->pixels) {
    for (auto &pixel : col) {
      if (pixel.index == NO_PIXEL.index)
        continue;

      DecrementFadeCounter(pixel.index);

      uint8_t ledIndex = pixel.positions.empty() ? 0 : pixel.positions[0];
      float x, y;
      fxLedXY(ledIndex, total, x, y);
      RGB color = fxHsv(x * 0.86f + y * 0.10f + hueDrift, 1.0f, bri);

      for (auto &pos : pixel.positions)
        frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
    }
  }

  this->nextRunTime = make_timeout_time_ms(20);
  return true;
}

#define RAINBOW_CYCLE_INCREMENT   10
#define RAINBOW_CYCLE_MAX         INT16_MAX - RAINBOW_CYCLE_INCREMENT
#define RAINBOW_CYCLE_MIN         1         + RAINBOW_CYCLE_INCREMENT

void Rainbow::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime < RAINBOW_CYCLE_MAX) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime + RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = INT16_MAX;
  }
}

void Rainbow::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.rainbowCycleTime > RAINBOW_CYCLE_MIN) {
    animationOptions.rainbowCycleTime = animationOptions.rainbowCycleTime - RAINBOW_CYCLE_INCREMENT;
  } else {
    animationOptions.rainbowCycleTime = 1;
  }
}
