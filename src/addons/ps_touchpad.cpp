#include "addons/ps_touchpad.h"

#include "storagemanager.h"
#include "gamepad.h"
#include "drivers/ps4/PS4Descriptors.h"
#include "enums.pb.h"

#include "pico/time.h"

namespace {
	constexpr int32_t kSwipeSpeedX = 3000; // touchpad units per second (1920 wide)
	constexpr int32_t kSwipeSpeedY = 1500; // 943 tall
	constexpr uint32_t kMaxDeltaUs = 20000;

	uint16_t clampCoord(int32_t value, int32_t lo, int32_t hi) {
		if (value < lo) return static_cast<uint16_t>(lo);
		if (value > hi) return static_cast<uint16_t>(hi);
		return static_cast<uint16_t>(value);
	}
}

bool PSTouchpadAddon::available() {
	return PS_TOUCHPAD_ENABLED;
}

void PSTouchpadAddon::setup() {
	resetFinger();
	held = false;
	usedSwipe = false;
	lastUs = time_us_32();
}

void PSTouchpadAddon::resetFinger() {
	posX = PS4_TP_X_MAX / 2;
	posY = PS4_TP_Y_MAX / 2;
	fracX = 0;
	fracY = 0;
}

void PSTouchpadAddon::suppressDpad(Gamepad * gamepad) {
	gamepad->state.dpad &= ~GAMEPAD_MASK_DPAD;
	switch (gamepad->getActiveDpadMode()) {
		case DpadMode::DPAD_MODE_LEFT_ANALOG:
			gamepad->state.lx = GAMEPAD_JOYSTICK_MID;
			gamepad->state.ly = GAMEPAD_JOYSTICK_MID;
			break;
		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			gamepad->state.rx = GAMEPAD_JOYSTICK_MID;
			gamepad->state.ry = GAMEPAD_JOYSTICK_MID;
			break;
		default:
			break;
	}
}

void PSTouchpadAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	GamepadAux3DSensor & finger = gamepad->auxState.sensors.touchpad[0];
	const InputMode mode = gamepad->getOptions().inputMode;
	const uint32_t now = time_us_32();

	if (mode != InputMode::INPUT_MODE_PS4 &&
			mode != InputMode::INPUT_MODE_PS5 &&
			mode != InputMode::INPUT_MODE_P5GENERAL) {
		if (finger.enabled) {
			finger.enabled = false;
			finger.active = false;
		}
		held = false;
		usedSwipe = false;
		lastUs = now;
		return;
	}

	const bool a2 = gamepad->pressedA2();
	const bool s1 = gamepad->pressedS1();
	const uint8_t dir = static_cast<uint8_t>(
		(gamepad->state.dpad | (gamepad->state.dpadOriginal & GAMEPAD_MASK_DPAD)) & GAMEPAD_MASK_DPAD
	);

	if (!a2) {
		finger.enabled = false;
		finger.active = false;
		resetFinger();
		held = false;
		usedSwipe = false;
		lastUs = now;
		return;
	}

	if (!held) {
		resetFinger();
		usedSwipe = false;
		lastUs = now;
	}
	held = true;

	uint32_t dt = now - lastUs;
	lastUs = now;
	if (dt > kMaxDeltaUs) {
		dt = kMaxDeltaUs;
	}

	if (dir) {
		usedSwipe = true;
		if (dir & GAMEPAD_MASK_LEFT) {
			fracX -= kSwipeSpeedX * static_cast<int32_t>(dt);
		}
		if (dir & GAMEPAD_MASK_RIGHT) {
			fracX += kSwipeSpeedX * static_cast<int32_t>(dt);
		}
		if (dir & GAMEPAD_MASK_UP) {
			fracY -= kSwipeSpeedY * static_cast<int32_t>(dt);
		}
		if (dir & GAMEPAD_MASK_DOWN) {
			fracY += kSwipeSpeedY * static_cast<int32_t>(dt);
		}

		const int32_t stepX = fracX / 1000000;
		const int32_t stepY = fracY / 1000000;
		fracX -= stepX * 1000000;
		fracY -= stepY * 1000000;
		posX = clampCoord(static_cast<int32_t>(posX) + stepX, PS4_TP_X_MIN, PS4_TP_X_MAX);
		posY = clampCoord(static_cast<int32_t>(posY) + stepY, PS4_TP_Y_MIN, PS4_TP_Y_MAX);
		suppressDpad(gamepad);
	}

	if (usedSwipe) {
		finger.enabled = true;
		finger.active = true;
		finger.x = posX;
		finger.y = posY;
		if (s1) {
			// Keep A2 so the driver sends the touchpad click at the current finger.
			gamepad->state.buttons &= ~GAMEPAD_MASK_S1;
		} else {
			gamepad->state.buttons &= ~GAMEPAD_MASK_A2;
		}
	} else {
		finger.enabled = false;
		finger.active = false;
		if (s1) {
			gamepad->state.buttons &= ~GAMEPAD_MASK_S1;
		}
	}
}
