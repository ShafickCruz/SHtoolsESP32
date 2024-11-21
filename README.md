# SHtoolsESP32

A biblioteca `SHtoolsESP32` permite que o ESP32 exiba remotamente as informações do serial monitor, receba atualizações de firmware via Wi-Fi em modo AP e se comunique com outros ESP através da tecnologia EspNow. Com ela, você pode atualizar o firmware do seu dispositivo de forma remota e promova a comunicação direta entre vários ESP sem a necessidade de uma rede Wi-Fi intermediária ou internet. Utilize um botão físico para iniciar o processo de inicio do modo servidor.

**Prática**: Ao pressionar o botão de atualização por 3 segundos, um LED começa a piscar e um servidor web assíncrono em modo access point é criado. Você deve acessar a rede Wi-Fi criada e, no navegador, inserir o IP do ESP32 que está no SSID. Em seguida, selecionar uma das opções que deseja utilizar, entre autualizações OTA, serial monitor remoto, etc.

## Recursos

- **OTA via Wi-Fi**: Faça atualizações de firmware através de uma rede Wi-Fi e da tecnologia OTA.
- **Modo Async**: Utilize esses recursos de forma assíncrona através de um `AsyncWebServer` criado pela biblioteca `ESPAsyncWebServer`.
- **Modo AP**: Elimine a necessidade de uma rede Wi-Fi intermediária através do Access Point criado pelo ESP32.
- **Serial Monitor**: Visualize informações do serial monitor de forma remota.
- **Envio de comandos**: Utilize o serial monitor remoto para enviar comandos previamente definidos ao ESP32.
- **EspNow**: Promova comunicação entre várias placas ESP de forma direta.

## Instalação

### Usando o Gerenciador de Bibliotecas do Arduino

1. Abra o Arduino IDE.
2. Vá para **Sketch** > **Include Library** > **Manage Libraries...**.
3. Na barra de pesquisa, digite `SHtoolsESP32`.
4. Selecione a biblioteca e clique em **Install**.

### Usando PlatformIO

Adicione a seguinte linha ao seu `platformio.ini`:
lib_deps = (https://github.com/ShafickCruz/SHtoolsESP32.git)

### Dependências Herdadas

As seguintes bibliotecas serão herdadas. Certifique-se se qeu elas estão instaladas em seu projeto:

```
<Arduino.h>
<WiFi.h>
<ESPAsyncWebServer.h>
<AsyncTCP.h>
<AsyncWebSocket.h>
<Update.h>
<esp_ota_ops.h>
<Preferences.h>
<esp_system.h>
<esp_chip_info.h>
<esp_spi_flash.h>
<soc/rtc.h>
<mbedtls/sha256.h>
<pgmspace.h>
<esp_now.h>
```

## Uso

### Exemplo Básico

Aqui está um exemplo de como usar a biblioteca para configurar o ESP32 para atualizações OTA via Wi-Fi:

```
#include <SHtoolsESP32.h>

// Defina os pinos do LED, do botão e o nome do sketch
const int ledPin = 23;
const int buttonPin = 27;
String nomeSketch = "MeuSketch";

void setup() {  
// Inicialize a biblioteca
SHtoolsESP32::setup(ledPin, buttonPin, "MeuSketch", nullptr, nullptr);
}

void loop() {
  // Associe o loop da biblioteca ao loop de seu projeto
  SHtoolsESP32::Servidor::loop();
}
```
