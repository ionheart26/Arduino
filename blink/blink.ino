#include <Blink.h>

Blink blink(4);

void setup() {
  Serial.begin(115200);
  Serial.println(“\nBlinkVersion 1.0 Your_First_NameYour_Last_Name”);
}

void loop() {
  blink.on(500);
  blink.off(300);
}
