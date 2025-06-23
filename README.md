# SHtoolsESP32

A biblioteca `SHtoolsESP32` permite que o ESP32 exiba remotamente as informações do serial monitor, receba atualizações de firmware via Wi-Fi em modo AP e se comunique com outros ESP através da tecnologia EspNow. 

Com ela, você pode atualizar o firmware do seu dispositivo remotamente e permitir comunicação direta entre vários ESP32 sem rede Wi-Fi ou internet.

## Recursos

- **OTA via Wi-Fi (modo AP)**
- **Servidor web assíncrono com ESPAsyncWebServer**
- **Serial monitor remoto com envio de comandos**
- **Comunicação direta com EspNow**
- **Rollback e verificação de firmware**

## Instalação

### Usando PlatformIO

Adicione ao `platformio.ini` do seu projeto:

```ini
lib_deps = https://github.com/ShafickCruz/SHtoolsESP32.git
```

> 📦 Todas as dependências (ESPAsyncWebServer, AsyncTCP etc.) já estão embutidas.  
> Nenhuma outra biblioteca precisa ser instalada manualmente.

## Exemplo básico

```cpp
#include <SHtoolsESP32.h>

const int ledPin = 23;
const int buttonPin = 27;

void setup() {
  SHtoolsESP32::setup(ledPin, buttonPin, "MeuSketch", nullptr, nullptr);
}

void loop() {
  SHtoolsESP32::Servidor::loop();
}
```
