#ifndef SHtoolsESP32_H
#define SHtoolsESP32_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncWebSocket.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <Preferences.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_spi_flash.h>
#include <soc/rtc.h>
#include <mbedtls/sha256.h>
#include <pgmspace.h> // Necessário para PROGMEM que esta sendo definido dentro dos arquivos binarios do webserver
#include <esp_now.h>

namespace SHtoolsESP32
{
    extern bool HabilitarDebug;
    static bool *BanheiraLed_ON = nullptr;           // Ponteiro para a variável
    static bool *BanheiraLed_ON_viaEspNow = nullptr; // Ponteiro para a variável
    typedef void (*funcao_led_banheira)(bool, bool); // Declaração do tipo de ponteiro para função

    void setup(int _ledPin, int _buttonPin, String _nomeSketch,
               bool *_BanheiraLed_ON = nullptr,
               bool *_BanheiraLed_ON_viaEspNow = nullptr,
               funcao_led_banheira _led_banheira = nullptr);

    namespace Servidor
    {
        void init();
        void loop();
        void ServerMod_handle();
        void bt_handle();
        void startServerMod(bool _softRestart);
        bool ServerMod();
        void rotasEcallbacks();
        bool WifiSetup();
        String obterInformacoesPlaca();
        void OTA_FirmwareUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);
        String OTA_FirmwareUpdate_ChecksumFinal(mbedtls_sha256_context *ctx);
        bool SerialCMD(String _cmd);
        const char *gerarSSID();
    }

    namespace EspNow
    {
        extern bool EspNowHabilitado;
        extern bool EspNowACK;

        bool EspNow_init();
        void EspNow_configurarPeer(esp_now_peer_info_t &peerInfo, const uint8_t *macAddress);
        bool EspNow_adicionarPeer(const uint8_t *macAddress);
        bool EspNow_removerPeer(const uint8_t *macAddress);
        String criarMSGcomando(int executar, int comando, int arg1, int arg2, const char *argSTR);
        bool EspNow_EnviarDados(uint8_t *peer, String msg, unsigned short ACK_timeout_ms = 3000);
        void EspNow_CallbackReceber(const uint8_t *peer, const uint8_t *incomingData, int len);
        int processarComando(const char *msgRecebida);
        void dividirString(const String &str, char sep, std::vector<String> &partes);
        int getBanheiraLed_ON();
        int getBanheiraLed_ON_viaEspNow();
        void setBanheiraLed_ON(bool value);
        void setBanheiraLed_ON_viaEspNow(bool value);
        void chamarLedBanheira(bool _alterar, bool _fromEspNow); // Declaração de funções para usar os ponteiros
    }

    namespace Auxiliares
    {
        bool preferencias(int8_t _opcao, const char *_chave = "", bool _valor = false);
        void printMSG(const String &_msg, bool newline, bool _debug = false);
        void printDEBUG(String _msg);
        void delayYield(unsigned long ms = 1000);
        void ReiniciarESP(int _tempoDelay = 1000);
    }
}
#endif
