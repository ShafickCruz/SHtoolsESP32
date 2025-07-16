
# SHtoolsESP32 - Biblioteca de Comunicação e Controle para ESP32

## Visão Geral

A **SHtoolsESP32** é uma biblioteca modular que provê suporte a:

- Comunicação entre ESP32 via **ESP-NOW**
- Controle remoto via comandos padronizados (protocolo SHtools)
- Registro e gerenciamento de **Peers**
- Servidor Web integrado (OTA, debug, info)
- Watchdog e verificação de comunicação
- Utilitários auxiliares para debug, delay cooperativo, reinício etc.

---

## Estrutura dos Comandos

### Formato de Mensagem

```
assinatura|comando|arg1|arg2|argSTR
```

- **assinatura**: identificador (normalmente o nome do módulo)
- **comando**: número do comando
- **arg1**, **arg2**: inteiros (pode ser -1 para ignorar)
- **argSTR**: string com parâmetros (pode ser "" se não usado)

### Registro de Comandos

```cpp
SHtoolsESP32::registrarComando(3, [](int arg1, int arg2, String argStr) -> bool
{
    // Processa comando
    return true; // Sucesso
});
```

---

## Envio de Comandos

### Criação da Mensagem

```cpp
String msg = SHtoolsESP32::EspNow::criarMSGcomando(comando, arg1, arg2, "param");
```

### Envio da Mensagem

```cpp
SHtoolsESP32::EspNow::EspNow_EnviarDados("nomePeer", msg);
```

---

## Gerenciamento de Peers

A lib possui um sistema interno de peers com identificação por nome.

### Funções principais

| Função | Descrição |
|---|---|
| `registrarPeer(nome, mac)` | Adiciona ou substitui peer |
| `removerPeer(nome)` | Remove peer pelo nome |
| `existePeer(nome)` | Verifica se o peer existe |
| `getMAC_Peer(nome)` | Retorna o MAC do peer |
| `listarPeer_JSON()` | Retorna JSON com os peers atuais |

Os peers são armazenados em Preferences e persistem após reboot.

---

## Catálogo de Comandos Padrão

| Comando | Descrição |
|---|---|
| 1 | toBANHEIRA_SOLICITAR_ESTADO_LED |
| 2 | toBANHEIRA_ALTERAR_ESTADO_LED |
| 3 | fromBANHEIRA_ESTADO_LED_ATUALIZADO |
| 4 | toREGAJARDIM_BACKEND_REGA1_ALTERAR_ESTADO |
| 5 | toREGAJARDIM_BACKEND_REGA2_ALTERAR_ESTADO |
| 6 | toREGAJARDIM_BACKEND_BOMBAFILTRO_ALTERAR_ESTADO |
| 7 | fromREGAJARDIM_BACKEND_STATUS |
| 8 | toREGAJARDIM_SOLICITAR_STATUS |

---

## Servidor Web Integrado

- Acesso a debug e logs via HTTP
- Suporte a OTA (opcional)
- Monitoramento remoto

---

## Utilitários Auxiliares

| Função | Descrição |
|---|---|
| `printDEBUG(msg)` | Print em caixa alta via serial |
| `delayYield(ms)` | Delay cooperativo sem travar loop |
| `ReiniciarESP()` | Reinicia o ESP32 |
| `verificadorGenerico()` | Watchdog padrão (tempo, ESP-NOW, etc) |
| `preferencias(namespace, chave, valor)` | Escreve/ler Preferences de forma facilitada |

---

## Arquitetura

```
SHtoolsESP32/
├── SHtoolsESP32.cpp / .h
├── SHtools_cmd_rotas.cpp / .h
├── SHtools_peers.cpp / .h
```

---

## Autor

Shafick Cruz  
Julho/2025

