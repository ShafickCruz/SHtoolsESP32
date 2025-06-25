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


## Instalação e dependências

A partir desta versão, a instalação da biblioteca SHtoolsESP32 e suas dependências deve ser feita **manualmente** (ou via script).

### Passos obrigatórios:

1. **Baixe o repositório SHtoolsESP32** e coloque toda a pasta em `lib/` do seu projeto PlatformIO.

2. **Copie as dependências AsyncTCP, ESPAsyncTCP e ESP Async WebServer** para dentro da pasta `lib/` do seu projeto.  
   Todas essas dependências estão disponíveis em `lib/` dentro do próprio repositório SHtoolsESP32, podendo ser copiadas manualmente ou via script.

3. (Opcional, recomendado) **Automatize o processo com o script `BaixarSHtools.py`**:
    - O script está disponível em `tools/BaixarSHtools.py` dentro do repositório SHtoolsESP32.
    - Copie `BaixarSHtools.py` para a pasta `tools/` do seu projeto PlatformIO.
    - Adicione no `platformio.ini` do seu projeto:
      ```
      extra_scripts = 
          pre:tools/BaixarSHtools.py
      ```
    - O script irá baixar o repositório SHtoolsESP32 e copiar as dependências automaticamente para `lib/` do projeto, **caso ainda não estejam presentes**.

4. **Não utilize mais `lib_deps` para SHtoolsESP32 ou suas dependências**. O método recomendado é **sempre usar o código local**.

---

## Resumo do novo fluxo

- Apenas `lib/` do projeto principal importa.
- O script facilita a instalação e atualização local.
- Você tem total controle sobre as versões usadas, sem downloads automáticos inesperados.

---

**Exemplo de configuração no platformio.ini**:
```ini
[env:esp32doit-devkit-v1]
extra_scripts = 
    pre:tools/BaixarSHtools.py
framework = arduino
platform = espressif32
board = esp32doit-devkit-v1
build_flags =
    -Wall -Wextra
    -D CONFIG_ARDUHAL_LOG_COLORS
    -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_DEBUG
    -D VERSION_MACRO="1.1.1"
    -I include
monitor_speed = 115200
monitor_filters = esp32_exception_decoder, log2file
```

---

Para detalhes de uso da biblioteca, consulte os exemplos abaixo (sem alterações na API de uso).

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
