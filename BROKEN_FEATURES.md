# Broken Features Needing Revisiting

## LED Weather Alert Flashing System

### Status: CLOSED - Superseded by hardware redesign (March 2026)

### Resolution
This feature is a **non-issue** in the final hardware design. The HT16K33-addressed LEDs are now dedicated exclusively to the HybridClock hour ring display and are no longer available for weather alert flashing. Repurposing them for alerts would conflict with the clock display.

The alert triggering logic in `main.cpp` still works correctly; only the visual feedback mechanism (NeoPixel flashing via `LightingEffects`) is obsolete. `LightingEffects` is already a stub class and can remain so.

### Planned Replacement: Scrolling Alert Details

When a weather alert fires, the intended new behavior is:
1. Display the alert type (as currently)
2. **Follow immediately with scrolling descriptive text** explaining what the alert means and why it triggered

Examples:
- `PRES DROP - STORM LIKELY`
- `HEAT WARN - FEELS 102F`
- `RAPID TEMP CHANGE DETECTED`

This is strictly better UX than silent LED flashing — it gives the user actionable information rather than a visual indicator they might miss. Implementation notes:
- The alert type enum (`PRESSURE`, `TEMPERATURE`, `RAPID_CHANGE`) provides a natural hook for message generation
- Message strings can be plain `const char[]` — RAM is not the bottleneck on the Nano Every (54% used); flash is. `PROGMEM` adds `pgm_read_*` overhead in flash and is not beneficial here
- The HT16K33 display manager already supports scrolling; no new mechanism needed

### Original Debugging History (archived for reference)

Multiple non-blocking state machine approaches were attempted over several hours without success. A blocking `delay()`-based version worked correctly in isolation but could not be integrated cleanly. The root cause was never definitively identified — candidates included state machine logic errors, normal-lighting update interference, and memory pressure at 98.3% flash usage. This investigation is considered closed.


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
