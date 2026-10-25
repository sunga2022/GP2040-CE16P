#include "customtheme.h"
#include "fxutil.h"
#include "keyfx.h"
#include "storagemanager.h"

CustomTheme::CustomTheme(PixelMatrix &matrix) : Animation(matrix) {
  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUp);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDown);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeft);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRight);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3);
	}
}

static uint32_t customPackedForMask(const AnimationOptions &options, uint32_t mask) {
  switch (mask) {
    case GAMEPAD_MASK_DU: return options.customThemeUp;
    case GAMEPAD_MASK_DD: return options.customThemeDown;
    case GAMEPAD_MASK_DL: return options.customThemeLeft;
    case GAMEPAD_MASK_DR: return options.customThemeRight;
    case GAMEPAD_MASK_B1: return options.customThemeB1;
    case GAMEPAD_MASK_B2: return options.customThemeB2;
    case GAMEPAD_MASK_B3: return options.customThemeB3;
    case GAMEPAD_MASK_B4: return options.customThemeB4;
    case GAMEPAD_MASK_L1: return options.customThemeL1;
    case GAMEPAD_MASK_R1: return options.customThemeR1;
    case GAMEPAD_MASK_L2: return options.customThemeL2;
    case GAMEPAD_MASK_R2: return options.customThemeR2;
    case GAMEPAD_MASK_S1: return options.customThemeS1;
    case GAMEPAD_MASK_S2: return options.customThemeS2;
    case GAMEPAD_MASK_A1: return options.customThemeA1;
    case GAMEPAD_MASK_A2: return options.customThemeA2;
    case GAMEPAD_MASK_L3: return options.customThemeL3;
    case GAMEPAD_MASK_R3: return options.customThemeR3;
    default: return 0;
  }
}

bool CustomTheme::Animate(RGB (&frame)[100]) {
  UpdateTime();
  UpdatePresses(frame);

  const AnimationOptions &options = Storage::getInstance().getAnimationOptions();
  const float seconds = getMillis() * 0.001f;

  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index)
        continue;

      DecrementFadeCounter(matrix->pixels[r][c].index);

      const uint32_t packed = customPackedForMask(options, matrix->pixels[r][c].mask);
      const RGB idle = keyFxScale(RGB(packed), keyFxLevel(keyFxId(packed), seconds));
      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
        frame[matrix->pixels[r][c].positions[p]] = BlendColor(
            hitColor[matrix->pixels[r][c].index], idle, times[matrix->pixels[r][c].index]);
      }
    }
  }

  return true;
}

bool CustomTheme::HasTheme() {
  return theme.size() > 0;
}

void CustomTheme::ParameterUp() {
}

void CustomTheme::ParameterDown() {
}
