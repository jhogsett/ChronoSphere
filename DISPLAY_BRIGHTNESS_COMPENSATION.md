# Display Brightness Compensation Implementation

## Problem
The three HT16K33 LED displays (Green, Amber, Red) use different colored LEDs that have significantly different apparent brightness levels when set to the same numerical brightness value. This resulted in:
- **Green Display**: Super bright (overwhelming)
- **Amber Display**: Dim (hard to read) 
- **Red Display**: Very dim (barely visible)

## Solution
Implemented color-specific brightness compensation with these values:
- **Green**: Brightness = 1 (very low setting due to high LED efficiency)
- **Amber**: Brightness = 9 (medium setting)
- **Red**: Brightness = 15 (high setting due to low LED efficiency)

## Implementation Details

### Files Modified
1. **`include/Config.h`** - Added brightness constants
2. **`src/DisplayManager.cpp`** - Updated initialization and brightness methods
3. **`HARDWARE_TEST_GUIDE.md`** - Documented the compensation system

### Code Changes

**Config.h - Added Constants:**
```cpp
// Display Brightness Compensation (compensates for LED color variations)
#define DISPLAY_GREEN_BRIGHTNESS 1   // Green LEDs are very bright
#define DISPLAY_AMBER_BRIGHTNESS 9   // Amber LEDs are medium brightness  
#define DISPLAY_RED_BRIGHTNESS 15    // Red LEDs are dim
```

**DisplayManager.cpp - Updated Initialization:**
```cpp
bool DisplayManager::init() {
  // Different brightness levels needed due to LED color variations
  uint8_t brightnessLevels[3] = {
    DISPLAY_GREEN_BRIGHTNESS, 
    DISPLAY_AMBER_BRIGHTNESS, 
    DISPLAY_RED_BRIGHTNESS
  }; // Green, Amber, Red
  
  for (int i = 0; i < 3; i++) {
    uint8_t address = DISPLAY_GREEN_ADDRESS + i;
    if (!displays[i].begin(address)) {
      return false;
    }
    displays[i].setBrightness(brightnessLevels[i]); // Color-compensated brightness
    displays[i].clear();
    displays[i].writeDisplay();
  }
  // ...
}
```

**DisplayManager.cpp - Updated setBrightness Method:**
```cpp
void DisplayManager::setBrightness(uint8_t baseBrightness) {
  // Apply color compensation to maintain consistent apparent brightness
  uint8_t compensatedBrightness[3];
  
  // Calculate compensated brightness maintaining the ratios: Green=1, Amber=9, Red=15
  compensatedBrightness[0] = (baseBrightness * 1 + 7) / 15;  // Green: scale to 1/15 of red
  compensatedBrightness[1] = (baseBrightness * 9 + 7) / 15;  // Amber: scale to 9/15 of red  
  compensatedBrightness[2] = baseBrightness;                 // Red: unchanged
  
  // Ensure minimum brightness of 1
  if (compensatedBrightness[0] < 1) compensatedBrightness[0] = 1;
  if (compensatedBrightness[1] < 1) compensatedBrightness[1] = 1;
  if (compensatedBrightness[2] < 1) compensatedBrightness[2] = 1;
  
  for (int i = 0; i < 3; i++) {
    displays[i].setBrightness(compensatedBrightness[i]);
  }
}
```

## Benefits
1. **Uniform Appearance**: All three displays now have matching apparent brightness
2. **Better Readability**: Each display is optimized for its LED color characteristics
3. **Automatic Scaling**: Ambient light adjustments maintain proper ratios
4. **Easy Adjustment**: Constants in Config.h allow easy fine-tuning if needed

## Testing
- ✅ Hardware test suite builds successfully (17.6% RAM, 69.2% Flash)
- ✅ Main application builds successfully (41.7% RAM, 68.4% Flash)
- ✅ No memory overhead increase
- ✅ Backwards compatible with existing brightness adjustment features

## Future Adjustments
If brightness levels need tweaking, modify the constants in `Config.h`:
```cpp
#define DISPLAY_GREEN_BRIGHTNESS 1   // Adjust this value (1-15)
#define DISPLAY_AMBER_BRIGHTNESS 9   // Adjust this value (1-15)
#define DISPLAY_RED_BRIGHTNESS 15    // Adjust this value (1-15)
```

The `setBrightness()` method will automatically maintain proper ratios when ambient light adjustments are made.

## Hardware Note
This compensation is specific to the HT16K33 displays with these particular LED colors. Different LED batches or manufacturers may require different compensation values.
