# VS1053_MIDI Library

A comprehensive Arduino library for controlling VS1053/VS1003 audio modules via MIDI over SPI.

## Overview

This library provides full MIDI control capabilities for VS1053/VS1003 audio modules, including General MIDI instrument synthesis, note control, and volume management. It was developed through extensive reverse engineering and testing to provide the complete VS1053 MIDI packet format and proper SPI communication protocols.

## 🚀 Major Discovery: Optimal Plugin Selection

**For VS1053 modules:** This library uses the minimal 28-word MIDI plugin by default, which provides **identical audio quality** to the large 1036-word plugin while saving ~2KB of PROGMEM. The VS1053 has built-in General MIDI support (16.5KB RAM) and doesn't need the large plugin originally designed for VS1003 (5.5KB RAM).

**Benefits of minimal plugin:**
- ✅ **Saves 2KB PROGMEM** (1008 words saved)
- ✅ **Faster initialization** (28 vs 1036 words to load)
- ✅ **Identical audio quality** (VS1053 built-in GM synthesis)
- ✅ **Clean MIT licensing** (official VLSI source)
- ✅ **More memory for your application**

## Features

- ✅ **Full General MIDI Support** - All 128 GM instruments with proper plugin loading
- ✅ **Complete MIDI Control** - Note on/off, instrument changes, volume control
- ✅ **Optimized Memory Usage** - Smart plugin selection based on chip capabilities
- ✅ **Proper VS1053 Protocol** - Correct packet formatting with padding bytes
- ✅ **SPI Interface Management** - Separate command (XCS) and data (XDCS) channel handling
- ✅ **Hardware Abstraction** - Easy pin configuration for different Arduino boards
- ✅ **Real-time Performance** - Optimized for responsive MIDI playback
- ✅ **Comprehensive Examples** - From basic usage to advanced wind chime systems

## Hardware Requirements

- Arduino-compatible microcontroller
- VS1053 or VS1003 audio module (SPI interface)
- Speaker or headphones connected to audio module

### Typical Wiring (Arduino Nano)

| VS1053 Pin | Arduino Pin | Description |
|------------|-------------|-------------|
| MISO       | 12          | SPI Master In, Slave Out |
| MOSI       | 11          | SPI Master Out, Slave In |
| SCK        | 13          | SPI Serial Clock |
| DREQ       | 2           | Data Request (ready signal) |
| XRST       | 8           | Reset (active low) |
| XCS        | 6           | Command Chip Select (active low) |
| XDCS       | 7           | Data Chip Select (active low) |

## Installation

### Arduino Library Manager
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "VS1053_MIDI"
4. Click Install

### Manual Installation
1. Download the library as a ZIP file
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file

## Quick Start

```cpp
#include <SPI.h>
#include <VS1053_MIDI.h>

// Pin definitions
#define VS1053_XCS   6
#define VS1053_XDCS  7
#define VS1053_DREQ  2
#define VS1053_RESET 8

VS1053_MIDI midi(VS1053_XCS, VS1053_XDCS, VS1053_DREQ, VS1053_RESET);

void setup() {
  SPI.begin();
  
  // Initialize with MIDI plugin loading
  if (midi.begin(true)) {
    Serial.println("VS1053 MIDI ready!");
  }
  
  // Set volume (0x00 = loudest, 0xFE = quietest)
  midi.setMasterVolume(0x20, 0x20);
}

void loop() {
  // Play a piano note
  midi.setInstrument(0, GM_ACOUSTIC_GRAND_PIANO);
  midi.noteOn(0, 60, 80);  // Channel 0, Middle C, velocity 80
  delay(500);
  midi.noteOff(0, 60, 64);
  delay(500);
}
```

## API Reference

### Constructor

```cpp
VS1053_MIDI(uint8_t xcs_pin, uint8_t xdcs_pin, uint8_t dreq_pin, uint8_t reset_pin)
```

### Methods

#### `bool begin(bool load_plugin = true)`
Initialize the VS1053 module. Set `load_plugin` to `true` for full General MIDI support.

#### `void noteOn(uint8_t channel, uint8_t note, uint8_t velocity)`
Start playing a note. 
- `channel`: MIDI channel (0-15)
- `note`: MIDI note number (0-127)
- `velocity`: Note velocity (0-127)

#### `void noteOff(uint8_t channel, uint8_t note, uint8_t velocity)`
Stop playing a note.

#### `void setInstrument(uint8_t channel, uint8_t instrument)`
Change the instrument for a MIDI channel.
- `instrument`: General MIDI instrument number (0-127) or use GM constants

#### `void setVolume(uint8_t channel, uint8_t volume)`
Set the volume for a MIDI channel.
- `volume`: Volume level (0-127)

#### `void setMasterVolume(uint8_t left_vol, uint8_t right_vol)`
Set the hardware master volume.
- `left_vol`, `right_vol`: Volume levels (0x00 = loudest, 0xFE = quietest)

#### `void allNotesOff(uint8_t channel)` / `void allNotesOff()`
Stop all playing notes on a channel or all channels.

#### `bool isReady()`
Check if the VS1053 is ready for more data.

## General MIDI Instruments

The library includes constants for all 128 General MIDI instruments:

```cpp
// Piano Family
GM_ACOUSTIC_GRAND_PIANO  // 0
GM_BRIGHT_ACOUSTIC_PIANO // 1
GM_ELECTRIC_GRAND_PIANO  // 2
// ... and many more

// Chromatic Percussion
GM_CELESTA               // 8
GM_GLOCKENSPIEL         // 9
GM_VIBRAPHONE           // 11
GM_MARIMBA              // 12
GM_TUBULAR_BELLS        // 14

// See VS1053_MIDI.h for complete list
```

## Examples

### BasicMIDI
Demonstrates fundamental MIDI operations including instrument changes, scales, chords, and melodies.

### ElectronicWindChimes
Advanced example creating an electronic wind chime system with:
- Multiple instrument scenarios
- Random note generation
- Simulated or sensor-based wind intensity
- Smooth transitions between different chime types

## Technical Details

### VS1053 MIDI Packet Format
The library handles the VS1053-specific MIDI packet format internally:
```
Standard MIDI: [command] [data1] [data2]
VS1053 Format: [0x00] [command] [0x00] [data1] [0x00] [data2]
```

### Pin Usage
- **XCS (Command Chip Select)**: Used for register access and control commands
- **XDCS (Data Chip Select)**: Used exclusively for MIDI data transmission
- **DREQ**: Monitored to ensure the VS1053 is ready for data

### MIDI Plugin
The library includes the compressed MIDI plugin data required for full General MIDI synthesis. This plugin is automatically loaded when `begin(true)` is called.

## Troubleshooting

### No Sound
1. Check all wiring connections
2. Verify speaker/headphones are connected to VS1053 audio output
3. Ensure `begin(true)` was called to load the MIDI plugin
4. Check master volume settings

### Wrong Instruments
1. Ensure MIDI plugin was loaded successfully (`begin(true)`)
2. Verify instrument numbers are valid (0-127)
3. Allow delay after instrument changes

### Poor Performance
1. Use appropriate SPI clock divider: `SPI.setClockDivider(SPI_CLOCK_DIV16)`
2. Ensure DREQ pin is properly connected and monitored
3. Avoid sending MIDI data too rapidly

## Contributing

This library was developed through extensive reverse engineering and testing. If you discover improvements or find bugs, please:

1. Open an issue describing the problem
2. Submit a pull request with fixes
3. Share your VS1053 MIDI discoveries with the community

## License

This library is released under the MIT License. See LICENSE file for details.

## Acknowledgments

- Based on research from the pschatzmann/arduino-vs1053 library
- VS1053 datasheet and application notes from VLSI Solution
- Arduino community contributions and testing

## Version History

### v1.0.0
- Initial release with full General MIDI support
- Complete VS1053 MIDI packet format implementation
- Comprehensive examples and documentation
- Arduino Library Manager compatibility
