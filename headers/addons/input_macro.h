#ifndef _InputMacro_H
#define _InputMacro_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef INPUT_MACRO_ENABLED
#define INPUT_MACRO_ENABLED 0
#endif

#ifndef BOARD_LED_PIN
#ifndef PICO_DEFAULT_LED_PIN
#define BOARD_LED_PIN 25
#else
#define BOARD_LED_PIN PICO_DEFAULT_LED_PIN
#endif
#endif

#ifndef INPUT_MACRO_BOARD_LED_ENABLED
#define INPUT_MACRO_BOARD_LED_ENABLED 0
#endif

#ifndef INPUT_MACRO_PIN
#define INPUT_MACRO_PIN -1
#endif

#define MAX_MACRO_INPUT_LIMIT 30
#define MAX_MACRO_LIMIT 6
#define INPUT_HOLD_US 16666

// Hold FN this long to start or finish a recording.
#define MACRO_RECORD_HOLD_US 5000000ULL
#define MACRO_RECORD_FRAME_US 16666U
#define MACRO_RECORD_FLASH_MS 70

// Input Macro Module Name
#define InputMacroName "Input Macro"

// True while the onboard LED is blinking a record start/save pattern.
bool inputMacroLedOverride();

class InputMacro : public GPAddon {
public:
    virtual bool available();   // GPAddon available
    virtual void setup();       // Analog Setup
    virtual void process() {};     // Analog Process
    virtual void preprocess();
    virtual void postprocess(bool sent) {}
    virtual void reinit();
    virtual std::string name() { return InputMacroName; }
private:
    void checkMacroPress();
    void checkMacroAction();
    void runCurrentMacro();
    void reset();
    void restart(Macro& macro);
    void updateRecorder();
    void sampleRecording(uint64_t now);
    void beginRecordFlash(int flashes, bool saving);
    void tickRecordFlash();
    void commitRecording();
    void sealOpenStep(uint64_t now);
    uint32_t captureRecordMask();
    bool recordingHasContent() const;
    uint32_t nextRecordSequence() const;
    bool isMacroRunning;
    bool isMacroTriggerHeld;
    int macroPosition;
    uint32_t macroButtonMask;
    uint32_t macroPinMasks[6];
    uint64_t macroStartTime;
    uint64_t currentMicros;
    int pressedMacro;
    int macroInputPosition;
    uint32_t macroInputHoldTime;
    bool prevMacroInputPressed;
    bool boardLedEnabled;
    MacroOptions * inputMacroOptions;

    enum RecordPhase {
        RECORD_IDLE = 0,
        RECORD_FLASHING,
        RECORD_ACTIVE,
    };
    RecordPhase recordPhase;
    bool recordFlashSaves;
    bool fnWasDown;
    bool fnSeenReleased;
    uint64_t fnDownSince;
    int recordFlashesLeft;
    bool recordLedOn;
    uint32_t recordLedNextMs;
    uint32_t recordMask;
    uint64_t recordStepStart;
    int recordStepCount;
    uint32_t recordStepMask[MAX_MACRO_INPUT_LIMIT];
    uint32_t recordStepDuration[MAX_MACRO_INPUT_LIMIT];
    uint32_t recordStepWait[MAX_MACRO_INPUT_LIMIT];
};

#endif  // _InputMacro_H_