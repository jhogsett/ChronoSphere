In lib/HybridClock/ClockTime.cpp:

> +    secondChanged = (newSecond != lastSecond);
+    minuteChanged = (newMinute != lastMinute);
+    hourChanged = (newHour != lastHour);

The change detection compares the freshly read values against lastSecond/lastMinute/lastHour, which start at -1 and aren't updated unless a change is detected. This makes secondChanged effectively always true. 

DO THIS:
Verify that the suggest is correct, and if so, implement the PR Review suggested change: "Compare against currentSecond/currentMinute/currentHour instead, then shift current* into last* before storing the new value."

⬇️ Suggested change
-    secondChanged = (newSecond != lastSecond);
-    minuteChanged = (newMinute != lastMinute);
-    hourChanged = (newHour != lastHour);
+    secondChanged = (newSecond != currentSecond);
+    minuteChanged = (newMinute != currentMinute);
+    hourChanged = (newHour != currentHour);

----------

In lib/HybridClock/Clock.cpp:

> +    // Store RTC reference
+    if (rtcPtr != nullptr) {
+        externalRTC = rtcPtr;
+        usingExternalRTC = true;
+        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Using external RTC instance"));
+    } else {
+        usingExternalRTC = false;
+        if (verboseLogging) SERIAL_PRINTLN(F("Clock: Using internal RTC instance"));

begin(DS3231* rtcPtr) stores the external RTC pointer, but nothing in the implementation uses it—ClockTime always reads from its own internal DS3231 rtc. This makes the rtcPtr parameter misleading to callers. 

DO THIS:
Determine which is the proper thing to do given that the PR review suggests: "Either plumb the pointer into ClockTime, or drop the parameter/state."


----------

In src/main.cpp:

> +  // Don't trigger new alerts if we're in the cooldown period
+  unsigned long currentMillis = millis();
+  if (currentMillis - lastAlertTime < ALERT_COOLDOWN_MS) {
+    return;  // Still in cooldown from previous alert
   }
lastAlertTime starts at 0, so during the first ~10 minutes after boot (millis() < ALERT_COOLDOWN_MS) this cooldown check will suppress all alerts even though no alert has ever fired. 

DO THIS:
Determine if this is the correct thing to do given the PR Review suggests: "Initialize lastAlertTime so the first alert can trigger immediately (e.g., set it to millis() - ALERT_COOLDOWN_MS in setup(), or use a separate bool hasTriggeredAlert guard)."


----------


In platformio.ini:

> @@ -22,9 +22,7 @@ lib_deps =
 	adafruit/Adafruit AHTX0@^2.0.3
 	dfrobot/DFRobot_BMP280@^1.0.1
 	claws/BH1750@^1.3.0
-	adafruit/Adafruit NeoPixel@^1.11.0
 	paulstoffregen/Encoder@^1.4.4

env:nanoatmega328 no longer declares NeoPixel/Stepper dependencies, but src/main.cpp now instantiates HybridClock which requires both libraries. Building the nanoatmega328 environment will likely fail to compile unless these deps are re-added (or the clock code is conditionally compiled per environment).

DO THIS:
Since the device can only run on the Arduino Nano Every, remove the other device sections.

