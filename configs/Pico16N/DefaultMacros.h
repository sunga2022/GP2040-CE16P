#ifndef PICO16N_DEFAULT_MACROS_H_
#define PICO16N_DEFAULT_MACROS_H_

#include "GamepadState.h"
#include "config.pb.h"
#include "enums.pb.h"

#define MACRO_FRAME_US 16666u

static inline void pico16nSetMacroInput(MacroInput& in, uint32_t mask, uint8_t holdFrames, uint8_t waitFrames) {
	in.buttonMask = mask;
	in.duration = holdFrames * MACRO_FRAME_US;
	in.waitDuration = waitFrames * MACRO_FRAME_US;
	in.has_buttonMask = true;
	in.has_duration = true;
	in.has_waitDuration = true;
}

// SF6 Ken optimal-ish confirm: 2MK > Drive Rush > 2HP > H Jinrai > Kazekama > Senka > SA3
static inline void pico16nFillKenCombo(Macro& macro, uint32_t toward, uint32_t downToward) {
	int i = 0;
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_DD | GAMEPAD_MASK_B2, 3, 1);
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_B4 | GAMEPAD_MASK_B2, 2, 10);
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_DD | GAMEPAD_MASK_R1, 3, 2);
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_DD, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], downToward, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], toward | GAMEPAD_MASK_R2, 2, 3);
	pico16nSetMacroInput(macro.macroInputs[i++], toward | GAMEPAD_MASK_B1, 2, 8);
	pico16nSetMacroInput(macro.macroInputs[i++], toward | GAMEPAD_MASK_R2, 2, 4);
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_DD, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], downToward, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], toward, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], GAMEPAD_MASK_DD, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], downToward, 1, 0);
	pico16nSetMacroInput(macro.macroInputs[i++], toward | GAMEPAD_MASK_R1, 4, 0);
	macro.macroInputs_count = i;
}

// Earlier BoardConfig marked A1/A2/S1/S2 as ASSIGNED_TO_ADDON, so already-flashed
// boards keep dead utility keys until these actions are written back.
static inline void restorePico16NUtilityButtons(Config& config) {
	const struct {
		uint8_t pin;
		GpioAction action;
	} buttons[] = {
		{6, GpioAction::BUTTON_PRESS_A2},
		{7, GpioAction::BUTTON_PRESS_A1},
		{8, GpioAction::BUTTON_PRESS_S1},
		{17, GpioAction::BUTTON_PRESS_S2},
	};

	for (const auto& button : buttons) {
		GpioMappingInfo& pin = config.gpioMappings.pins[button.pin];
		if (pin.action == GpioAction::ASSIGNED_TO_ADDON || pin.action == GpioAction::NONE) {
			pin.action = button.action;
		}
		for (int profile = 0; profile < config.profileOptions.gpioMappingsSets_count; profile++) {
			GpioMappingInfo& alt = config.profileOptions.gpioMappingsSets[profile].pins[button.pin];
			if (alt.action == GpioAction::ASSIGNED_TO_ADDON || alt.action == GpioAction::NONE) {
				alt.action = button.action;
			}
		}
	}
}

// Hardware buses on Pico16N: I2C OLED on GP0/GP1, USB host D+=GP4 / D-=GP3.
// Force these back on so a saved config that disabled USB host still works.
static inline void restorePico16NPeripherals(Config& config) {
	auto markAddon = [](GpioMappingInfo& pin) {
		pin.action = GpioAction::ASSIGNED_TO_ADDON;
	};
	const uint8_t addonPins[] = {0, 1, 3, 4, 28};
	for (uint8_t pin : addonPins) {
		markAddon(config.gpioMappings.pins[pin]);
		for (int profile = 0; profile < config.profileOptions.gpioMappingsSets_count; profile++) {
			markAddon(config.profileOptions.gpioMappingsSets[profile].pins[pin]);
		}
	}

	PeripheralOptions& peri = config.peripheralOptions;
	peri.blockI2C0.enabled = true;
	peri.blockI2C0.sda = 0;
	peri.blockI2C0.scl = 1;
	peri.blockI2C0.speed = 400000;
	peri.blockI2C0.has_enabled = true;
	peri.blockI2C0.has_sda = true;
	peri.blockI2C0.has_scl = true;
	peri.blockI2C0.has_speed = true;
	peri.has_blockI2C0 = true;

	peri.blockUSB0.enabled = true;
	peri.blockUSB0.dp = 4;
	peri.blockUSB0.order = 1;
	peri.blockUSB0.enable5v = -1;
	peri.blockUSB0.has_enabled = true;
	peri.blockUSB0.has_dp = true;
	peri.blockUSB0.has_order = true;
	peri.blockUSB0.has_enable5v = true;
	peri.has_blockUSB0 = true;
	config.has_peripheralOptions = true;

	config.addonOptions.gamepadUSBHostOptions.enabled = true;
	config.addonOptions.gamepadUSBHostOptions.has_enabled = true;
	config.addonOptions.has_gamepadUSBHostOptions = true;
	config.has_addonOptions = true;

	config.displayOptions.enabled = true;
	config.displayOptions.has_enabled = true;
	config.has_displayOptions = true;
}

// A failed web save used to write action 0 (NONE) onto most remappable
// pins. Heal that wiped state back to BoardConfig defaults, but only when
// the map looks empty. Do not touch pins the user already remapped.
static inline void restorePico16NWipedPinMap(Config& config) {
	const struct {
		uint8_t pin;
		GpioAction action;
	} buttons[] = {
		{2, GpioAction::BUTTON_PRESS_L3},
		{5, GpioAction::BUTTON_PRESS_FN},
		{9, GpioAction::BUTTON_PRESS_B4},
		{10, GpioAction::BUTTON_PRESS_R1},
		{11, GpioAction::BUTTON_PRESS_L1},
		{12, GpioAction::BUTTON_PRESS_B3},
		{13, GpioAction::BUTTON_PRESS_B2},
		{14, GpioAction::BUTTON_PRESS_R2},
		{15, GpioAction::BUTTON_PRESS_L2},
		{16, GpioAction::BUTTON_PRESS_RIGHT},
		{18, GpioAction::BUTTON_PRESS_B1},
		{19, GpioAction::BUTTON_PRESS_R3},
		{20, GpioAction::BUTTON_PRESS_L3},
		{21, GpioAction::BUTTON_PRESS_UP},
		{22, GpioAction::BUTTON_PRESS_DOWN},
		{23, GpioAction::BUTTON_PRESS_LEFT},
		{24, GpioAction::BUTTON_PRESS_TURBO},
		{29, GpioAction::BUTTON_PRESS_UP},
	};

	auto fillIfNone = [&](GpioMappings& mappings) {
		int noneCount = 0;
		for (const auto& button : buttons) {
			if (mappings.pins[button.pin].action == GpioAction::NONE)
				noneCount++;
		}
		if (noneCount < 8)
			return;
		for (const auto& button : buttons) {
			if (mappings.pins[button.pin].action == GpioAction::NONE)
				mappings.pins[button.pin].action = button.action;
		}
	};

	fillIfNone(config.gpioMappings);
	for (int profile = 0; profile < config.profileOptions.gpioMappingsSets_count; profile++) {
		fillIfNone(config.profileOptions.gpioMappingsSets[profile]);
	}
}

// Do not rewrite remappable GPIO actions here. Forcing GP20/GP29 on boot
// made web pin-map changes for M1/M2 disappear after reboot.
static inline void restorePico16NM1M2Pins(Config& config) {
	config.displayOptions.buttonLayout = BUTTON_LAYOUT_BOARD_DEFINED_A;
	config.displayOptions.buttonLayoutRight = BUTTON_LAYOUT_BOARD_DEFINED_B;
	config.displayOptions.has_buttonLayout = true;
	config.displayOptions.has_buttonLayoutRight = true;

	if (config.animationOptions.baseAnimationIndex > 30)
		config.animationOptions.baseAnimationIndex = LEDS_BASE_ANIMATION_INDEX;
	config.animationOptions.buttonColorIndex = 1;
	config.animationOptions.has_baseAnimationIndex = true;
	config.animationOptions.has_buttonColorIndex = true;
}

static inline void applyPico16NDefaultMacros(Config& config) {
	MacroOptions& opts = config.addonOptions.macroOptions;
	opts.macroList_count = 6;

	if (opts.macroList[0].macroInputs_count == 0) {
		pico16nFillKenCombo(opts.macroList[0], GAMEPAD_MASK_DR, GAMEPAD_MASK_DD | GAMEPAD_MASK_DR);
	}
	if (opts.macroList[1].macroInputs_count == 0) {
		pico16nFillKenCombo(opts.macroList[1], GAMEPAD_MASK_DL, GAMEPAD_MASK_DD | GAMEPAD_MASK_DL);
	}
}

#endif
