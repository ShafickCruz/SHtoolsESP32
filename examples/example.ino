#include <SHtoolsESP32.h>

// Defina os pinos do LED, do botão e o nome do sketch
const int ledPin = 23;
const int buttonPin = 27;
String nomeSketch = "MeuSketch";

void setup()
{
    // Inicialize a biblioteca
    SHtoolsESP32::setup(ledPin, buttonPin, "MeuSketch", nullptr, nullptr);
}

void loop()
{
    // Associe o loop da biblioteca ao loop de seu projeto
    SHtoolsESP32::Servidor::loop();
}