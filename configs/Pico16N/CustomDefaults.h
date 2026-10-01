#pragma once
#include <cstring>
#include "GamepadState.h"

namespace Pico16NDefaults {
struct Step { uint32_t mask; uint32_t holdFrames; uint32_t waitFrames; };
// Classic controls: X/Y/R1=LP/MP/HP, A/B/R2=LK/MK/HK.
// OD route: 2MK > cancel DR > 2HP > 236LK+HK > 6LK > 6HK > 236236HK.
// After OD Kazekama, 6HK is Kasai Thrust. Timings require in-game tuning.
static const Step kenRight[] = {
    {GAMEPAD_MASK_DD | GAMEPAD_MASK_B2, 2, 8},
    {GAMEPAD_MASK_B4 | GAMEPAD_MASK_B2, 2, 20},
    {GAMEPAD_MASK_DD | GAMEPAD_MASK_R1, 2, 10},
    {GAMEPAD_MASK_DD, 2, 0},
    {GAMEPAD_MASK_DD | GAMEPAD_MASK_DR, 2, 0},
    {GAMEPAD_MASK_DR | GAMEPAD_MASK_B1 | GAMEPAD_MASK_R2, 2, 27},
    {GAMEPAD_MASK_DR | GAMEPAD_MASK_B1, 2, 15},
    {GAMEPAD_MASK_DR | GAMEPAD_MASK_R2, 2, 10},
    {GAMEPAD_MASK_DD, 2, 0},
    {GAMEPAD_MASK_DD | GAMEPAD_MASK_DR, 2, 0},
    {GAMEPAD_MASK_DR, 2, 0},
    {GAMEPAD_MASK_DD, 2, 0},
    {GAMEPAD_MASK_DD | GAMEPAD_MASK_DR, 2, 0},
    {GAMEPAD_MASK_DR | GAMEPAD_MASK_R2, 2, 0},
};
static_assert(sizeof(kenRight) / sizeof(Step) <= 30, "Macro input limit");
inline uint32_t mirror(uint32_t mask) {
    return (mask & ~(GAMEPAD_MASK_DL | GAMEPAD_MASK_DR)) |
        ((mask & GAMEPAD_MASK_DL) ? GAMEPAD_MASK_DR : 0) |
        ((mask & GAMEPAD_MASK_DR) ? GAMEPAD_MASK_DL : 0);
}
inline void initKenMacro(Macro& macro, bool left) {
    macro = Macro Macro_init_default;
    macro.enabled = true;
    macro.macroType = ON_PRESS;
    macro.exclusive = true;
    macro.interruptible = false;
    macro.showFrames = true;
    macro.useMacroTriggerButton = false;
    macro.deprecatedMacroTriggerPin = -1;
    strncpy(macro.macroLabel, left ? "Ken OD combo facing left (tune)" : "Ken OD combo facing right (tune)", sizeof(macro.macroLabel)-1);
    macro.macroInputs_count = sizeof(kenRight) / sizeof(Step);
    for (unsigned int i = 0; i < macro.macroInputs_count; ++i) {
        auto& input = macro.macroInputs[i];
        input.buttonMask = left ? mirror(kenRight[i].mask) : kenRight[i].mask;
        input.duration = kenRight[i].holdFrames * 16666U;
        input.waitDuration = kenRight[i].waitFrames * 16666U;
    }
}
inline void applyPreset(Config& config) {
    if (config.migrations.pico16NCustomDefaultsApplied)
        return;
    config.gamepadOptions.inputMode = INPUT_MODE_XINPUT;
    config.gamepadOptions.dpadMode = DPAD_MODE_DIGITAL;
    config.gamepadOptions.socdMode = SOCD_MODE_NEUTRAL;
    config.gamepadOptions.ps4AuthType = INPUT_MODE_AUTH_TYPE_KEYS;
    config.gamepadOptions.ps5AuthType = INPUT_MODE_AUTH_TYPE_USB;
    config.addonOptions.psPassthroughOptions.enabled = false;
    config.addonOptions.turboOptions.enabled = true;
    config.addonOptions.turboOptions.shotCount = 30;
    config.peripheralOptions.blockUSB0.enabled = true;
    config.peripheralOptions.blockUSB0.dp = 3;
    config.peripheralOptions.blockUSB0.order = 0;
    config.peripheralOptions.blockUSB0.enable5v = -1;
    config.peripheralOptions.blockI2C0.enabled = true;
    config.peripheralOptions.blockI2C0.sda = 0;
    config.peripheralOptions.blockI2C0.scl = 1;
    config.displayOptions.enabled = true;
    config.displayOptions.buttonLayout = BUTTON_LAYOUT_BOARD_DEFINED_A;
    config.displayOptions.buttonLayoutRight = BUTTON_LAYOUT_BOARD_DEFINED_B;
    config.displayOptions.buttonLayoutOrientation = BUTTON_ORIENTATION_DEFAULT;
    config.displayOptions.inputHistoryEnabled = false;
    config.displayOptions.splashDuration = 3000;
    // Visible status uses a single header; leave the lower row for the board circles.
    config.displayOptions.inputMode = true;
    config.displayOptions.dpadMode = true;
    config.displayOptions.socdMode = true;
    config.displayOptions.turboMode = true;
    config.displayOptions.macroMode = false;
    config.displayOptions.profileMode = false;
    const GpioAction boardPins[] = {GPIO_PIN_00, GPIO_PIN_01, GPIO_PIN_02,
        GPIO_PIN_03, GPIO_PIN_04, GPIO_PIN_05, GPIO_PIN_06, GPIO_PIN_07,
        GPIO_PIN_08, GPIO_PIN_09, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12,
        GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_16, GPIO_PIN_17,
        GPIO_PIN_18, GPIO_PIN_19, GPIO_PIN_20, GPIO_PIN_21, GPIO_PIN_22,
        GPIO_PIN_23, GPIO_PIN_24};
    auto setBoardPins = [&](GpioMappings& mappings) {
        mappings.pins_count = 30;
        for (unsigned int i = 0; i < sizeof(boardPins)/sizeof(GpioAction); ++i)
            mappings.pins[i].action = boardPins[i];
        mappings.pins[28].action = GPIO_PIN_28;
        mappings.pins[29].action = GPIO_PIN_29;
    };
    setBoardPins(config.gpioMappings);
    for (unsigned int i=0; i<config.profileOptions.gpioMappingsSets_count; ++i)
        setBoardPins(config.profileOptions.gpioMappingsSets[i]);
    config.addonOptions.macroOptions.enabled = true;
    config.addonOptions.macroOptions.macroList_count = 6;
    initKenMacro(config.addonOptions.macroOptions.macroList[0], false);
    initKenMacro(config.addonOptions.macroOptions.macroList[1], true);
    // Ensure fields have export/storage presence; save() fills all protobuf flags.
    config.migrations.pico16NCustomDefaultsApplied = true;
}
inline void ensureBoardKeys(PS4Options& keys) {
    if (keys.rsaN.size == 256 && keys.rsaE.size == 4 && keys.rsaP.size == 128 &&
        keys.rsaQ.size == 128 && keys.serial.size == 16 && keys.signature.size == 256)
        return;
    keys.serial.size = sizeof(Pico16NKeys::serial);
    memcpy(keys.serial.bytes, Pico16NKeys::serial, sizeof(Pico16NKeys::serial));
    keys.signature.size = sizeof(Pico16NKeys::signature);
    memcpy(keys.signature.bytes, Pico16NKeys::signature, sizeof(Pico16NKeys::signature));
    keys.rsaN.size = sizeof(Pico16NKeys::rsaN);
    memcpy(keys.rsaN.bytes, Pico16NKeys::rsaN, sizeof(Pico16NKeys::rsaN));
    keys.rsaE.size = sizeof(Pico16NKeys::rsaE);
    memcpy(keys.rsaE.bytes, Pico16NKeys::rsaE, sizeof(Pico16NKeys::rsaE));
    keys.rsaD.size = sizeof(Pico16NKeys::rsaD);
    memcpy(keys.rsaD.bytes, Pico16NKeys::rsaD, sizeof(Pico16NKeys::rsaD));
    keys.rsaP.size = sizeof(Pico16NKeys::rsaP);
    memcpy(keys.rsaP.bytes, Pico16NKeys::rsaP, sizeof(Pico16NKeys::rsaP));
    keys.rsaQ.size = sizeof(Pico16NKeys::rsaQ);
    memcpy(keys.rsaQ.bytes, Pico16NKeys::rsaQ, sizeof(Pico16NKeys::rsaQ));
    keys.rsaDP.size = sizeof(Pico16NKeys::rsaDP);
    memcpy(keys.rsaDP.bytes, Pico16NKeys::rsaDP, sizeof(Pico16NKeys::rsaDP));
    keys.rsaDQ.size = sizeof(Pico16NKeys::rsaDQ);
    memcpy(keys.rsaDQ.bytes, Pico16NKeys::rsaDQ, sizeof(Pico16NKeys::rsaDQ));
    keys.rsaQP.size = sizeof(Pico16NKeys::rsaQP);
    memcpy(keys.rsaQP.bytes, Pico16NKeys::rsaQP, sizeof(Pico16NKeys::rsaQP));
    keys.rsaRN.size = sizeof(Pico16NKeys::rsaRN);
    memcpy(keys.rsaRN.bytes, Pico16NKeys::rsaRN, sizeof(Pico16NKeys::rsaRN));
}
}
