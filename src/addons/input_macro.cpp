#include "addons/input_macro.h"
#include "storagemanager.h"
#include "GamepadState.h"

#include "hardware/gpio.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static volatile bool s_inputMacroLedOverride = false;

static const char RECORD_LABEL_PREFIX[] = "录制 ";

static uint32_t quantizePressUs(uint32_t elapsedUs) {
    uint32_t frames = (elapsedUs + (MACRO_RECORD_FRAME_US / 2)) / MACRO_RECORD_FRAME_US;
    if (frames < 1) {
        frames = 1;
    }
    return frames * MACRO_RECORD_FRAME_US;
}

static uint32_t quantizeWaitUs(uint32_t elapsedUs) {
    uint32_t frames = (elapsedUs + (MACRO_RECORD_FRAME_US / 2)) / MACRO_RECORD_FRAME_US;
    return frames * MACRO_RECORD_FRAME_US;
}

bool inputMacroLedOverride() {
    return s_inputMacroLedOverride;
}

bool InputMacro::available() {
    // Recording uses FN and playback can use a trigger button, so the addon
    // stays loaded even when no MACRO pin is mapped.
    return true;
}

void InputMacro::setup() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
                macroButtonMask = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_1:
                macroPinMasks[0] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_2:
                macroPinMasks[1] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_3:
                macroPinMasks[2] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_4:
                macroPinMasks[3] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_5:
                macroPinMasks[4] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_6:
                macroPinMasks[5] = 1 << pin;
                break;
            default:
                break;
        }
    }

    inputMacroOptions = &Storage::getInstance().getAddonOptions().macroOptions;
    if (inputMacroOptions->macroBoardLedEnabled && isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
        boardLedEnabled = true;
    } else {
        boardLedEnabled = false;
    }
    boardLedEnabled = false;
    prevMacroInputPressed = false;
    recordPhase = RECORD_IDLE;
    recordFlashSaves = false;
    fnWasDown = false;
    fnSeenReleased = true;
    fnDownSince = 0;
    recordFlashesLeft = 0;
    recordLedOn = false;
    recordLedNextMs = 0;
    recordMask = 0;
    recordStepStart = 0;
    recordStepCount = 0;
    memset(recordStepMask, 0, sizeof(recordStepMask));
    memset(recordStepDuration, 0, sizeof(recordStepDuration));
    memset(recordStepWait, 0, sizeof(recordStepWait));
    s_inputMacroLedOverride = false;
    if (isValidPin(BOARD_LED_PIN)) {
        gpio_init(BOARD_LED_PIN);
        gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
    }
    reset();
}


void InputMacro::reset() {
    macroPosition = -1;
    pressedMacro = -1;
    isMacroRunning = false;
    macroStartTime = 0;
    macroInputPosition = 0;
    isMacroTriggerHeld = false;
    macroInputHoldTime = INPUT_HOLD_US;
    if (boardLedEnabled) {
        gpio_put(BOARD_LED_PIN, 0);
    }
}

void InputMacro::restart(Macro& macro) {
    macroStartTime = currentMicros;
    macroInputPosition = 0;
    MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
    uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
    macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
}

void InputMacro::checkMacroPress() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t allPins = gamepad->debouncedGpio;

    // Go through our macro list
    pressedMacro = -1;
    for(int i = 0; i < MAX_MACRO_LIMIT; i++) {
        if ( inputMacroOptions->macroList[i].enabled == false ) // Skip disabled macros
            continue;
        Macro * macro = &inputMacroOptions->macroList[i];
        if ( macro->useMacroTriggerButton ) {
            // The assigned button alone starts the macro. A MACRO pin is not required.
            uint32_t trigger = macro->macroTriggerButton;
            if ((gamepad->state.buttons & trigger) ||
                    (gamepad->state.dpad & (trigger >> 16))) {
                pressedMacro = i;
                break;
            }
        } else if ( macroPinMasks[i] && (allPins & macroPinMasks[i]) ) {
            // Use Pin Manager for Macro Trigger
            pressedMacro = i;
            break;
        }
    }
}

void InputMacro::checkMacroAction() {
    bool macroInputPressed = (pressedMacro != -1); // Was any macro input pressed?

    // Is our pressed macro button different from our current macro AND no macro is running?
    if ( pressedMacro != macroPosition && !isMacroRunning ) {
        macroPosition = pressedMacro; // move our position to that macro
    }

    bool newPress = macroInputPressed && (prevMacroInputPressed ^ macroInputPressed);

    if (macroPosition < 0 || macroPosition >= MAX_MACRO_LIMIT) {
        prevMacroInputPressed = macroInputPressed;
        return;
    }

    // Check to see if we should change the current macro (or turn off based on input)
    if ( inputMacroOptions->macroList[macroPosition].macroType == ON_PRESS ) {
        // START Macro: On Press or On Hold Repeat
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        }
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT ) {
        isMacroTriggerHeld = macroInputPressed;
    } else if ( inputMacroOptions->macroList[macroPosition].macroType == ON_TOGGLE ) {
        //isMacroTriggerHeld = macroInputPressed;
        if (!isMacroRunning ) {
            isMacroTriggerHeld = newPress;
        } else if (isMacroRunning && newPress) {
            // STOP Macro: Toggle on new press
            reset(); // Stop Macro: Toggle
            prevMacroInputPressed = macroInputPressed;
            return;
        }
    }

    prevMacroInputPressed = macroInputPressed;
    if (!isMacroRunning && isMacroTriggerHeld) {
        // New Macro to run
        macroPosition = pressedMacro; // Set current macro
        Macro& macro = inputMacroOptions->macroList[macroPosition];
        MacroInput& macroInput = macro.macroInputs[macroInputPosition];
        uint32_t macroInputDuration = macroInput.duration + macroInput.waitDuration;
        macroInputHoldTime = macroInputDuration <= 0 ? INPUT_HOLD_US : macroInputDuration;
        isMacroRunning = true;
        macroStartTime = getMicro(); // current time
    }
}

void InputMacro::runCurrentMacro() {
    // Do nothing if macro is not currently running
    if (!isMacroRunning ||
            macroPosition == -1)
        return;

    Macro& macro = inputMacroOptions->macroList[macroPosition];

    // Stop Macro if released (ON PRESS & ON HOLD REPEAT)
    if (inputMacroOptions->macroList[macroPosition].macroType == ON_HOLD_REPEAT &&
            !isMacroTriggerHeld ) {
        reset();
        return;
    }

    MacroInput& macroInput = macro.macroInputs[macroInputPosition];
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    currentMicros = getMicro();

    if (!macro.interruptible && macro.exclusive) {
        // Prevent any other inputs from modifying our input (Exclusive)
        gamepad->state.dpad = 0;
        gamepad->state.buttons = 0;
    } else {
        if (macro.useMacroTriggerButton) {
            // Remove the trigger button from the input state
            gamepad->state.dpad &= ~(macro.macroTriggerButton >> 16);
            gamepad->state.buttons &= ~macro.macroTriggerButton;
        }
        if (macro.interruptible &&
            (gamepad->state.buttons != 0 || gamepad->state.dpad != 0)) {
            // Macro is interruptible and a user pressed something
            reset();
            return;
        }
    }

    // Have we elapsed the input hold time?
    if ((currentMicros - macroStartTime) >= macroInputHoldTime) {
        macroStartTime = currentMicros;
        macroInputPosition++;
        
        if (macroInputPosition >= (macro.macroInputs_count)) {
            if ( macro.macroType == ON_PRESS ) {
                reset(); // On press = no more macro
            } else {
                restart(macro); // On Hold-Repeat or On Toggle = start macro again
            }
        } else {
            MacroInput& newMacroInput = macro.macroInputs[macroInputPosition];
            uint32_t newMacroInputDuration = newMacroInput.duration + newMacroInput.waitDuration;
            macroInputHoldTime = newMacroInputDuration <= 0 ? INPUT_HOLD_US : newMacroInputDuration;
        }
    }

    // Check if we should still hold this macro input based on duration
    if ((currentMicros - macroStartTime) <= macroInput.duration) {
        uint32_t buttonMask = macroInput.buttonMask;
        if (buttonMask & GAMEPAD_MASK_DU) {
            gamepad->state.dpad |= GAMEPAD_MASK_UP;
        }
        if (buttonMask & GAMEPAD_MASK_DD) {
            gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
        }
        if (buttonMask & GAMEPAD_MASK_DL) {
            gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
        }
        if (buttonMask & GAMEPAD_MASK_DR) {
            gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
        }
        gamepad->state.buttons |= buttonMask;

        // Macro LED is on if we're currently running and inputs are doing something (wait-timers turn it off)
        if (boardLedEnabled) {
            gpio_put(BOARD_LED_PIN, (gamepad->state.dpad || gamepad->state.buttons) ? 1 : 0);
        }
    }
}

void InputMacro::preprocess()
{
    updateRecorder();
    if (recordPhase != RECORD_IDLE) {
        return;
    }

    FocusModeOptions * focusModeOptions = &Storage::getInstance().getAddonOptions().focusModeOptions;
    if (focusModeOptions->enabled && focusModeOptions->macroLockEnabled) {
        Gamepad * gamepad = Storage::getInstance().GetGamepad();
        // Override Toggle Pressed OR focus mode pin is set
        if (focusModeOptions->overrideEnabled ||
            (gamepad->mapFocusMode->pinMask && (gamepad->debouncedGpio & gamepad->mapFocusMode->pinMask))) {
            return;
        }
    }

    checkMacroPress();
    checkMacroAction();
    runCurrentMacro();
}

void InputMacro::reinit() {
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    macroButtonMask = 0;
    memset(macroPinMasks, 0, sizeof(macroPinMasks));
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch( pinMappings[pin].action ) {
            case GpioAction::BUTTON_PRESS_MACRO:
                macroButtonMask = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_1:
                macroPinMasks[0] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_2:
                macroPinMasks[1] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_3:
                macroPinMasks[2] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_4:
                macroPinMasks[3] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_5:
                macroPinMasks[4] = 1 << pin;
                break;
            case GpioAction::BUTTON_PRESS_MACRO_6:
                macroPinMasks[5] = 1 << pin;
                break;
            default:
                break;
        }
    }
}

uint32_t InputMacro::captureRecordMask() {
    // Read the pins themselves so turbo flicker and SOCD are not stored as steps.
    const uint32_t recordedButtons =
        GAMEPAD_MASK_B1 | GAMEPAD_MASK_B2 | GAMEPAD_MASK_B3 | GAMEPAD_MASK_B4 |
        GAMEPAD_MASK_L1 | GAMEPAD_MASK_R1 | GAMEPAD_MASK_L2 | GAMEPAD_MASK_R2 |
        GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3 |
        GAMEPAD_MASK_E1 | GAMEPAD_MASK_E2 | GAMEPAD_MASK_E3;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t values = gamepad->debouncedGpio;
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    uint32_t mask = 0;
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        if ((values & (1u << pin)) == 0) {
            continue;
        }
        switch (pinMappings[pin].action) {
            case GpioAction::BUTTON_PRESS_UP:
            case GpioAction::DIGITAL_DIRECTION_UP:
                mask |= GAMEPAD_MASK_DU;
                break;
            case GpioAction::BUTTON_PRESS_DOWN:
            case GpioAction::DIGITAL_DIRECTION_DOWN:
                mask |= GAMEPAD_MASK_DD;
                break;
            case GpioAction::BUTTON_PRESS_LEFT:
            case GpioAction::DIGITAL_DIRECTION_LEFT:
                mask |= GAMEPAD_MASK_DL;
                break;
            case GpioAction::BUTTON_PRESS_RIGHT:
            case GpioAction::DIGITAL_DIRECTION_RIGHT:
                mask |= GAMEPAD_MASK_DR;
                break;
            case GpioAction::BUTTON_PRESS_B1: mask |= GAMEPAD_MASK_B1; break;
            case GpioAction::BUTTON_PRESS_B2: mask |= GAMEPAD_MASK_B2; break;
            case GpioAction::BUTTON_PRESS_B3: mask |= GAMEPAD_MASK_B3; break;
            case GpioAction::BUTTON_PRESS_B4: mask |= GAMEPAD_MASK_B4; break;
            case GpioAction::BUTTON_PRESS_L1: mask |= GAMEPAD_MASK_L1; break;
            case GpioAction::BUTTON_PRESS_R1: mask |= GAMEPAD_MASK_R1; break;
            case GpioAction::BUTTON_PRESS_L2: mask |= GAMEPAD_MASK_L2; break;
            case GpioAction::BUTTON_PRESS_R2: mask |= GAMEPAD_MASK_R2; break;
            case GpioAction::BUTTON_PRESS_L3: mask |= GAMEPAD_MASK_L3; break;
            case GpioAction::BUTTON_PRESS_R3: mask |= GAMEPAD_MASK_R3; break;
            case GpioAction::BUTTON_PRESS_E1: mask |= GAMEPAD_MASK_E1; break;
            case GpioAction::BUTTON_PRESS_E2: mask |= GAMEPAD_MASK_E2; break;
            case GpioAction::BUTTON_PRESS_E3: mask |= GAMEPAD_MASK_E3; break;
            case GpioAction::CUSTOM_BUTTON_COMBO:
                mask |= pinMappings[pin].customButtonMask & recordedButtons;
                if (pinMappings[pin].customDpadMask & GAMEPAD_MASK_UP) mask |= GAMEPAD_MASK_DU;
                if (pinMappings[pin].customDpadMask & GAMEPAD_MASK_DOWN) mask |= GAMEPAD_MASK_DD;
                if (pinMappings[pin].customDpadMask & GAMEPAD_MASK_LEFT) mask |= GAMEPAD_MASK_DL;
                if (pinMappings[pin].customDpadMask & GAMEPAD_MASK_RIGHT) mask |= GAMEPAD_MASK_DR;
                break;
            default:
                break;
        }
    }
    return mask;
}

bool InputMacro::recordingHasContent() const {
    return recordStepCount > 0 || recordMask != 0;
}

uint32_t InputMacro::nextRecordSequence() const {
    uint32_t maxSeq = 0;
    size_t prefixLen = strlen(RECORD_LABEL_PREFIX);
    for (int i = 0; i < MAX_MACRO_LIMIT; i++) {
        const char* label = inputMacroOptions->macroList[i].macroLabel;
        if (strncmp(label, RECORD_LABEL_PREFIX, prefixLen) == 0) {
            int seq = atoi(label + prefixLen);
            if (seq > (int)maxSeq) {
                maxSeq = (uint32_t)seq;
            }
        }
    }
    return maxSeq + 1;
}

void InputMacro::sampleRecording(uint64_t now) {
    uint32_t mask = captureRecordMask();
    if (mask == recordMask || recordStepCount >= MAX_MACRO_INPUT_LIMIT) {
        return;
    }

    if (recordMask != 0) {
        uint32_t elapsed = (uint32_t)(now - recordStepStart);
        recordStepMask[recordStepCount] = recordMask;
        recordStepDuration[recordStepCount] = quantizePressUs(elapsed);
        recordStepWait[recordStepCount] = 0;
        recordStepCount++;
    } else if (recordStepCount > 0) {
        uint32_t elapsed = (uint32_t)(now - recordStepStart);
        recordStepWait[recordStepCount - 1] = quantizeWaitUs(elapsed);
    }

    recordMask = (recordStepCount >= MAX_MACRO_INPUT_LIMIT) ? 0 : mask;
    recordStepStart = now;
}

void InputMacro::sealOpenStep(uint64_t now) {
    if (recordMask == 0 || recordStepCount >= MAX_MACRO_INPUT_LIMIT) {
        recordMask = 0;
        return;
    }
    uint32_t elapsed = (uint32_t)(now - recordStepStart);
    recordStepMask[recordStepCount] = recordMask;
    recordStepDuration[recordStepCount] = quantizePressUs(elapsed);
    recordStepWait[recordStepCount] = 0;
    recordStepCount++;
    recordMask = 0;
}

void InputMacro::beginRecordFlash(int flashes, bool saving) {
    recordPhase = RECORD_FLASHING;
    recordFlashSaves = saving;
    recordFlashesLeft = flashes;
    recordLedOn = false;
    recordLedNextMs = 0;
    fnSeenReleased = false;
    fnWasDown = false;
    s_inputMacroLedOverride = true;
    reset();
}

void InputMacro::tickRecordFlash() {
    uint32_t nowMs = getMillis();
    if (recordLedNextMs != 0 && (int32_t)(nowMs - recordLedNextMs) < 0) {
        return;
    }

    if (!recordLedOn) {
        recordLedOn = true;
        gpio_put(BOARD_LED_PIN, 1);
        recordLedNextMs = nowMs + MACRO_RECORD_FLASH_MS;
        return;
    }

    recordLedOn = false;
    gpio_put(BOARD_LED_PIN, 0);
    recordFlashesLeft--;
    recordLedNextMs = nowMs + MACRO_RECORD_FLASH_MS;
    if (recordFlashesLeft > 0) {
        return;
    }

    s_inputMacroLedOverride = false;
    if (recordFlashSaves) {
        commitRecording();
    } else {
        recordMask = 0;
        recordStepCount = 0;
        recordStepStart = getMicro();
        memset(recordStepMask, 0, sizeof(recordStepMask));
        memset(recordStepDuration, 0, sizeof(recordStepDuration));
        memset(recordStepWait, 0, sizeof(recordStepWait));
        recordPhase = RECORD_ACTIVE;
        return;
    }
    recordPhase = RECORD_IDLE;
    fnSeenReleased = false;
}

void InputMacro::commitRecording() {
    if (recordStepCount <= 0) {
        return;
    }

    uint32_t seq = nextRecordSequence();
    int slot = (int)((seq - 1) % MAX_MACRO_LIMIT);
    Macro& macro = inputMacroOptions->macroList[slot];

    if (macro.macroType != MacroType::ON_PRESS &&
            macro.macroType != MacroType::ON_HOLD_REPEAT &&
            macro.macroType != MacroType::ON_TOGGLE) {
        macro.macroType = MacroType::ON_PRESS;
    }
    macro.has_macroType = true;
    macro.enabled = true;
    macro.has_enabled = true;
    macro.showFrames = true;
    macro.has_showFrames = true;
    macro.has_useMacroTriggerButton = true;
    macro.has_macroTriggerButton = true;
    macro.has_exclusive = true;
    macro.has_interruptible = true;

    snprintf(macro.macroLabel, sizeof(macro.macroLabel), "%s%u", RECORD_LABEL_PREFIX, seq);
    macro.has_macroLabel = true;

    for (int i = 0; i < recordStepCount; i++) {
        MacroInput& input = macro.macroInputs[i];
        input.buttonMask = recordStepMask[i];
        input.has_buttonMask = true;
        input.duration = recordStepDuration[i];
        input.has_duration = true;
        input.waitDuration = recordStepWait[i];
        input.has_waitDuration = true;
    }
    macro.macroInputs_count = recordStepCount;
    inputMacroOptions->macroList_count = MAX_MACRO_LIMIT;
    inputMacroOptions->enabled = true;
    inputMacroOptions->has_enabled = true;

    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));

    recordStepCount = 0;
    recordMask = 0;
}

void InputMacro::updateRecorder() {
    if (recordPhase == RECORD_FLASHING) {
        tickRecordFlash();
        return;
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    bool fnDown = (gamepad->state.aux & AUX_MASK_FUNCTION) != 0;
    uint64_t now = getMicro();

    if (recordPhase == RECORD_ACTIVE) {
        if (!fnDown) {
            fnSeenReleased = true;
            fnWasDown = false;
        }
        sampleRecording(now);
        if (fnSeenReleased && fnDown) {
            if (!fnWasDown) {
                fnWasDown = true;
                fnDownSince = now;
            } else if ((now - fnDownSince) >= MACRO_RECORD_HOLD_US) {
                if (recordingHasContent()) {
                    sealOpenStep(now);
                    beginRecordFlash(3, true);
                } else {
                    recordPhase = RECORD_IDLE;
                    recordMask = 0;
                    recordStepCount = 0;
                    fnSeenReleased = false;
                    fnWasDown = false;
                    reset();
                }
            }
        }
        return;
    }

    if (!fnDown) {
        fnWasDown = false;
        fnSeenReleased = true;
        return;
    }
    if (!fnSeenReleased) {
        return;
    }
    if (!fnWasDown) {
        fnWasDown = true;
        fnDownSince = now;
        return;
    }
    if ((now - fnDownSince) >= MACRO_RECORD_HOLD_US) {
        beginRecordFlash(5, false);
    }
}
