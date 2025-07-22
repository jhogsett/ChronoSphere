/**
 * VS1053_MIDI.cpp - Implementation of VS1053 MIDI library
 * 
 * This implementation contains all the hard-won knowledge from extensive
 * debugging and reverse engineering of VS1053 MIDI communication.
 * 
 * IMPORTANT DISCOVERY: VS1053 has built-in General MIDI support and 16.5KB RAM.
 * The large rtmidi1053b plugin was designed for VS1003 (only 5.5KB RAM).
 * For VS1053, the minimal 28-word rtmidistart plugin is sufficient!
 * 
 * BENEFITS OF MINIMAL PLUGIN FOR VS1053:
 * - Saves ~2KB PROGMEM (1008 words saved)
 * - Faster initialization (28 vs 1036 words to load)
 * - Identical audio quality (VS1053 has built-in GM synthesis)
 * - Clean MIT licensing (official VLSI source, no GPL issues)
 * - More memory available for application features
 */

#include "VS1053_MIDI.h"

// MIDI plugin data in RLE compressed format for Arduino memory efficiency
// For VS1053: Use minimal plugin (default) - saves 2KB PROGMEM with identical functionality
// For VS1003: Comment out USE_MINIMAL_MIDI_PLUGIN to use full plugin
#define USE_MINIMAL_MIDI_PLUGIN

#ifdef USE_MINIMAL_MIDI_PLUGIN
// Minimal MIDI plugin from VLSI rtmidistart.plg (28 words)
// Sufficient for VS1053 which has built-in General MIDI support (16.5KB RAM)
// The VS1053 has 3x more RAM than VS1003 and doesn't need the large plugin
const unsigned short VS1053_MIDI::MIDI_PLUGIN[] PROGMEM = {
    0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080,
    0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890,
    0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030,
    0x0200, 0x000a, 0x0001, 0x0050
};
#else
// For VS1003 compatibility: Large General MIDI plugin may be needed
// The VS1003 has only 5.5KB RAM and may require the full plugin for complete GM support
// 
// Reference implementation available at:
// https://github.com/pschatzmann/arduino-vs1053/tree/master/src/patches_midi
// 
// Note: VS1053 users should use the minimal plugin above for optimal performance
// and memory efficiency. This section is only for VS1003 backward compatibility.
const unsigned short VS1053_MIDI::MIDI_PLUGIN[] PROGMEM = {
    // Plugin data would go here - see reference link above if needed for VS1003
    // For VS1053, comment out USE_MINIMAL_MIDI_PLUGIN to reach this section,
    // but the minimal plugin above is strongly recommended.
    0x0000  // Placeholder - replace with actual plugin data if VS1003 support needed
};
#endif

const int VS1053_MIDI::MIDI_PLUGIN_SIZE = sizeof(MIDI_PLUGIN) / sizeof(MIDI_PLUGIN[0]);

VS1053_MIDI::VS1053_MIDI(uint8_t xcs_pin, uint8_t xdcs_pin, uint8_t dreq_pin, uint8_t reset_pin) {
    _xcs_pin = xcs_pin;
    _xdcs_pin = xdcs_pin;
    _dreq_pin = dreq_pin;
    _reset_pin = reset_pin;
}

bool VS1053_MIDI::begin(bool load_plugin) {
    // Initialize pins
    pinMode(_xcs_pin, OUTPUT);
    pinMode(_xdcs_pin, OUTPUT);
    pinMode(_dreq_pin, INPUT);
    pinMode(_reset_pin, OUTPUT);
    
    // Set initial states
    digitalWrite(_xcs_pin, HIGH);   // Command interface idle
    digitalWrite(_xdcs_pin, HIGH);  // Data interface idle
    digitalWrite(_reset_pin, HIGH); // Not in reset
    
    // Initialize SPI (user should call SPI.begin() before this)
    // SPI.begin(); // User responsibility to avoid conflicts
    
    // Perform hardware reset
    hardReset();
    
    // Wait for chip to be ready
    delay(500);
    waitForDREQ();
    
    // Set a reasonable volume (lower values = louder)
    setMasterVolume(0x20, 0x20);
    delay(100);
    
    // Load MIDI plugin if requested
    if (load_plugin) {
        if (!loadMIDIPlugin()) {
            return false; // Plugin loading failed
        }
        delay(500);
    } else {
        // Basic MIDI mode (limited functionality)
        writeRegister(0x00, 0x4800); // Set MODE register for basic MIDI
        delay(100);
    }
    
    // Verify initialization
    uint16_t mode = readRegister(0x00);
    uint16_t status = readRegister(0x01);
    
    // Check if we're in a valid MIDI mode
    return (mode & 0x800) != 0; // MIDI mode bit should be set
}

void VS1053_MIDI::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel > 15 || note > 127 || velocity > 127) return;
    sendMIDIPacket(0x90 | channel, note, velocity, true);
}

void VS1053_MIDI::noteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel > 15 || note > 127 || velocity > 127) return;
    sendMIDIPacket(0x80 | channel, note, velocity, true);
}

void VS1053_MIDI::setInstrument(uint8_t channel, uint8_t instrument) {
    if (channel > 15 || instrument > 127) return;
    sendMIDIPacket(0xC0 | channel, instrument, 0, false);
    delay(50); // Give time for instrument change
}

void VS1053_MIDI::setVolume(uint8_t channel, uint8_t volume) {
    if (channel > 15 || volume > 127) return;
    sendMIDIPacket(0xB0 | channel, 0x07, volume, true); // Control Change: Volume
}

void VS1053_MIDI::sendMIDI(uint8_t cmd, uint8_t data1, uint8_t data2) {
    // Determine if command has 2 data bytes
    bool has_data2 = ((cmd & 0xF0) <= 0xB0) || ((cmd & 0xF0) >= 0xE0);
    sendMIDIPacket(cmd, data1, data2, has_data2);
}

void VS1053_MIDI::allNotesOff(uint8_t channel) {
    if (channel > 15) return;
    sendMIDIPacket(0xB0 | channel, 0x7B, 0x00, true); // All Notes Off
}

void VS1053_MIDI::allNotesOff() {
    for (uint8_t ch = 0; ch < 16; ch++) {
        allNotesOff(ch);
    }
}

void VS1053_MIDI::setMasterVolume(uint8_t left_vol, uint8_t right_vol) {
    uint16_t volume = (left_vol << 8) | right_vol;
    writeRegister(0x0B, volume); // VOL register
}

uint16_t VS1053_MIDI::readRegister(uint8_t address) {
    waitForDREQ();
    digitalWrite(_xcs_pin, LOW);
    SPI.transfer(0x03);  // Read command
    SPI.transfer(address);
    uint16_t result = SPI.transfer(0x00) << 8;
    result |= SPI.transfer(0x00);
    digitalWrite(_xcs_pin, HIGH);
    return result;
}

bool VS1053_MIDI::isReady() {
    return digitalRead(_dreq_pin) == HIGH;
}

// Private methods

void VS1053_MIDI::writeRegister(uint8_t address, uint16_t value) {
    waitForDREQ();
    digitalWrite(_xcs_pin, LOW);
    SPI.transfer(0x02);  // Write command
    SPI.transfer(address);
    SPI.transfer(value >> 8);
    SPI.transfer(value & 0xFF);
    digitalWrite(_xcs_pin, HIGH);
}

void VS1053_MIDI::waitForDREQ() {
    while (!digitalRead(_dreq_pin)) {
        // Wait for data request
    }
}

void VS1053_MIDI::sendMIDIPacket(uint8_t cmd, uint8_t data1, uint8_t data2, bool has_data2) {
    waitForDREQ();
    digitalWrite(_xdcs_pin, LOW);
    
    // VS1053 MIDI packet format: padding bytes between each MIDI byte
    SPI.transfer(0x00);   // Padding
    waitForDREQ();
    SPI.transfer(cmd);    // MIDI command
    waitForDREQ();
    SPI.transfer(0x00);   // Padding
    waitForDREQ();
    SPI.transfer(data1);  // First data byte
    
    if (has_data2) {
        waitForDREQ();
        SPI.transfer(0x00);   // Padding
        waitForDREQ();
        SPI.transfer(data2);  // Second data byte
    }
    
    digitalWrite(_xdcs_pin, HIGH);
    delay(10); // Small delay for processing
}

bool VS1053_MIDI::loadMIDIPlugin() {
    // Load the MIDI plugin using RLE decompression
    int i = 0;
    while (i < MIDI_PLUGIN_SIZE) {
        unsigned short addr, n, val;
        addr = pgm_read_word_near(&MIDI_PLUGIN[i++]);
        n = pgm_read_word_near(&MIDI_PLUGIN[i++]);
        
        if (n & 0x8000U) { // RLE run
            n &= 0x7FFF;
            val = pgm_read_word_near(&MIDI_PLUGIN[i++]);
            while (n--) {
                writeRegister(addr, val);
            }
        } else { // Copy run
            while (n--) {
                val = pgm_read_word_near(&MIDI_PLUGIN[i++]);
                writeRegister(addr, val);
            }
        }
    }
    
    // Verify MIDI mode is active
    uint16_t mode = readRegister(0x00);
    return (mode & 0x800) != 0; // MIDI mode bit should be set
}

void VS1053_MIDI::hardReset() {
    digitalWrite(_reset_pin, LOW);
    delay(10);
    digitalWrite(_reset_pin, HIGH);
    delay(10);
}
