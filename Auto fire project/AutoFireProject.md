# Arcade Button USB Mouse Click — Auto-Fire Edition

**Board:** Pro Micro clone (ATmega32U4 / Binghe 3)

A single arcade button that acts as a USB mouse left-click, with a hold-to-activate auto-fire mode, a potentiometer to control the click speed, and an LED to indicate auto-fire state.

---

## Features

- **Single press** — sends one left mouse click
- **Hold for 3 seconds** — activates auto-fire mode (continues clicking after you release)
- **Press again** — deactivates auto-fire immediately on press
- **Potentiometer** — adjusts auto-fire speed in real time
- **LED indicator** — solid on during auto-fire, rapid blink on activation, off when idle
- **Boot safety** — hold button during power-on to disable mouse (prevents lockout)

---

## Hardware Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| Pro Micro (ATmega32U4) | 1 | 5V/16MHz or 3.3V/8MHz clone |
| Arcade button with microswitch | 1 | Sanwa, Cherry, or similar |
| 10kΩ potentiometer | 1 | Linear taper (B10K) recommended |
| LED (any colour) | 1 | 3mm or 5mm standard LED |
| 220Ω resistor | 1 | Current limiter for the LED |
| Hookup wire | — | — |

---

## Wiring

### Arcade Button

| Button Terminal | Connects To |
|-----------------|-------------|
| Terminal 1 | Pro Micro **Pin 2** |
| Terminal 2 | Pro Micro **GND** |

No external pull-up resistor is needed. The code enables the ATmega32U4's internal pull-up (~20–50kΩ).

### LED (Auto-Fire Indicator)

| LED Lead | Connects To |
|----------|-------------|
| Anode (+) long leg | Pro Micro **Pin 3** (through 220Ω resistor) |
| Cathode (−) short leg / flat side | Pro Micro **GND** |

The wiring path is: **Pin 3 → 220Ω resistor → LED anode → LED cathode → GND**.

> **Why 220Ω?** At 5V with a typical LED forward voltage of ~2V, this gives roughly 14mA — bright enough to see clearly, well within the ATmega32U4's 20mA per-pin limit. For a 3.3V board, 100Ω works. For a very bright LED that's blinding, bump up to 330–470Ω.

### Potentiometer

| Pot Pin | Connects To |
|---------|-------------|
| Pin 1 (outer leg) | Pro Micro **GND** |
| Pin 2 (wiper / middle) | Pro Micro **A0** |
| Pin 3 (outer leg) | Pro Micro **VCC** |

> **Note:** If turning the knob clockwise makes it slower when you want it faster, swap the GND and VCC connections on the outer pot legs.

---

## LED Behaviour

| State | LED |
|-------|-----|
| Idle / single click mode | **Off** |
| Auto-fire just activated | **Rapid blink** (3 flashes) then solid on |
| Auto-fire running | **Solid on** |
| Auto-fire deactivated (button press) | **Off immediately** |
| Boot bail-out active | **Off** (mouse disabled entirely) |

The rapid blink on activation gives you tactile + visual confirmation that auto-fire has engaged, so you know it's safe to release the button.

---

## Code Walkthrough

### Pin Definitions

```cpp
const int buttonPin = 2;
const int ledPin    = 3;
const int potPin    = A0;
```

- **Pin 2** — arcade button input with internal pull-up
- **Pin 3** — digital output driving the indicator LED
- **A0** — analog input reading the potentiometer wiper (0–1023)

### Auto-Fire Rate Limits

```cpp
const unsigned long AUTO_FIRE_MIN_INTERVAL = 30;   // Fastest (ms between clicks)
const unsigned long AUTO_FIRE_MAX_INTERVAL = 500;   // Slowest (ms between clicks)
```

These define the potentiometer mapping range:

| Interval | Clicks per Second |
|----------|-------------------|
| 30ms (min) | ~33 clicks/sec |
| 100ms | ~10 clicks/sec |
| 250ms | ~4 clicks/sec |
| 500ms (max) | ~2 clicks/sec |

Adjust these constants to suit your application. For gaming, 30–300ms is a practical range.

### Hold Threshold

```cpp
const unsigned long HOLD_THRESHOLD = 3000;  // 3 seconds
```

The button must be held continuously for this duration to activate auto-fire. Prevents accidental activation during fast presses. Lower to 1500–2000ms if 3 seconds feels too long.

### Debouncing

```cpp
const unsigned long DEBOUNCE_DELAY = 30;
```

Arcade microswitches produce electrical bounce — rapid on/off oscillations for a few milliseconds when the contacts close or open. The debounce logic ignores state changes for 30ms after the first transition, ensuring one clean event is registered. Increase to 40–50ms if you get double-clicks.

### `INPUT_PULLUP`

```cpp
pinMode(buttonPin, INPUT_PULLUP);
```

Activates the chip's internal pull-up resistor (~20–50kΩ), pulling the pin **HIGH** when the button is open. When the button is pressed, it connects the pin to GND, reading **LOW**. This eliminates the need for an external resistor and avoids a "floating" pin that would give erratic readings.

### LED Setup

```cpp
pinMode(ledPin, OUTPUT);
digitalWrite(ledPin, LOW);
```

Configures pin 3 as an output and ensures the LED starts off. The LED is driven directly from the GPIO pin — `HIGH` turns it on, `LOW` turns it off.

### LED Blink Function

```cpp
void blinkLed(int count, unsigned long duration) {
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);
    delay(duration);
  }
}
```

A simple blocking blink used only at the moment of auto-fire activation. It blinks the LED 3 times at 80ms intervals (total ~480ms). This is a deliberate `delay()` — we *want* to block briefly here to give clear visual feedback before auto-fire clicks start. The slight pause also prevents an accidental click on the exact frame auto-fire starts.

### Safety Bail-Out

```cpp
delay(500);
if (digitalRead(buttonPin) == LOW) {
  mouseEnabled = false;
  return;
}
Mouse.begin();
```

Once `Mouse.begin()` runs, the Pro Micro appears as a USB mouse to your computer. If the code misbehaves and sends uncontrolled clicks, reprogramming becomes very difficult.

The bail-out checks the button 500ms after boot. If held, mouse functionality is **disabled entirely** and the LED stays off as a visual indicator that bail-out is active.

**To use:** hold the arcade button → plug in the USB cable → keep holding for at least 1 second → release. The board will enumerate as USB but send no mouse events.

### State Machine — Button Press Logic

```cpp
// Button just pressed (HIGH → LOW)
if (previousStable == HIGH && stableState == LOW) {
  pressStartTime = millis();
  buttonIsHeld = true;
  holdTriggeredAuto = false;

  if (autoFireActive) {
    autoFireActive = false;
    digitalWrite(ledPin, LOW);
    return;
  }
}
```

On every press, the code first checks: **is auto-fire already running?** If yes, it stops immediately and turns off the LED. The `return` skips all further processing for this cycle — the press is consumed entirely by the deactivation, producing no click.

If auto-fire isn't active, the press starts the hold timer.

```cpp
// Button just released (LOW → HIGH)
if (previousStable == LOW && stableState == HIGH) {
  buttonIsHeld = false;

  if (!holdTriggeredAuto && !autoFireActive) {
    Mouse.click(MOUSE_LEFT);
  }
}
```

On release, a single click is sent **only if** this wasn't a hold that activated auto-fire. The `holdTriggeredAuto` flag prevents the release from also sending a click after a 3-second hold.

### State Variables Summary

| Variable | Purpose |
|----------|---------|
| `stableState` | Debounced confirmed button state (HIGH = open, LOW = pressed) |
| `buttonIsHeld` | Whether the button is currently held down |
| `holdTriggeredAuto` | Whether this hold already activated auto-fire (prevents click on release) |
| `autoFireActive` | Whether auto-fire mode is currently running |
| `mouseEnabled` | Safety flag — false if bail-out was triggered on boot |

### Hold Detection → Auto-Fire Activation

```cpp
if (buttonIsHeld && !holdTriggeredAuto && !autoFireActive) {
  if ((millis() - pressStartTime) >= HOLD_THRESHOLD) {
    autoFireActive = true;
    holdTriggeredAuto = true;
    lastAutoFireClick = millis();

    blinkLed(BLINK_COUNT, BLINK_DURATION);
    digitalWrite(ledPin, HIGH);
  }
}
```

Checked every loop iteration while the button is held. Once the hold crosses 3 seconds:

1. Activates auto-fire
2. Sets the hold flag so release won't also click
3. Blinks the LED 3 times for confirmation
4. Leaves the LED solid on

### Auto-Fire Execution with Potentiometer

```cpp
if (autoFireActive) {
  int potValue = analogRead(potPin);
  unsigned long clickInterval = map(
    potValue, 0, 1023,
    AUTO_FIRE_MIN_INTERVAL, AUTO_FIRE_MAX_INTERVAL
  );

  if ((millis() - lastAutoFireClick) >= clickInterval) {
    Mouse.click(MOUSE_LEFT);
    lastAutoFireClick = millis();
  }
}
```

While auto-fire is active, the potentiometer is read on **every loop iteration**, so speed adjustments happen in real time. `analogRead()` returns 0–1023 (10-bit ADC), which `map()` scales linearly to the configured interval range.

`Mouse.click(MOUSE_LEFT)` sends a complete press + release via the ATmega32U4's native USB HID. The computer sees it as a real mouse click — no drivers needed.

---

## Complete State Flow

```
IDLE (LED off)
  │
  ├─ Short press + release → Single mouse click → back to IDLE
  │
  └─ Hold for 3 seconds → LED blinks 3x → LED solid on
        │
        AUTO-FIRE ACTIVE (LED on)
        │  Clicking at potentiometer-controlled rate
        │  Pot adjustable in real time
        │
        └─ Press button → LED off → back to IDLE
```

---

## Arduino IDE Setup

1. **Board Manager** — if "SparkFun Pro Micro" isn't available, add this URL to **File → Preferences → Additional Board Manager URLs:**
   ```
   https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
   ```
2. **Board selection** — select **SparkFun Pro Micro** and choose the correct processor:
   - `ATmega32U4 (5V, 16MHz)` — most common for clones
   - `ATmega32U4 (3.3V, 8MHz)` — less common
3. **Port** — select the COM port that appears when you plug in the board.
4. **Upload** — click Upload. The board will briefly disconnect and reconnect during programming — this is normal for 32U4 boards.

---

## Customisation Quick Reference

| What to Change | Variable | Default |
|----------------|----------|---------|
| Fastest auto-fire speed | `AUTO_FIRE_MIN_INTERVAL` | 30ms (~33 clicks/sec) |
| Slowest auto-fire speed | `AUTO_FIRE_MAX_INTERVAL` | 500ms (~2 clicks/sec) |
| Hold time to activate | `HOLD_THRESHOLD` | 3000ms (3 seconds) |
| Button pin | `buttonPin` | 2 |
| LED pin | `ledPin` | 3 |
| Potentiometer pin | `potPin` | A0 |
| Debounce time | `DEBOUNCE_DELAY` | 30ms |
| Activation blink count | `BLINK_COUNT` | 3 |
| Activation blink speed | `BLINK_DURATION` | 80ms |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Double-clicking on single press | Increase `DEBOUNCE_DELAY` to 40–50ms |
| Auto-fire activates too easily | Increase `HOLD_THRESHOLD` |
| Pot direction is reversed | Swap the GND and VCC wires on the outer pot legs |
| LED doesn't light up | Check polarity (long leg = anode → resistor → pin 3) |
| LED too dim | Decrease resistor value (try 150Ω, stay above 100Ω for 5V) |
| LED too bright / blinding | Increase resistor value (330–470Ω) |
| Board won't reprogram (clicking constantly) | Use the bail-out: hold button while plugging in USB |
| Board not recognised in IDE | Check you selected the right processor voltage/speed |
| Auto-fire too fast / too slow | Adjust `AUTO_FIRE_MIN_INTERVAL` and `AUTO_FIRE_MAX_INTERVAL` |
