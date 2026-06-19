# Two-Way RFID Smart Door Lock System

Dual-unit Arduino-based smart door lock with XOR-encrypted RFID authentication, I2C inter-unit communication, servo motor actuation, and interior button override.

---

## System Overview

The system uses two Arduino UNO units communicating over I2C:

```
OUTDOOR UNIT (I2C master)          INDOOR UNIT (I2C slave, address 8)
  ├── MFRC522 RFID reader            ├── Servo motor (door lock)
  ├── Status LEDs                    ├── Buzzer (audio feedback)
  ├── Buzzer                         ├── Interior override button (pin A1)
  └── XOR encryption of card UID     └── Authorization logic (checks master UIDs)
          │
          └──── I2C (Wire.h) ────────────────────────────────►
                Encrypted 4-byte card UID transmitted to indoor unit
```

The separation of reader (outdoor) from controller (indoor) means the authorisation logic and door mechanism are on the interior side — tampering with the outdoor unit cannot directly actuate the lock.

---

## Authentication Flow

```
1. User presents RFID card to outdoor reader
2. Outdoor unit reads 4-byte card UID
3. XOR encryption: each UID byte XOR'd with key=77 (0x4D)
4. Encrypted bytes transmitted over I2C to indoor unit
5. Indoor unit decrypts (XOR is self-inverse: decrypt = re-XOR with same key)
6. Decrypted UID compared against stored master tags:
     MasterTag  = "8D8C4CD3"
     MasterTag1 = "9CAE54A1"
7. If match → servo rotates to unlock position + buzzer confirmation
8. Door auto-relocks after unlock duration
9. No match → buzzer error tone
```

### XOR Encryption Detail

XOR with a fixed key is a lightweight symmetric cipher suitable for constrained microcontrollers:
- Key: `77` (decimal) = `0x4D`
- Each byte: `encrypted[i] = UID[i] XOR 77`
- Decryption: `UID[i] = encrypted[i] XOR 77` (identical operation)
- Prevents raw UID sniffing from the I2C bus from directly revealing the authorised card values

---

## Interior Override

The indoor unit supports manual unlocking from inside via a button on pin `A1`:
```cpp
int a = analogRead(button);  // button = A1
if (a > 900)
    door_open();
```
Analog read with threshold 900 debounces the button without external RC circuit.

---

## Hardware Components

| Component | Role |
|---|---|
| Arduino UNO (×2) | Outdoor master + indoor slave microcontrollers |
| MFRC522 RFID module | 13.56 MHz contactless card reader (outdoor) |
| Servo motor (pin 9) | Physical door lock actuation (indoor) |
| Buzzer (pin 7) | Audio feedback for grant/deny/events |
| LED (pin 2) | Visual status indicator |
| Push button (A1) | Interior manual override |
| I2C bus (SDA/SCL) | Communication between outdoor and indoor units |

---

## File Structure

| File | Contents |
|---|---|
| `indoorunit.ino` | Indoor Arduino: I2C slave, XOR decryption, servo control, button override |
| `outdoorunit.ino` | Outdoor Arduino: RFID read, XOR encryption, I2C master transmission |
| `indoorunit.asm` | Assembly-level version of indoor unit logic |
| `outdoorunit.asm` | Assembly-level version of outdoor unit logic |
| `final_report.pdf` | Project report with full circuit diagram and component specifications |

---

## Key Technical Concepts

- **I2C master-slave**: `Wire.begin(8)` (slave at address 8) / `Wire.beginTransmission(8)` (master)
- **XOR cipher**: lightweight symmetric encryption on 4-byte RFID UID without library overhead
- **Servo control**: `doorlock.write(angle)` to lock/unlock via angular position
- **Dual UID authorisation**: two master cards supported; extensible by adding more UID strings
- **Assembly versions**: `.asm` files demonstrate the same logic at register level — useful for understanding low-level AVR architecture

---

## Setup

1. Flash `outdoorunit.ino` to the outdoor Arduino
2. Flash `indoorunit.ino` to the indoor Arduino
3. Connect I2C lines (SDA pin 20 / SCL pin 21 on Mega, or A4/A5 on UNO)
4. Wire MFRC522 to outdoor unit via SPI (SS=10, MOSI=11, MISO=12, SCK=13)
5. Connect servo to indoor unit pin 9; buzzer to pin 7; button to A1
6. Power both units; scan an RFID card to read its UID, then update `MasterTag` values

See `final_report.pdf` for complete circuit schematic.

---

## References

- MFRC522 Arduino Library: https://github.com/miguelbalboa/rfid
- Arduino Wire (I2C) Library: https://www.arduino.cc/en/Reference/Wire
