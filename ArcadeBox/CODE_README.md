# Arcade Button USB Mouse Click — Pro Micro (ATmega32U4)

## What Each Part Does

### `INPUT_PULLUP`

Activates the chip's internal pull-up resistor (~20–50kΩ), pulling the pin **HIGH** when the button is open. This avoids a "floating" pin that would give erratic readings.

### Debouncing

Arcade buttons (especially microswitches like Cherry or Sanwa) bounce electrically when actuated. The contacts physically vibrate for a few milliseconds, which can register as multiple rapid clicks. The debounce logic ignores state changes for **30ms** after the first one. You can tune this value — if you're getting double clicks, bump it up to **40–50ms**.

### `Mouse.click(MOUSE_LEFT)`

Sends a complete mouse click (press + release) in one call. If you ever need to hold the button (like for drag operations), you'd use `Mouse.press()` and `Mouse.release()` instead.

---

## Practical Notes

### 1. Board Selection in Arduino IDE

Select **"SparkFun Pro Micro"** and make sure you pick the right voltage/speed (`5V/16MHz` or `3.3V/8MHz` depending on your specific clone). If SparkFun doesn't show up, you'll need to add the SparkFun board URL to your Board Manager.

### 2. Safety Tip

Once this sketch is uploaded, the board immediately acts as a mouse. If something goes wrong and it starts clicking uncontrollably, it can be hard to reprogram.

A smart safeguard is to add a **bail-out** — hold the button during boot to skip `Mouse.begin()`:

```cpp
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  delay(500); // give time to hold button
  if (digitalRead(buttonPin) == HIGH) {
    Mouse.begin();  // only start mouse if button is NOT held
  }
}
```

### 3. Multiple Buttons

If you want to expand this to a full arcade controller with several buttons, the same pattern works. You'd just track each button separately or use arrays.
