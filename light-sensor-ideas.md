# Light Sensor Enhancement Ideas

## Current Usage (BH1750)

- Read every 30 seconds into `SensorData.lightLevel` (lux)
- Controls HT16K33 display brightness in 5 discrete steps (`adjustBrightnessForAmbientLight()`)
  - <10 lux → brightness 2 (very dim)
  - <50 lux → brightness 4 (dim)
  - <200 lux → brightness 8 (medium)
  - <1000 lux → brightness 12 (bright)
  - 1000+ lux → brightness 15 (very bright)
- Lux value shown as one of the five rotating display items
- Hybrid clock NeoPixel LEDs are **not** affected — fixed at `CLOCK_BRIGHTNESS 63`

---

## Ideas

### 1. Drive Hybrid Clock LED Brightness (low effort, high impact)
The NeoPixel clock rings currently use a fixed brightness constant regardless of ambient light.
Feed `lightLevel` into `hybridClock.getDisplay().setBrightness()` on each sensor read using the
same (or similar) lux thresholds already used for the HT16K33. The existing quiet-hours percentage
could serve as a brightness *ceiling* during 10 PM–8 AM so both systems stay in sync.

### 2. Smooth Continuous Brightness Curve (refinement)
Replace the current 5-step stepped brightness in `adjustBrightnessForAmbientLight()` with a
logarithmic interpolation. Human brightness perception is logarithmic, so the current discrete
jumps when lux crosses a threshold are noticeable. A smooth curve would make both the HT16K33
and the clock LEDs track ambient light more naturally. Would apply to both Idea 1 and the
existing HT16K33 control.

### 3. Context-Aware Quiet Hours (light + time)
Quiet hours are currently pure time-based (10 PM–8 AM, 50%). The light sensor could act as
a confirming or overriding signal:
- If it's quiet hours but the room is brightly lit (someone is up) → maintain normal brightness
- If it's daytime but lux is very low (heavy curtains, overcast) → reduce brightness anyway
This makes the dimming behaviour feel more like the room is "aware" rather than just clock-driven.

### 4. Dark Room Chime Suppression (low effort)
If lux drops below a threshold (e.g., <5 lux) for a sustained period, treat it as
"room is dark / everyone is asleep" and suppress chimes entirely — independent of the
time-based quiet window. Useful as a safeguard for irregular schedules. Would require a
short hysteresis timer (e.g., lux must be below threshold for 5+ minutes) to avoid
suppressing a chime just because a light briefly turned off.

### 5. Day/Night Display Mode Switching
Use lux as a secondary signal (alongside time) to switch display content:
- Low lux → show only time and temperature (minimal, easy to read in the dark)
- Normal lux → show the full five-item rolling display rotation
Could replace or supplement the existing `MODE_ROLLING_CURRENT` vs other display modes.

### 6. Lux Trend Logging (DataLogger integration)
Track lux over time in `DataLogger` alongside temperature and pressure. A sustained lux
rise through the morning vs. a flat/low reading could help infer "clear day" vs. "overcast"
and optionally reflect that in a weather summary display item. Would require adding a
`lightLevel` field to `HourlyRecord` and updating `logHourlyData()`.

---

## Implementation Priority Suggestion

| # | Idea | Effort | Impact |
|---|------|--------|--------|
| 1 | Clock LED brightness from lux | Low | High |
| 2 | Smooth brightness curve | Low | Medium |
| 4 | Dark room chime suppression | Low | Medium |
| 3 | Context-aware quiet hours | Medium | Medium |
| 5 | Day/night display switching | Medium | Low–Medium |
| 6 | Lux trend logging | Medium | Low |