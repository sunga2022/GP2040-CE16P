#include "chase.h"
#include "fxutil.h"
#include "storagemanager.h"
#include <math.h>

#define CHASE_CYCLE_INCREMENT   10
#define CHASE_CYCLE_MAX         INT16_MAX/2
#define CHASE_CYCLE_MIN         10

Chase::Chase(PixelMatrix &matrix) : Animation(matrix) {
}

bool Chase::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return false;
  }

  UpdateTime();
  UpdatePresses(frame);

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  const float tt = fxAnimTime(animationOptions.chaseCycleTime);
  float head = fmodf(tt * 0.32f, 1.22f) - 0.11f;

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
      float g = fxStreak(x, y, head, 0.48f, 1.0f, 0.0f, 0.52f, 0.13f);
      RGB color = fxHsv(head * 0.85f + tt * 0.14f, 1.0f, g);

      for (auto &pos : pixel.positions)
        frame[pos] = BlendColor(hitColor[pixel.index], color, times[pixel.index]);
    }
  }

  if (animationOptions.chaseCycleTime < CHASE_CYCLE_MIN) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MIN;
  } else if (animationOptions.chaseCycleTime > CHASE_CYCLE_MAX) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MAX;
  }

  this->nextRunTime = make_timeout_time_ms(16);
  return true;
}

bool Chase::IsChasePixel(int i) {
  return true;
}

int Chase::WheelFrame(int i) {
  return 0;
}

void Chase::ParameterUp() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.chaseCycleTime = animationOptions.chaseCycleTime + CHASE_CYCLE_INCREMENT;
  if (animationOptions.chaseCycleTime > CHASE_CYCLE_MAX) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MAX;
  }
}

void Chase::ParameterDown() {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  animationOptions.chaseCycleTime =animationOptions.chaseCycleTime - CHASE_CYCLE_INCREMENT;
  if (animationOptions.chaseCycleTime < CHASE_CYCLE_MIN) {
    animationOptions.chaseCycleTime = CHASE_CYCLE_MIN;
  }
}
