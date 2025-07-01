# 📚 SHtoolsESP32 - Atualizado

Este projeto fornece uma biblioteca para ESP32 com recursos integrados para:

✅ **WebServer OTA** (servidor para atualização de firmware)  
✅ **Configurações persistentes** via Preferences  
✅ **Comunicação P2P** entre ESPs via ESP-NOW  
✅ **Roteamento centralizado de comandos** com verificação de validade via catálogo de comandos.

---

## 🗂️ Arquivos principais

- `SHtoolsESP32.h` + `SHtoolsESP32.cpp`: implementação do servidor OTA, EspNow, e funções auxiliares.
- `SHtools_cmd_rotas.h` + `SHtools_cmd_rotas.cpp`: roteamento de comandos, com checagem de cmdId no catálogo oficial.
- `SHtools_cmd.h`: catálogo fixo com a lista de comandos válidos para os dispositivos.
- `main.cpp`: exemplo de sketch que usa SHtools, define handlers com `SHtoolsESP32::registrarComando()`.

---

## 🚦 Como registrar um comando

Em seu `setup()` do sketch:

```cpp
SHtoolsESP32::registrarComando(1, [](int executar, int arg1, int arg2, String argStr) -> int {
    if (executar) {
        // executar ação real
        return 1; // sucesso
    }
    return 0; // falha ou não executado
});
```

### Importante:
- **`cmdId`** deve existir no catálogo em `SHtools_cmd.h`, senão registro falha.
- `registrarComando()` agora retorna **1 (ok)** ou **0 (falha)**. Use este retorno para log ou tratamento no sketch.

---

## 📡 Como enviar comandos via ESP-NOW

```cpp
String msg = SHtoolsESP32::EspNow::criarMSGcomando(1, 1, 10, 0, "param");
bool ok = SHtoolsESP32::EspNow::EspNow_EnviarDados(peerMac, msg);
```

---

## 📬 Como receber comandos

Quando outro ESP enviar um comando, será tratado por `EspNow_CallbackReceber()`, que chama internamente:
- `processarComando()` → `cmd_rotas::route()` → handler registrado.
- Resultado do handler define se mensagem foi processada com sucesso (1) ou falha (0).

---

## 🔎 Depuração

- Use `Auxiliares::printDEBUG(...)` ou `Auxiliares::printMSG(...)`.
- Ative/desative debug em runtime via comando `msgDEBUG` no WebSocket/Serial.
