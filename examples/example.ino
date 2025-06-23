#include <SHtoolsESP32.h>

const int ledPin = 23;
const int buttonPin = 27;

void setup() {
  SHtoolsESP32::setup(ledPin, buttonPin, "ExemploOTA", nullptr, nullptr);
}

void loop() {
  SHtoolsESP32::Servidor::loop();
}
