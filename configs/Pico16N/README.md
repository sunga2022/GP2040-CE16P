# GP2040 Configuration for Raspberry Pi Pico16N

![Pin Mapping](assets/PinMapping.png)

Basic pin setup for a stock Raspberry Pi Pico. Combine with a simple GPIO breakout/screw terminal board for an easy DIY arcade stick.

## Main Pin Mapping Configuration

| RP2040 Pin | Action                        | GP2040 | Xinput | Switch | PS3/4/5  | Dinput | Arcade |
|------------|-------------------------------|--------|--------|--------|----------|--------|--------|
| GPIO_PIN_21| GpioAction::BUTTON_PRESS_UP   | UP     | UP     | UP      | UP      | UP     | UP     |
| GPIO_PIN_22| GpioAction::BUTTON_PRESS_DOWN | DOWN   | DOWN   | DOWN    | DOWN    | DOWN   | DOWN   |
| GPIO_PIN_16| GpioAction::BUTTON_PRESS_RIGHT| RIGHT  | RIGHT  | RIGHT   | RIGHT   | RIGHT  | RIGHT  |
| GPIO_PIN_23| GpioAction::BUTTON_PRESS_LEFT | LEFT   | LEFT   | LEFT    | LEFT    | LEFT   | LEFT   |
| GPIO_PIN_18| GpioAction::BUTTON_PRESS_B1   | B1     | A      | B       | Cross   | 2      | K1     |
| GPIO_PIN_13| GpioAction::BUTTON_PRESS_B2   | B2     | B      | A       | Circle  | 3      | K2     |
| GPIO_PIN_14| GpioAction::BUTTON_PRESS_R2   | R2     | RT     | ZR      | R2      | 8      | K3     |
| GPIO_PIN_15| GpioAction::BUTTON_PRESS_L2   | L2     | LT     | ZL      | L2      | 7      | K4     |
| GPIO_PIN_12| GpioAction::BUTTON_PRESS_B3   | B3     | X      | Y       | Square  | 1      | P1     |
| GPIO_PIN_09| GpioAction::BUTTON_PRESS_B4   | B4     | Y      | X       | Triangle| 4      | P2     |
| GPIO_PIN_10| GpioAction::BUTTON_PRESS_R1   | R1     | RB     | R       | R1      | 6      | P3     |
| GPIO_PIN_11| GpioAction::BUTTON_PRESS_L1   | L1     | LB     | L       | L1      | 5      | P4     |
| GPIO_PIN_08| GpioAction::BUTTON_PRESS_S1   | S1     | Back   | Minus   | Select  | 9      | Coin   |
| GPIO_PIN_17| GpioAction::BUTTON_PRESS_S2   | S2     | Start  | Plus    | Start   | 10     | Start  |
| GPIO_PIN_02| GpioAction::BUTTON_PRESS_L3   | L3     | LS     | LS      | L3      | 11     | LS     |
| GPIO_PIN_19| GpioAction::BUTTON_PRESS_R3   | R3     | RS     | RS      | R3      | 12     | RS     |
| GPIO_PIN_07| GpioAction::BUTTON_PRESS_A1   | A1     | Guide  | Home    | PS      | 13     | ~      |
| GPIO_PIN_06| GpioAction::BUTTON_PRESS_A2   | A2     | ~      | Capture | ~       | 14     | ~      |
| GPIO_PIN_29| GpioAction::BUTTON_PRESS_UP    | M1     | UP     | UP      | UP       | UP     | UP     |
| GPIO_PIN_20| GpioAction::BUTTON_PRESS_L3    | M2     | LS     | LS      | L3       | 11     | LS     |

## Defaults

| Setting | Value |
|---------|--------|
| Input mode | XInput |
| Direction | Digital d-pad |
| SOCD | Neutral |
| Turbo | 30 shots/sec |
| USB host peripheral | On (D+ = GPIO 4, D− = GPIO 3), also shown under 功能配置 → 外设映射 |
| PS4 auth | Keys compiled in from `PS4Auth/` (`key.pem`, `serial.txt`, `sig.bin`) |
| PS5 auth | USB passthrough |
| M1 | Macro 1: SF6 Ken facing right |
| M2 | Macro 2: SF6 Ken facing left |

Ken combo: `2MK > Drive Rush (MP+MK) > 2HP > H Jinrai > Kazekama > Senka > SA3`.

## PS4 / PS5 touchpad

Only in PS4 or PS5 input mode. TOUCH is A2 (GP6), SELECT is S1 (GP8).

| Input | What the console sees |
|-------|------------------------|
| Tap TOUCH | Touchpad click |
| Hold TOUCH + d-pad | Finger swipe on the touchpad (d-pad is not sent) |
| Hold TOUCH + SELECT | Touchpad click (SELECT/Share is not sent) |
| Hold TOUCH + d-pad + SELECT | Swipe and click at the current finger position |

Swipe starts from the center of the pad each time you press TOUCH.

To rebuild the compiled-in PS4 key header after replacing the three files:

```bash
python3 configs/Pico16N/generate_ps4_keys_header.py
```

## RGB LEDs

Data pin is GPIO 28, 14 LEDs in GRB order. Press lighting follows the physical chain / GPIO, so remapping a key in Web Config does not move which LED lights. A button with turbo assigned blinks its own RGB at the turbo shot rate while held.

| Chain index | Button | GPIO |
|-------------|--------|------|
| 0 | Left | 23 |
| 1 | Down | 22 |
| 2 | Right | 16 |
| 3 | Up | 21 |
| 4 | B3 | 12 |
| 5 | B4 | 9 |
| 6 | R1 | 10 |
| 7 | L1 | 11 |
| 8 | B1 | 18 |
| 9 | B2 | 13 |
| 10 | R2 | 14 |
| 11 | L2 | 15 |
| 12 | R3 | 19 |
| 13 | L3 | 2 |

Web Config has an RGB menu: 30 pad effects, plus a per-button custom mode. Cycle them with the LED animation hotkeys. The last step is per-button custom. In that mode each lit button has its own idle color, effect (solid, breath, jump, fast flash, slow flash, heartbeat), and separate press color. Preset effects still turn a pressed key white.