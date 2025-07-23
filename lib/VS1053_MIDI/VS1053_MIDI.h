/**
 * VS1053_MIDI.h - Easy-to-use VS1053 MIDI library for Arduino
 * 
 * This library provides simple MIDI control for generic VS1053 audio modules.
 * Developed through extensive reverse engineering and testing.
 * 
 * Key Discoveries:
 * - VS1053 requires specific MIDI packet format with padding bytes
 * - Pin separation: XCS (commands) vs XDCS (data) is critical
 * - MIDI plugin loading enables full instrument control
 * - Standard MIDI parameter order (not swapped) with proper packet format
 * 
 * Hardware Requirements:
 * - Arduino-compatible microcontroller
 * - Generic VS1053 module with separate XCS and XDCS pins
 * - SPI connection (MISO, MOSI, SCK)
 * 
 * Pin Connections:
 * - MISO: Pin 12 (or custom)
 * - MOSI: Pin 11 (or custom) 
 * - SCK:  Pin 13 (or custom)
 * - XCS:  Command interface (register access)
 * - XDCS: Data interface (MIDI data)
 * - DREQ: Data request pin
 * - XRST: Reset pin
 * 
 * Author: Developed through collaborative debugging
 * License: MIT
 * Version: 1.0.0
 */

#ifndef VS1053_MIDI_H
#define VS1053_MIDI_H

#include <Arduino.h>
#include <SPI.h>

class VS1053_MIDI {
public:
    /**
     * Constructor
     * @param xcs_pin Command interface pin (XCS)
     * @param xdcs_pin Data interface pin (XDCS) 
     * @param dreq_pin Data request pin (DREQ)
     * @param reset_pin Reset pin (XRST)
     */
    VS1053_MIDI(uint8_t xcs_pin, uint8_t xdcs_pin, uint8_t dreq_pin, uint8_t reset_pin);
    
    /**
     * Initialize the VS1053 for MIDI operation
     * @param load_plugin Whether to load the MIDI plugin (recommended: true)
     * @return true if initialization successful
     */
    bool begin(bool load_plugin = true);
    
    /**
     * Play a MIDI note
     * @param channel MIDI channel (0-15)
     * @param note MIDI note number (0-127)
     * @param velocity Note velocity (0-127, 0 = note off)
     */
    void noteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    
    /**
     * Stop a MIDI note
     * @param channel MIDI channel (0-15)
     * @param note MIDI note number (0-127)
     * @param velocity Release velocity (usually same as note on)
     */
    void noteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    
    /**
     * Change instrument on a channel
     * @param channel MIDI channel (0-15)
     * @param instrument General MIDI instrument number (0-127)
     */
    void setInstrument(uint8_t channel, uint8_t instrument);
    
    /**
     * Set channel volume
     * @param channel MIDI channel (0-15)
     * @param volume Volume level (0-127)
     */
    void setVolume(uint8_t channel, uint8_t volume);
    
    /**
     * Send raw MIDI command (advanced users)
     * @param cmd MIDI command byte
     * @param data1 First data byte
     * @param data2 Second data byte (0 for single-byte commands)
     */
    void sendMIDI(uint8_t cmd, uint8_t data1, uint8_t data2 = 0);
    
    /**
     * All notes off on a channel
     * @param channel MIDI channel (0-15)
     */
    void allNotesOff(uint8_t channel);
    
    /**
     * All notes off on all channels
     */
    void allNotesOff();
    
    /**
     * Set master volume (hardware volume control)
     * @param left_vol Left channel volume (0-254, lower = louder)
     * @param right_vol Right channel volume (0-254, lower = louder)
     */
    void setMasterVolume(uint8_t left_vol, uint8_t right_vol);
    
    /**
     * Read VS1053 register (for advanced diagnostics)
     * @param address Register address
     * @return Register value
     */
    uint16_t readRegister(uint8_t address);
    
    /**
     * Check if VS1053 is ready for data
     * @return true if ready
     */
    bool isReady();
    
    /**
     * Get the size of the loaded MIDI plugin
     * @return Number of plugin words
     */
    int getPluginSize() { return MIDI_PLUGIN_SIZE; }

private:
    // Pin assignments
    uint8_t _xcs_pin;   // Command interface
    uint8_t _xdcs_pin;  // Data interface  
    uint8_t _dreq_pin;  // Data request
    uint8_t _reset_pin; // Reset
    
    // Internal methods
    void writeRegister(uint8_t address, uint16_t value);
    void waitForDREQ();
    void sendMIDIPacket(uint8_t cmd, uint8_t data1, uint8_t data2, bool has_data2);
    bool loadMIDIPlugin();
    void hardReset();
    
    // MIDI plugin data
    static const unsigned short MIDI_PLUGIN[] PROGMEM;
    static const int MIDI_PLUGIN_SIZE;
};

// General MIDI Instrument Constants
namespace GM_Instruments {
    // Chromatic Percussion
    const uint8_t ACOUSTIC_GRAND_PIANO = 0;
    const uint8_t BRIGHT_ACOUSTIC_PIANO = 1;
    const uint8_t ELECTRIC_GRAND_PIANO = 2;
    const uint8_t HONKY_TONK_PIANO = 3;
    const uint8_t ELECTRIC_PIANO_1 = 4;
    const uint8_t ELECTRIC_PIANO_2 = 5;
    const uint8_t HARPSICHORD = 6;
    const uint8_t CLAVINET = 7;
    const uint8_t CELESTA = 8;
    const uint8_t GLOCKENSPIEL = 9;
    const uint8_t MUSIC_BOX = 10;
    const uint8_t VIBRAPHONE = 11;
    const uint8_t MARIMBA = 12;
    const uint8_t XYLOPHONE = 13;
    const uint8_t TUBULAR_BELLS = 14;
    const uint8_t DULCIMER = 15;
    
    // Organ
    const uint8_t DRAWBAR_ORGAN = 16;
    const uint8_t PERCUSSIVE_ORGAN = 17;
    const uint8_t ROCK_ORGAN = 18;
    const uint8_t CHURCH_ORGAN = 19;
    const uint8_t REED_ORGAN = 20;
    const uint8_t ACCORDION = 21;
    const uint8_t HARMONICA = 22;
    const uint8_t TANGO_ACCORDION = 23;
    
    // Guitar
    const uint8_t ACOUSTIC_GUITAR_NYLON = 24;
    const uint8_t ACOUSTIC_GUITAR_STEEL = 25;
    const uint8_t ELECTRIC_GUITAR_JAZZ = 26;
    const uint8_t ELECTRIC_GUITAR_CLEAN = 27;
    const uint8_t ELECTRIC_GUITAR_MUTED = 28;
    const uint8_t OVERDRIVEN_GUITAR = 29;
    const uint8_t DISTORTION_GUITAR = 30;
    const uint8_t GUITAR_HARMONICS = 31;
    
    // Strings
    const uint8_t VIOLIN = 40;
    const uint8_t VIOLA = 41;
    const uint8_t CELLO = 42;
    const uint8_t CONTRABASS = 43;
    const uint8_t TREMOLO_STRINGS = 44;
    const uint8_t PIZZICATO_STRINGS = 45;
    const uint8_t ORCHESTRAL_HARP = 46;
    const uint8_t TIMPANI = 47;
    
    // Brass
    const uint8_t TRUMPET = 56;
    const uint8_t TROMBONE = 57;
    const uint8_t TUBA = 58;
    const uint8_t MUTED_TRUMPET = 59;
    const uint8_t FRENCH_HORN = 60;
    const uint8_t BRASS_SECTION = 61;
    
    // Reed
    const uint8_t SOPRANO_SAX = 64;
    const uint8_t ALTO_SAX = 65;
    const uint8_t TENOR_SAX = 66;
    const uint8_t BARITONE_SAX = 67;
    const uint8_t OBOE = 68;
    const uint8_t ENGLISH_HORN = 69;
    const uint8_t BASSOON = 70;
    const uint8_t CLARINET = 71;
    
    // Pipe
    const uint8_t PICCOLO = 72;
    const uint8_t FLUTE = 73;
    const uint8_t RECORDER = 74;
    const uint8_t PAN_FLUTE = 75;
    
    // Ethnic
    const uint8_t SITAR = 104;
    const uint8_t BANJO = 105;
    const uint8_t SHAMISEN = 106;
    const uint8_t KOTO = 107;
    
    // Sound Effects  
    const uint8_t RAIN = 96;
    const uint8_t CRYSTAL = 98;
    const uint8_t ATMOSPHERE = 99;
    const uint8_t BRIGHTNESS = 100;
}

#endif // VS1053_MIDI_H
