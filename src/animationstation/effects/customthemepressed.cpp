#include "customthemepressed.h"
#include "GamepadState.h"
#include "storagemanager.h"

CustomThemePressed::CustomThemePressed(PixelMatrix &matrix) : Animation(matrix) {
  this->filtered = true;

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUpPressed);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDownPressed);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeftPressed);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRightPressed);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1Pressed);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2Pressed);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3Pressed);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4Pressed);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1Pressed);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1Pressed);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2Pressed);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2Pressed);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1Pressed);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2Pressed);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1Pressed);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2Pressed);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3Pressed);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3Pressed);
	}
}

CustomThemePressed::CustomThemePressed(PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pixels(&pixels) {
  this->filtered = true;

  AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
  if (animationOptions.hasCustomTheme)
	{
      theme[GAMEPAD_MASK_DU] = RGB(animationOptions.customThemeUpPressed);
      theme[GAMEPAD_MASK_DD] = RGB(animationOptions.customThemeDownPressed);
      theme[GAMEPAD_MASK_DL] = RGB(animationOptions.customThemeLeftPressed);
      theme[GAMEPAD_MASK_DR] = RGB(animationOptions.customThemeRightPressed);
      theme[GAMEPAD_MASK_B1] = RGB(animationOptions.customThemeB1Pressed);
      theme[GAMEPAD_MASK_B2] = RGB(animationOptions.customThemeB2Pressed);
      theme[GAMEPAD_MASK_B3] = RGB(animationOptions.customThemeB3Pressed);
      theme[GAMEPAD_MASK_B4] = RGB(animationOptions.customThemeB4Pressed);
      theme[GAMEPAD_MASK_L1] = RGB(animationOptions.customThemeL1Pressed);
      theme[GAMEPAD_MASK_R1] = RGB(animationOptions.customThemeR1Pressed);
      theme[GAMEPAD_MASK_L2] = RGB(animationOptions.customThemeL2Pressed);
      theme[GAMEPAD_MASK_R2] = RGB(animationOptions.customThemeR2Pressed);
      theme[GAMEPAD_MASK_S1] = RGB(animationOptions.customThemeS1Pressed);
      theme[GAMEPAD_MASK_S2] = RGB(animationOptions.customThemeS2Pressed);
      theme[GAMEPAD_MASK_A1] = RGB(animationOptions.customThemeA1Pressed);
      theme[GAMEPAD_MASK_A2] = RGB(animationOptions.customThemeA2Pressed);
      theme[GAMEPAD_MASK_L3] = RGB(animationOptions.customThemeL3Pressed);
      theme[GAMEPAD_MASK_R3] = RGB(animationOptions.customThemeR3Pressed);
	}
}

static RGB pressedColorForMask(const AnimationOptions &options, uint32_t mask) {
  switch (mask) {
    case GAMEPAD_MASK_DU: return RGB(options.customThemeUpPressed);
    case GAMEPAD_MASK_DD: return RGB(options.customThemeDownPressed);
    case GAMEPAD_MASK_DL: return RGB(options.customThemeLeftPressed);
    case GAMEPAD_MASK_DR: return RGB(options.customThemeRightPressed);
    case GAMEPAD_MASK_B1: return RGB(options.customThemeB1Pressed);
    case GAMEPAD_MASK_B2: return RGB(options.customThemeB2Pressed);
    case GAMEPAD_MASK_B3: return RGB(options.customThemeB3Pressed);
    case GAMEPAD_MASK_B4: return RGB(options.customThemeB4Pressed);
    case GAMEPAD_MASK_L1: return RGB(options.customThemeL1Pressed);
    case GAMEPAD_MASK_R1: return RGB(options.customThemeR1Pressed);
    case GAMEPAD_MASK_L2: return RGB(options.customThemeL2Pressed);
    case GAMEPAD_MASK_R2: return RGB(options.customThemeR2Pressed);
    case GAMEPAD_MASK_S1: return RGB(options.customThemeS1Pressed);
    case GAMEPAD_MASK_S2: return RGB(options.customThemeS2Pressed);
    case GAMEPAD_MASK_A1: return RGB(options.customThemeA1Pressed);
    case GAMEPAD_MASK_A2: return RGB(options.customThemeA2Pressed);
    case GAMEPAD_MASK_L3: return RGB(options.customThemeL3Pressed);
    case GAMEPAD_MASK_R3: return RGB(options.customThemeR3Pressed);
    default: return ColorBlack;
  }
}

bool CustomThemePressed::Animate(RGB (&frame)[100]) {
  const AnimationOptions &options = Storage::getInstance().getAnimationOptions();
  for (size_t r = 0; r != matrix->pixels.size(); r++) {
    for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
      if (matrix->pixels[r][c].index == NO_PIXEL.index || this->notInFilter(matrix->pixels[r][c]))
        continue;

      const RGB pressed = pressedColorForMask(options, matrix->pixels[r][c].mask);
      for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++)
        frame[matrix->pixels[r][c].positions[p]] = pressed;
    }
  }
  return true;
}

bool CustomThemePressed::HasTheme() {
  return theme.size() > 0;
}
