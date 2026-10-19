# Game Show Buzzer System

A classroom review game buzzer system built with an ATtiny85 microcontroller and large arcade buttons with built-in PL9823 addressable LEDs. First button pressed wins — no more "I said it first!" arguments.

## How It Works

1. **Idle** — Both LEDs glow blue, waiting for a buzz-in
2. **Buzz-in** — First button pressed turns green; the other LED goes dark (locked out)
3. **Countdown** — Winner's LED smoothly fades green → red over 10 seconds
4. **Time's up** — LED flashes red to signal the teacher
5. **Reset** — Press both buttons simultaneously to return to idle (blue)

## Hardware

| Part | Quantity | Notes |
|------|----------|-------|
| ATtiny85-20PU | 1 | DIP-8 package (easy to socket) |
| Large arcade button with PL9823 LED | 2 | PL9823 uses RGB byte order, not GRB |
| 5V power supply | 1 | USB-C PD trigger module works well |
| 100Ω resistor (optional) | 2 | On LED data lines; not strictly required at short cable lengths |
| Custom PCB or perfboard | 1 | See Wiring section |

## Wiring

```
ATtiny85 DIP-8 pinout:

          +-\/-+
RESET PB5 1|    |8  VCC (5V)
      PB3 2|    |7  PB2
      PB4 3|    |6  PB1
      GND 4|    |5  PB0
          +----+

Pin  | ATtiny85 | Connection
-----|----------|--------------------------------------------
 5   | PB0      | Button 1 (other leg to GND)
 6   | PB1      | Button 2 (other leg to GND)
 7   | PB2      | PL9823 #1 DIN (data in)
 2   | PB3      | PL9823 #2 DIN (data in)
 8   | VCC      | 5V
 4   | GND      | GND
```

Buttons use `INPUT_PULLUP` — wire each button between its pin and GND. No external pull-up resistors needed.

**Note:** If your PCB is one ground short, PB4 (pin 3) can be set `OUTPUT LOW` in firmware and used as an additional GND reference for a passive component.

## Software Setup

### Requirements
- [Arduino IDE](https://www.arduino.cc/en/software)
- [damellis ATtiny board support](https://github.com/damellis/attiny) — add this URL in Arduino IDE → Preferences → Additional Board Manager URLs:
  ```
  https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
  ```
- [Adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) — install via Library Manager

### Arduino IDE Settings

| Setting | Value |
|---------|-------|
| Board | ATtiny25/45/85 |
| Processor | ATtiny85 |
| Clock | Internal 8 MHz |
| Programmer | USBtinyISP |

> **Important:** Burn the bootloader first (Tools → Burn Bootloader) to set the 8 MHz fuse. Only needs to be done once per chip.

### Uploading

Use **Sketch → Upload Using Programmer** (not the regular Upload button).

## Programmer Setup (SparkFun Tiny AVR Programmer on Linux)

The SparkFun Tiny AVR Programmer uses `USBtinyISP`. On Linux you need a udev rule for permission:

1. Create `/etc/udev/rules.d/99-usbtiny.rules`:
   ```
   SUBSYSTEM=="usb", ATTR{idVendor}=="1781", ATTR{idProduct}=="0c9f", GROUP="plugdev", MODE="0666"
   ```

2. Add yourself to the plugdev group:
   ```bash
   sudo usermod -a -G plugdev $USER
   ```

3. Log out and back in, then reload udev rules:
   ```bash
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```

4. Unplug and replug the programmer.

## Key Implementation Notes

- **PL9823 byte order:** These LEDs use `NEO_RGB`, not the standard NeoPixel `NEO_GRB`. Without this, green and red would appear swapped during the fade.
- **Two separate NeoPixel strips:** Each LED gets its own `Adafruit_NeoPixel` instance on its own pin (not chained). This avoids timing issues on the ATtiny85 and makes the code straightforward.
- **State machine:** Three states — `IDLE`, `COUNTDOWN`, `FLASHING` — keep the logic clean and non-blocking.
- **Reset debounce:** After both buttons are confirmed held, the code waits for both to be released before returning to IDLE to prevent accidental immediate re-trigger.
- **Flash size:** 4328 bytes (52% of 8192) / RAM: 74 bytes (14% of 512) — plenty of headroom.

## Files

```
src/
  buzzer_system.ino   — ATtiny85 production sketch (what goes on the chip)
README.md
.gitignore
```

## License

MIT — do whatever you want with it.
