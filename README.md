# arcade-box
Arcade Button as USB Mouse Click with Pro Micro (ATmega32U4)
Great choice of board for this! The Pro Micro with the ATmega32U4 is one of the best options for USB HID (Human Interface Device) projects because it has native USB support — meaning it can appear as a mouse, keyboard, or gamepad directly to your computer without any extra hardware or drivers.
The Approach
We'll use the built-in Mouse library that comes with the Arduino IDE. This library works specifically with 32U4-based boards (like your Pro Micro, Leonardo, etc.) and lets the board act as a USB mouse.
Wiring
The circuit is dead simple:

One leg of the arcade button → a digital pin (let's use pin 2)
Other leg → GND

That's it. We'll use the ATmega32U4's internal pull-up resistor, so no external resistor is needed. When the button is open, the pin reads HIGH. When pressed, it connects to GND and reads LOW.
