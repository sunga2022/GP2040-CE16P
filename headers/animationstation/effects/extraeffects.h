#ifndef EXTRA_EFFECTS_H_
#define EXTRA_EFFECTS_H_

#include "animation.h"

// 30 pad effects. Index 30 is the per-button custom theme, not one of these.
// 0-8 solid, 9-17 motion, 18-23 pulse, 24-29 ambient.
#define EXTRA_EFFECT_COUNT 30

class ExtraEffect : public Animation {
public:
  ExtraEffect(PixelMatrix &matrix, uint8_t effectId);
  ~ExtraEffect() {};

  bool Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

protected:
  uint8_t effectId = 0;
  absolute_time_t nextRunTime = nil_time;
};

#endif
