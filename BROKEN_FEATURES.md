# Broken Features Needing Revisiting

## LED Weather Alert Flashing System

### Status: BROKEN - Non-functional after multiple attempts

### Problem Summary
The LED weather alert system fails to flash properly. Instead of flashing 3 times (on/off/on/off/on/off) followed by sustained alert color, the LEDs just turn on solid and stay on.

### What DOES Work
1. **Hardware is confirmed working**: Direct blocking flash test with delay() calls works perfectly
2. **Alert triggering works**: Weather alerts trigger correctly based on temperature/pressure conditions
3. **LED basic control works**: `setPixelColor()`, `show()`, `clear()` all function properly
4. **Serial debug output works**: Can see alert start messages in serial monitor

### What DOESN'T Work
1. **Non-blocking state machine flashing**: State machine approach fails to produce visible flashing
2. **State transitions**: Debug output suggests state machine may not be transitioning properly
3. **Integration with main loop**: Normal lighting updates may be interfering with alert flashing

### Technical Details

#### Current Implementation Architecture
- **File**: `src/LightingEffects.cpp`
- **State Machine**: `ALERT_STATE_IDLE` → `ALERT_STATE_FLASH_ON` → `ALERT_STATE_FLASH_OFF` → repeat → `ALERT_STATE_SUSTAINED`
- **Timing**: 200ms flash on, 200ms flash off, repeat 3 times, then 10 minutes sustained
- **Integration**: `updateAlert()` called every frame from `update()` function

#### Known Working Code (Blocking Version)
```cpp
// THIS WORKS - Direct blocking flash (for reference)
for (int flash = 0; flash < flashes; flash++) {
    // Flash ON
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, colorToUint32(color));
    }
    show();
    delay(200);  // ON for 200ms
    
    // Flash OFF
    clear();  
    show();
    delay(200);  // OFF for 200ms
}
```

#### Current Non-Working Code (State Machine)
- State machine logic appears correct in theory
- Debug output suggests states may not be transitioning
- Normal lighting updates completely disabled during alerts
- `updateAlert()` called every frame for precise timing

### Debugging History
Multiple approaches attempted over several hours:

1. **Initial State Machine**: Complex timing with constants - didn't work
2. **Timing Constant Issues**: `ALERT_FLASH_DURATION` was undefined, causing 30-second delays
3. **Fixed Constants**: Hardcoded 200ms timing - still didn't work  
4. **Frame Rate Issues**: Moved `updateAlert()` outside 30fps limit - still didn't work
5. **Normal Lighting Interference**: Completely disabled normal lighting during alerts - still didn't work
6. **Debug Output Added**: Can see alert starts but no visible flashing occurs

### Memory Constraints
- Arduino Nano Every at 98.3% flash usage (47,835/48,640 bytes)
- Very limited space for debug output and additional code
- Had to remove verbose debugging to fit in flash memory

### Suspected Root Causes
1. **State Machine Logic Error**: Something preventing proper state transitions
2. **Timing Issues**: Millisecond timing not working as expected in non-blocking context
3. **LED Driver Issues**: Some interference with NeoPixel library in non-blocking mode
4. **Memory Corruption**: Possible memory issues due to tight constraints
5. **Integration Issues**: Some other part of the system interfering with LED updates

### Files Modified
- `src/LightingEffects.cpp` - Primary implementation
- `src/LightingEffects.h` - State machine constants and definitions  
- `src/main.cpp` - Alert triggering logic (this part works)
- `include/Config.h` - Alert state enums

### Next Steps for Future Investigation

#### Immediate Debugging
1. **Verify State Transitions**: Add minimal debug to confirm state machine actually transitions between states
2. **Isolate LED Control**: Create standalone state machine test that only controls LEDs, no other features
3. **Memory Analysis**: Check for memory corruption or stack overflow issues
4. **Timing Verification**: Confirm `millis()` timing is actually working as expected

#### Alternative Approaches
1. **Hybrid Approach**: Use blocking delays for flashing, non-blocking for sustained alert
2. **Interrupt-Based**: Use timer interrupts for precise flashing timing
3. **Simplified State Machine**: Reduce complexity, fewer states
4. **External Library**: Consider using a different LED control approach

#### Testing Strategy
1. **Minimal Test Case**: Create smallest possible flashing test
2. **Hardware Isolation**: Test LED flashing completely separate from weather system
3. **Progressive Complexity**: Start simple and add complexity only when working
4. **Oscilloscope Analysis**: If available, monitor the actual LED signal

### Important Notes
- **DO NOT** spend more than 1-2 hours on initial retry
- **MUST** verify basic state machine transitions with debug output first
- **REMEMBER** hardware works fine - this is a software/timing issue
- **CONSIDER** this may be a fundamental Arduino/timing limitation requiring different approach

### Code Backup
Current working temperature gradient and alert triggering code is functional. Only the flashing mechanism itself is broken.

---

**Time Invested**: ~3-4 hours  
**Frustration Level**: HIGH  
**Priority**: Medium (nice-to-have feature)  
**Estimated Fix Time**: 2-4 hours with fresh perspective  

**Last Modified**: December 2024
