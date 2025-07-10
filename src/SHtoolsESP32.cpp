// src/SHtoolsESP32.cpp

#include "SHtoolsESP32.h"

// Include dos arquivos binarios para o webserver
#include "cmd_html.h"
#include "favicon_ico.h"
#include "index_html.h"
#include "info_html.h"
#include "ota_html.h"
#include "script_js.h"
#include "serial_html.h"
#include "sha_js.h"
#include "style_css.h"

// ****************************************************
// ****************** SHtoolsESP32 ********************
// ****************************************************
namespace SHtoolsESP32
{
  // ****** VARIAVEIS

  // setup
  int ledPin;
  int buttonPin;
  String nomeSketch;

  // geral
  bool WIFIradio_OFF = true;
  bool HabilitarDebug = true;
  bool ServidorInicializado = false;

  // preferences
  Preferences config; // instancia para configrações usando preferences
  // Limite máximo de 14 caracteres para Namespace e Key
  const char *PrefNameSpace_config = "_config";
  const char *PrefKey_configOK = "_configOK";
  const char *PrefKey_serverMod_auto = "ServerMod_AUTO";
  const char *PrefKey_novoFirmware = "NovoFirmware";
  const char *PrefKey_testeNovoFirmware = "tstNewFirmware";
  const char *PrefKey_fezRollback = "fezRollback";

  // ****** IMPLEMENTAÇÃO

  /// @brief Função para inicializar a biblioteca
  /// @param _ledPin
  /// @param _buttonPin
  /// @param _nomeSketch
  void setup(int _ledPin, int _buttonPin, String _nomeSketch)
  {
    ledPin = _ledPin;
    buttonPin = _buttonPin;
    nomeSketch = _nomeSketch;

    // Configura os pinos de I/O
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.begin(115200);

    Auxiliares::preferencias(-1); // monta config se não estiver montado

    // setup do servidor
    Servidor::init();
  }

  // ****************************************************
  // ******************* Servidor ***********************
  // ****************************************************
  namespace Servidor
  {
    // ****** VARIAVEIS

    bool ServerMod_AUTO = 0;
    bool ServerMod_ON = false;
    String controle_testeNovoFirmware_detalhe = "";
    int controle_testeNovoFirmware = 1; // 1 = pong(ping-pong)
    AsyncWebSocket ws("/ws_rota");
    unsigned long ServerModInicio = 0;
    unsigned long lastButtonStateChangeTime = 0;
    int lastButtonState = HIGH;
    unsigned long debounceDelay = 50;
    unsigned long buttonPressTime = 0;
    unsigned long longPressDuration = 3000;
    AsyncWebServer server(80);
    const char *HeaderOTA = "X-OTA-Header";
    const String HeaderOTAassinatura = "SHtoolsOTA";
    IPAddress rede_ip(192, 168, 100, 100);
    IPAddress rede_mask(255, 255, 255, 0);
    IPAddress rede_gateway(192, 168, 100, 1);
    const String ParamChecksum = "checksum";
    const char *HeaderOTAtestarFirmware = "X-TestarFirmware";
    bool restartSolicitado = false;

    // ****** IMPLEMENTAÇÃO

    /// @brief Função para inicializar o servidor
    void init()
    {
      ServidorInicializado = true;
      ServerMod_AUTO = Auxiliares::preferencias(2, PrefKey_serverMod_auto); // Obtem o valor para ServerMod_AUTO

      // Marca firmware como válido sem testar o firmware
      // Cliente web não está esperando resposta
      if (Auxiliares::preferencias(2, PrefKey_novoFirmware) && !Auxiliares::preferencias(2, PrefKey_testeNovoFirmware))
      {
        Auxiliares::printDEBUG("DETECTADO NOVO FIRMWARE SEM SOLICITAÇÃO DE TESTES");

        // desmarca novo firmware em preferencias
        Auxiliares::preferencias(1, PrefKey_novoFirmware, false);

        esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
        if (err == ESP_OK)
        {
          Auxiliares::printMSG("Novo firmware definido como válidado sem testes de funcionalidades.", true);
        }
        else
        {
          Auxiliares::printMSG("Erro ao definir novo firmware como válido. Iniciando processo de rollback com restart...\nDetalhes: " + String(esp_err_to_name(err)), true);
          // não há marcação de rollback efetuado porque sem solicitação de teste de novo firmware, o web cliente não aguarda resposta
          ESP.restart(); // restart rápido para tentar reestabelecer conexão com cliente antes de timout do servidor
        }
      }
    }

    /// @brief Função de handle para processar as requisições de servidor
    void loop()
    {
      yield();

      // DEBUG
      /*
      if (HabilitarDebug)
      {
        const int IntervaloTeste = 3000;
        static unsigned long testeUltimoTempo = millis();

        if (((unsigned long)millis() - testeUltimoTempo) >= IntervaloTeste)
        {
          printDEBUG("SERVERMOD ON = " + String(ServerMod_ON));
          testeUltimoTempo = millis();
        }
      }
      */

      /*
      Se estiver no modo Servidor, faz o LED piscar continuamente e processa as requisições. Se não estiver, verifica se debug inicial está habilitado.
      Se debug inicial estiver habilitado, inicia o processo de ServerMod e ignora o botão e se não estiver, keep watching o botão.
      */
      if (ServerMod_ON)
      {
        ServerMod_handle();
      }
      else
      {
        if (ServerMod_AUTO)
        {
          startServerMod(!(Auxiliares::preferencias(2, PrefKey_testeNovoFirmware)));

          Auxiliares::printDEBUG("SUCESSO NA INICIALIZAÇÃO DE SERVERMOD OU FALHA + SOLICITAÇÃO DE TESTE DE NOVO FIRMWARE");

          /*
          Se chegoun até aqui é porque startServerMod obteve sucesso ou
          teste de novo firmware está marcadao como true em preferences.
          Quando startServerMod não obtem sucesso, ocorre restart em seu processo (ServerMod()).

          Formas de solicitar ServerMod_AUTO:
          - CMD (web cliente)
          - Botão físico
          - Teste de novo firmware (OTA)
          */

          /*********************
          TESTE DE NOVO FIRMWARE
          *********************/

          if (ServerMod_ON)
          {
            if (Auxiliares::preferencias(2, PrefKey_testeNovoFirmware, true)) // teste de novo firmware obteve sucesso
            {
              Auxiliares::printDEBUG("SUCESSO NO TESTE DE NOVO FIRMWARE");

              // desmarca novo firmware em preferencias
              Auxiliares::preferencias(1, PrefKey_novoFirmware, false);

              // desmarca teste de novo firmware em preferencias
              Auxiliares::preferencias(1, PrefKey_testeNovoFirmware, false);

              Auxiliares::printMSG("Novo firmware executou testes com sucesso. Iniciando tentativa de marcar firmware como válido...", true);

              // Mark firmware as valid and cancel rollback process
              esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
              if (err == ESP_OK)
              {
                Auxiliares::printMSG("Novo firmware pronto a ser utilizado!", true);
                controle_testeNovoFirmware_detalhe = "Novo firmware pronto a ser utilizado!";
                controle_testeNovoFirmware = 2;
              }
              else
              {
                Auxiliares::printMSG("Erro ao definir novo firmware como válido. Iniciando processo de rollback com restart...\nDetalhes: " + String(esp_err_to_name(err)), true);
                controle_testeNovoFirmware_detalhe = "Erro ao definir novo firmware como válido. Iniciando processo de rollback com restart...";
                controle_testeNovoFirmware = 0;
                ESP.restart(); // restart rápido para tentar reestabelecer conexão com cliente antes de timout do servidor
              }
            }
          }
          else
          // ServerMod_ON sendo false e não havido boot no processo de ServerMod, significa que teste de novo firmware falhou
          {
            Auxiliares::printDEBUG("FALHA NO TESTE DE NOVO FIRMWARE");

            Auxiliares::printMSG("Teste de novo firmware falhou. Iniciando tentativa de roolback.", true);
            // sem possibilidades de informar ao web cliente

            // desmarca novo firmware em preferencias
            Auxiliares::preferencias(1, PrefKey_novoFirmware, false);

            // desmarca teste de novo firmware em preferencias
            Auxiliares::preferencias(1, PrefKey_testeNovoFirmware, false);

            /*
            Marca na preferencias que fez rollback
            Mesmo se não conseguir fazer, tratará como se tivesse feito, pois se o teste de novo firmware falhou
            e não conseguiu fazer rollback, o sistema estará inválido e não fará diferença esta marcação.
            */
            Auxiliares::preferencias(1, PrefKey_fezRollback, true);

            Auxiliares::printDEBUG("INICIO DE ROLLBACK SEGUIDO DE RESTART");

            // Mark firmware as invalid and rollback to previous firmware
            esp_err_t err = esp_ota_mark_app_invalid_rollback_and_reboot();

            // Provavelmente não chegará até aqui, pois poderá haver restart imediato
            if (err == ESP_OK)
            {
              Auxiliares::printMSG("Iniciando processo de rollback...", true);
              // sem possibilidades de informar ao web cliente
            }
            else
            {
              Auxiliares::printMSG("Não há firmware disponível para processo de rollback ou processo de rollback falhou.\nDetalhes: " + String(esp_err_to_name(err)), true);
              // sem possibilidades de informar ao web cliente
            }
          }
        }
        else
        {
          bt_handle();
        }
      }
    }

    /// @brief Função para processar as requisições quando estiver em ServerMod ON
    void ServerMod_handle()
    {

      yield();
      // Verifica se havia solicitação de teste de novo firmware e o mesmo falhou
      // Retorna mensagem para web cliente
      if (Auxiliares::preferencias(2, PrefKey_fezRollback))
      {
        controle_testeNovoFirmware_detalhe = "Falha ao iniciar modo servidor. Rollback efetuado com sucesso.";
        controle_testeNovoFirmware = 0;
        Auxiliares::preferencias(1, PrefKey_fezRollback, false); // reseta valor de chave de preferencia
      }

      ws.cleanupClients(); // Processa eventos do WebSocket e mantém o WebSocket ativo e limpa conexões inativas

      unsigned long cTime = millis();

      /*
      Se está em modo servidor há mais de 30 minutos,
      desativa o modo ServerMod_AUTO e reinicia o esp para sair do modo servidor
      */
      if ((cTime - ServerModInicio) >= 1800000) // 30 minutos
      {
        if (WiFi.softAPgetStationNum() == 0) // Verifica se há clientes conectados
        {
          ServerMod_AUTO = Auxiliares::preferencias(1, PrefKey_serverMod_auto, false); // Desativa ServerMod_AUTO
          Auxiliares::ReiniciarESP();                                                  // Reinicia o ESP32
        }
      }

      // Faz o LED piscar continuamente
      static unsigned long lastBlinkTime = 0;
      unsigned long blinkInterval = 150; // Piscar a cada 150ms
      cTime = millis();

      if ((cTime - lastBlinkTime) >= blinkInterval)
      {
        digitalWrite(ledPin, !digitalRead(ledPin)); // Inverte o estado do LED
        lastBlinkTime = cTime;
      }
    }

    /// @brief Função para monitorar os processor de press do botão físico
    void bt_handle()
    {
      int currentButtonState = digitalRead(buttonPin); // Lê o estado atual do botão
      unsigned long currentTime = millis();            // Captura o tempo atual

      // Verifica se o estado do botão mudou e se passou o tempo suficiente para o debounce
      if (currentButtonState != lastButtonState)
      {
        if ((currentTime - lastButtonStateChangeTime) > debounceDelay)
        {
          if (currentButtonState == LOW)
            buttonPressTime = currentTime; // Registra o momento em que o botão foi pressionado

          lastButtonStateChangeTime = currentTime;
        }
      }

      // Verifica se o botão está pressionado e se o tempo de pressionamento excede longPressDuration
      if (currentButtonState == LOW && (currentTime - buttonPressTime) >= longPressDuration)
      {
        // aguarda o botão ser liberado (de LOW para HIGH)
        digitalWrite(ledPin, !digitalRead(ledPin)); // apaga ou acende o led para indicar que o longpress foi reconhecido
        while (digitalRead(buttonPin) == LOW)
        {
          Auxiliares::delayYield(100);
        }

        // Tenta iniciar modo servidor e reinicia caso não consiga
        startServerMod(true);
      }

      // Atualiza o estado anterior do botão
      lastButtonState = currentButtonState;
    }

    /// @brief Função para iniciar o ServerMod ON
    /// @param _softRestart Promover restart limpo se houver falha ao iniciar ServerMod ON?
    void startServerMod(bool _softRestart)
    {
      yield();

      if (!ServerMod())
      {
        Auxiliares::printMSG("Falha ao iniciar modo servidor!", true);

        // se ServerMod_AUTO estiver true e houver falha na inicialização do modo servidor,
        // ficará em loop infinito. Para evitar, deve-se desativar o ServerMod_AUTO
        if (ServerMod_AUTO)
        {
          ServerMod_AUTO = Auxiliares::preferencias(1, PrefKey_serverMod_auto, false);
        }

        if (_softRestart)
        {
          Auxiliares::printMSG("Restart em 5 segundos...", true);
          Auxiliares::delayYield(4000);
          Auxiliares::ReiniciarESP();
        }
      }
    }

    /// @brief Funçao para promover o inicio de ServerMod ON
    /// @return true/false = sucesso/falha nas inicializações
    bool ServerMod()
    {
      yield();

      ServerMod_ON = false;

      Auxiliares::printMSG("Entrando em modo Servidor...", true);

      if (!WifiSetup()) // configura e inicializa o servidor wifi
      {
        Auxiliares::printMSG("Falha ao iniciar WiFi!", true);
        return false;
      }

      server.addHandler(&ws); // Inicializa o WebSocket

      rotasEcallbacks(); // define rotas e callbacks

      server.begin(); // Start webserver

      Auxiliares::printMSG("Servidor iniciado", true);
      ServerMod_ON = true;
      ServerModInicio = millis();

      return true;
    }

    /// @brief Função container para rotas/endpoints e callbacks utilizadas para ServerMod
    void rotasEcallbacks()
    {
      //
      // Status do server - PingPong
      //
      server.on("/server_status", HTTP_GET, [](AsyncWebServerRequest *request)
                {
              // Verificar se o cabeçalho personalizado está presente
              if (request->hasHeader(HeaderOTA))
              {
                String customHeaderValue = request->header(HeaderOTA); // Obter o valor do cabeçalho

                // Verificar se o valor é o esperado
                if (customHeaderValue == HeaderOTAassinatura)
                {
                  switch (controle_testeNovoFirmware)
                  {
                  case 0:
                  controle_testeNovoFirmware = 1; // reseta marcador
                    request->send(500, "text/plain", "Falha no teste de novo firmware.<br><strong>Detalhes:</strong> " + controle_testeNovoFirmware_detalhe); // resposta final
                    break;

                  case 1:
                    request->send(202); // utilizado para pong
                    break;

                  case 2:
                  controle_testeNovoFirmware = 1; // reseta marcador
                    request->send(200, "text/plain", "Servidor reiniciado.<br><strong>Detalhes:</strong> " + controle_testeNovoFirmware_detalhe); // resposta final
                    break;
                  }
                }
                else
                {
                  request->send(403, "text/plain", "Credenciais inválidas!"); // assinatura invalida
                }
              }
              else
              {
                request->send(403, "text/plain", "Credenciais ausentes!"); // cabeçalho ausente
              } });

      //
      // Rota para obter informações da placa
      //
      server.on("/placa_info", HTTP_GET, [](AsyncWebServerRequest *request)
                {
    // Verificar se o cabeçalho personalizado está presente
    if (request->hasHeader(HeaderOTA)) {
        String customHeaderValue = request->header(HeaderOTA); // Obter o valor do cabeçalho

        // Verificar se o valor é o esperado
        if (customHeaderValue == HeaderOTAassinatura) {
            // Obter informações da placa
            String jsonResponse = obterInformacoesPlaca();
            request->send(200, "application/json", jsonResponse); // Enviar a resposta como JSON
        } else {
            request->send(403, "text/plain", "Credenciais inválidas!"); // assinatura invalida
        }
    } else {
        request->send(403, "text/plain", "Credenciais ausentes!"); // cabeçalho ausente
    } });

      //
      // OTA: Rota para servir o upload de firmware
      //
      server.on("/upload_firmware", HTTP_POST, [](AsyncWebServerRequest *request)
                {
      // Cabeçalhos e permissões de upload
      if (request->hasHeader(HeaderOTA)) {
          String customHeaderValue = request->header(HeaderOTA);
          if (customHeaderValue != HeaderOTAassinatura) {              
              request->send(403, "text/plain", "Credenciais inválidas!"); // assinatura invalida
          }
      } else {
          request->send(403, "text/plain", "Credenciais ausentes!"); // cabeçalho ausente
      } }, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                { OTA_FirmwareUpdate(request, filename, index, data, len, final); });

      //
      // WEBSOCKET
      //

      ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                    AwsEventType type, void *arg, uint8_t *data, size_t len)
                 {
        switch (type) {
            case WS_EVT_CONNECT:
                Auxiliares::printMSG("Novo cliente conectado", true);
                break;
            case WS_EVT_DISCONNECT:
                Auxiliares::printMSG("Cliente desconectado", true);
                break;
            case WS_EVT_DATA:  // Mensagens recebidas por websocket
            {
              String message((char*)data, len); // Captura a mensagem recebida
              Auxiliares::printMSG(message, true); // Chama a função printMSG com a mensagem recebida
              break;
            }

            default:
                break;
        } });

      //
      // BINARIOS: Rota para servir os arquivos binarios
      //
      server.on("/cmd.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", cmd_html, cmd_html_len); });

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", index_html, index_html_len); });

      server.on("/info.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", info_html, info_html_len); });

      server.on("/ota.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", ota_html, ota_html_len); });

      server.on("/serial.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", serial_html, serial_html_len); });

      server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "application/javascript", script_js, script_js_len); });

      server.on("/sha.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "application/javascript", sha_js, sha_js_len); });

      server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/css", style_css, style_css_len); });

      server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "image/x-icon", favicon_ico, favicon_ico_len); });
    }

    /// @brief Função para configurar e iniciar processos do WiFi
    /// @return true/false = Sucesso ou falha nas inicialização/configurações
    bool WifiSetup()
    {
      // desconecta WiFi
      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.disconnect(WIFIradio_OFF, true);
        Auxiliares::delayYield(2000);
      }

      // Configura o servidor AP

      // Aguarda a inicialização do servidor wifi
      Auxiliares::printMSG("inicializando webserver.", true);
      unsigned long startTime = millis();  // Armazena o tempo de início
      const unsigned long timeout = 10000; // Tempo limite de 10 segundos
      WiFi.persistent(false);              // Desativa a persistência das configurações do Wi-Fi para não desgastar a memoria flash

      while (!WiFi.softAP(gerarSSID()))
      {
        Auxiliares::printMSG(".", false);
        Auxiliares::delayYield(250);

        // Verifica se o tempo limite foi atingido
        if ((unsigned long)millis() - startTime > timeout)
        {
          Auxiliares::printMSG("Falha ao iniciar o Access Point.", true);
          return false; // Retorna false se não conseguir iniciar
        }
      }

      // aplica as configurações
      WiFi.softAPConfig(rede_ip, rede_gateway, rede_mask);

      Auxiliares::printMSG("Access Point criado com sucesso!", true);
      Auxiliares::printMSG("SKETCH: " + nomeSketch, true);
      Auxiliares::printMSG("IP do ESP32: " + rede_ip.toString(), false);

      return true;
    }

    /// @brief Função para retornar informaçoes da placa ESP32
    /// @return String formato Json com informações da placa
    String obterInformacoesPlaca()
    {
      // Definindo a struct para armazenar as informações
      struct ESP32Info
      {
        String chipIsEP32 = "n/a";
        String chip = "n/a";
        String revisao = "n/a";
        String nucleos = "n/a";
        String cpuFreqMhz = "n/a";
        String cristal = "n/a";
        String flashType = "n/a";
        String flashSpeed = "n/a";
        String flashSize = "n/a";
        String flashDisponivel = "n/a";
        String ramTotal = "n/a";
        String ramDisponivel = "n/a";
        String mac = "n/a";
        String uptime = "n/a";
        bool wifi = false;
        bool ble = false;
        bool bt = false;
      } info;

      // Para obter informações detalhadas do chip
      esp_chip_info_t chip_info;
      esp_chip_info(&chip_info);

      String json;

      // Obter o modelo do chip
      const char *chipModel = ESP.getChipModel(); // Armazena como const char*

      if (chipModel)
      {
        info.chip = String(chipModel); // Converte para String
        info.chip.toUpperCase();

        // Verifica se o chip é ESP32
        if (info.chip.indexOf("ESP32") >= 0)
        {
          info.chipIsEP32 = "true";
        }
        else
        {
          info.chipIsEP32 = "false";
          json = "{\"chipIsEP32\": \"" + info.chipIsEP32 + "\"}"; // Monta JSON para chip não ESP32
          return json;                                            // Retorna JSON imediatamente se não for ESP32
        }
      }
      else
      {
        Auxiliares::printMSG("Erro ao tentar obter modelo do chip da placa", true);
        info.chipIsEP32 = "n/a";                                // Valor padrão em caso de erro
        json = "{\"chipIsEP32\": \"" + info.chipIsEP32 + "\"}"; // Monta JSON com erro
        return json;                                            // Retorna JSON imediatamente
      }

      // Atribuir valores às variáveis
      info.revisao = String(ESP.getChipRevision());
      info.nucleos = String(chip_info.cores);
      info.cpuFreqMhz = String(ESP.getCpuFreqMHz());  // MHz
      info.cristal = String(rtc_clk_xtal_freq_get()); // MHz
      info.flashType = String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "Embedded" : "External");
      info.flashSpeed = String(ESP.getFlashChipSpeed() / 1000000);        // MHz
      info.flashSize = String(spi_flash_get_chip_size() / (1024 * 1024)); // MB
      info.ramTotal = String(ESP.getHeapSize() / 1024);                   // KB
      info.ramDisponivel = String(ESP.getFreeHeap() / 1024);              // KB
      info.mac = WiFi.macAddress();
      info.uptime = String(millis() / 1000); // segundos;

      info.wifi = chip_info.features & CHIP_FEATURE_WIFI_BGN;
      info.ble = chip_info.features & CHIP_FEATURE_BLE;
      info.bt = chip_info.features & CHIP_FEATURE_BT;

      // Calcular flash disponível
      // uint32_t totalBytes = spi_flash_get_chip_size();
      info.flashDisponivel = String((ESP.getFlashChipSize() - ESP.getSketchSize()) / 1024); // KB

      // Montagem manual do objeto JSON
      json = "{";
      json += "\"chipIsEP32\": \"" + info.chipIsEP32 + "\",";
      json += "\"chip\": \"" + info.chip + "\",";
      json += "\"revisao\": \"" + info.revisao + "\",";
      json += "\"nucleos\": \"" + info.nucleos + "\",";
      json += "\"cpuFreqMhz\": \"" + info.cpuFreqMhz + "MHz\",";
      json += "\"cristal\": \"" + info.cristal + "MHz\",";
      json += "\"flashType\": \"" + info.flashType + "\",";
      json += "\"flashSpeed\": \"" + info.flashSpeed + "MHz\",";
      json += "\"flashSize\": \"" + info.flashSize + "MB\",";
      json += "\"flashDisponivel\": \"" + info.flashDisponivel + "KB\",";
      json += "\"ramTotal\": \"" + info.ramTotal + "KB\",";
      json += "\"ramDisponivel\": \"" + info.ramDisponivel + "KB\",";
      json += "\"mac\": \"" + info.mac + "\",";
      json += "\"uptime\": \"" + info.uptime + "s\",";
      json += "\"wifi\": " + String(info.wifi) + ",";
      json += "\"ble\": " + String(info.ble) + ",";
      json += "\"bt\": " + String(info.bt);
      json += "}";

      return json;
    }

    /// @brief Função principal na promoção do update de firmware via OTA
    /// @param request
    /// @param filename
    /// @param index
    /// @param data
    /// @param len
    /// @param final
    void OTA_FirmwareUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
    {
      static bool hasError = false;
      static bool testarFirmwareSolicitado = false;
      static String clientChecksum;
      static mbedtls_sha256_context ctx;
      String msgRetorno = "";

      // Se ocorreu um erro anterior, aborta o processo imediatamente
      if (hasError)
      {
        msgRetorno = "Erro anterior no upload. Processo abortado.";
        request->send(500, "text/plain", msgRetorno);
        Auxiliares::printMSG(msgRetorno, true);
        return;
      }

      if (index == 0)
      {
        // Inicialização
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0); // Inicia SHA-256

        if (request->hasParam(ParamChecksum, true))
        {
          clientChecksum = request->getParam(ParamChecksum, true)->value();
        }
        if (request->hasHeader(HeaderOTAtestarFirmware))
        {
          testarFirmwareSolicitado = request->getHeader(HeaderOTAtestarFirmware)->value() == "true";
        }

        // Verificações e inicializações
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
          hasError = true;
          msgRetorno = "Erro ao iniciar Update OTA";
          request->send(500, "text/plain", msgRetorno);
          Auxiliares::printMSG(msgRetorno, true);
          return;
        }
      }

      // Atualiza checksum e grava dados
      mbedtls_sha256_update(&ctx, data, len);
      if (Update.write(data, len) != len)
      {
        hasError = true;
        msgRetorno = "Erro ao gravar firmware";
        request->send(500, "text/plain", msgRetorno);
        Auxiliares::printMSG(msgRetorno, true);
        return;
      }

      if (final)
      {
        // Verifica checksum
        if (!clientChecksum.isEmpty())
        {
          String calculatedChecksum = OTA_FirmwareUpdate_ChecksumFinal(&ctx);
          if (calculatedChecksum != clientChecksum)
          {
            Update.abort();
            hasError = true;
            msgRetorno = "Checksum inválido";
            request->send(400, "text/plain", msgRetorno);
            Auxiliares::printMSG(msgRetorno, true);
            return;
          }
        }

        // Finaliza a atualização
        if (Update.end(true))
        {
          msgRetorno = "Upload via OTA bem sucedido! Ativando novo firmware...";
          request->send(200, "text/plain", msgRetorno);
          Auxiliares::printMSG(msgRetorno, true);
        }
        else
        {
          hasError = true;
          msgRetorno = "Erro ao finalizar o update OTA";
          request->send(500, "text/plain", msgRetorno);
          Auxiliares::printMSG(msgRetorno, true);
        }

        delay(500); // delay sincrono para dar tempo ao cliente processar a resposta antes do ESP32 reiniciar

        // Grava preferencia e faz restart para ativar novo firmware
        if (Update.isFinished())
        {
          Auxiliares::preferencias(1, PrefKey_novoFirmware, true);                    // informa que houve update de firmware
          ServerMod_AUTO = Auxiliares::preferencias(1, PrefKey_serverMod_auto, true); // Ativa ServerMod_AUTO

          if (testarFirmwareSolicitado)
          {
            Auxiliares::preferencias(1, PrefKey_testeNovoFirmware, true); // marca flag para testar novo firmware após restart
          }

          ESP.restart();
        }

        // Reinicializa as variáveis estáticas após sucesso ou falha
        hasError = false;
        testarFirmwareSolicitado = false;
        clientChecksum = "";
      }
    }

    /// @brief Função para calcular o checksum completo do arquivo de firmware recebido
    /// @param ctx
    /// @return String contendo rash sha256
    String OTA_FirmwareUpdate_ChecksumFinal(mbedtls_sha256_context *ctx)
    {
      uint8_t hash[32];
      mbedtls_sha256_finish(ctx, hash); // Finaliza o cálculo do hash

      // Converte o hash para string hexadecimal
      String checksumStr;
      for (int i = 0; i < 32; i++)
      {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        checksumStr += hex;
      }
      return checksumStr;
    }

    /// @brief Função de reonhecimento de funções enviadas por serial monitor remoto
    /// @param _cmd comando a ser reconhecido
    /// @return true/false = comando reconhecido/não reonhecido
    bool SerialCMD(String _cmd)
    {
      // Verifica qual comando foi enviado
      if (_cmd.equalsIgnoreCase("Teste"))
      {
        Auxiliares::printMSG(_cmd + ": ...teste ok...", true);
        return true;
      }
      else if (_cmd.equalsIgnoreCase("Restart"))
      {
        Auxiliares::printMSG(_cmd + " (executar): Em 3 segundos...", true);
        Auxiliares::ReiniciarESP(3000);
        return true;
      }
      else if (_cmd.equalsIgnoreCase("ServerMod_AUTO?"))
      {
        Auxiliares::printMSG(_cmd + " (estado): " + String(ServerMod_AUTO), true);
        return true;
      }
      else if (_cmd.equalsIgnoreCase("ServerMod_AUTO"))
      {
        Auxiliares::printMSG(_cmd + " (alterado): " + String(ServerMod_AUTO) + " >>>>> " + String(!ServerMod_AUTO), true);
        ServerMod_AUTO = Auxiliares::preferencias(1, PrefKey_serverMod_auto, !ServerMod_AUTO);
        Auxiliares::delayYield();
        restartSolicitado = true;
        return true;
      }
      else if (_cmd.equalsIgnoreCase("msgDEBUG?"))
      {
        Auxiliares::printMSG(_cmd + " (estado): " + String(HabilitarDebug), true);
        return true;
      }
      else if (_cmd.equalsIgnoreCase("msgDEBUG"))
      {
        Auxiliares::printMSG(_cmd + " (alterado): " + String(HabilitarDebug) + " >>>>> " + String(!HabilitarDebug), true);
        HabilitarDebug = !HabilitarDebug;
        return true;
      }
      else
      {
        Auxiliares::printMSG(_cmd + ": Comando desconhecido.", true);
        return false; // Retorna false para comando não reconhecido
      }
    }

    /// @brief Função para gerar SSID com base no IP local fixo e o nome do sketch
    /// @return char gerado para uso como SSID
    const char *gerarSSID()
    {
      static String result;
      result = rede_ip.toString() + "->" + nomeSketch;
      if (result.length() > 31)
      {
        result = result.substring(0, 31); // Trunca o resultado para 31 caracteres
      }
      result.toUpperCase();
      return result.c_str();
    }
  }

  // ****************************************************
  // ******************* EspNow *************************
  // ****************************************************
  namespace EspNow
  {
    bool EspNowHabilitado = false;         // marca como habilitado se esp_now_init() == ESP_OK
    bool EspNowIniciado = false;           // carma se a biblioteca foi iniciada corretamente
    bool EspNowEnvioOK = false;            // a mensagem foi enviada? Não indica entregue.
    bool EspNowACK = false;                // Indica se a mensagem foi recebida pelo destinatário
    const String cmdIdentificador = "CMD"; // prefixo id de comando
    const char separador = '|';            // delimitador
    struct Comando                         // Estrutura para armazenar informações dos comandos
    {
      int cmd;
      int arg1;
      int arg2;
      String argSTR;
    };

    /// @brief Função para inicializar EspNow
    /// @return true/false = Sucesso/Falha nas inicializações
    bool EspNow_init()
    {
      if (!ServidorInicializado)
      {
        Auxiliares::printDEBUG("BIBLIOTECA NÃO INICIALIZADA!");
        return false;
      }

      /*
           ESP-NOW configuração
           WiFi must be powered on to use ESP-NOW unicast.
           It could be either AP or STA mode, and does not have to be connected.
           For best results, ensure both devices are using the same WiFi channel.
           */
      if (WiFi.getMode() == WIFI_MODE_NULL)
      {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(false, true); // Convém desconectar de qualquer rede wifi antes de configurar o espnow
        Auxiliares::delayYield(100);
      }

      for (int i = 0; i < 3; i++)
      {
        if (esp_now_init() == ESP_OK) // tenta iniciar o espnow
        {
          esp_now_register_recv_cb(EspNow_CallbackReceber); // Define a função de callback para recebimento de dados
          EspNowIniciado = true;                            // define EspNow iniciado com base no resultado de init()
          WIFIradio_OFF = false;
          break;
        }

        Auxiliares::delayYield(2000);
        if (i == 2)
        { // Terceira tentativa falha
          Auxiliares::printDEBUG("FALHA AO INICIALIZAR ESP-NOW");
          EspNowIniciado = false;
          break;
          ;
        }
      }

      // Led aceso indica que EspNow está ativo; apagado indica desabilitado
      digitalWrite(SHtoolsESP32::ledPin, EspNowIniciado);

      SHtoolsESP32::EspNow::configurarEstado(EspNowIniciado); // Passar estado de espnow para arquivo auxiliar
      return EspNowIniciado;
    }

    /*
    /// @brief Função para configurar o peer
    /// @param peerInfo
    /// @param macAddress
    void EspNow_configurarPeer(esp_now_peer_info_t &peerInfo, const uint8_t *macAddress)
    {
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, macAddress, 6); // Copia o MAC do peer
      peerInfo.channel = 1;                      // Canal definido no Wi-Fi
      peerInfo.encrypt = false;                  // Criptografia desabilitada
    }
    */

    /*
    /// @brief Função para adicionar um peer no ESP-NOW
    /// @param nome Nome do peer (String)
    /// @param mac Endereço MAC do peer (array de 6 bytes)
    /// @note Esta função verifica se o peer já existe no mapa lógico e no ESP-NOW.
    ///       Se já existir, não adiciona novamente e retorna false.
    ///       Se não existir, configura o peer e adiciona tanto no ESP-NOW quanto
    ///       no mapa lógico de peers.
    /// @return true/false = Sucesso/Falha na adição do peer
    bool EspNow_adicionarPeer(const String &nome, const uint8_t mac[6])
    {
      if (!EspNowIniciado)
      {
        Auxiliares::printMSG("EspNow não não foi inicializado?", true);
        return false;
      }

      // Verifica se já está no mapa lógico
      const uint8_t *macExistente = SHtoolsESP32::EspNow::getPeer(nome);
      if (macExistente)
      {
        Auxiliares::printMSG("ERRO: Peer já existente para este nome.", true);
        return false;
      }

      // Verifica se já existe no ESP-NOW
      if (esp_now_is_peer_exist(mac))
      {
        Auxiliares::printMSG("ERRO: Peer já existente para este MAC.", true);
        return false;
      }

      // configura o peer
      esp_now_peer_info_t peerInfo;
      EspNow_configurarPeer(peerInfo, mac);

      // Adiciona o peer usando a API ESP-NOW
      if (esp_now_add_peer(&peerInfo) == ESP_OK)
      {

        // Adiciona o peer na lista lógica
        int res = SHtoolsESP32::EspNow::addPeer(nome, mac);
        if (res != 0)
        {
          Auxiliares::printMSG("Falha ao registrar peer na lista lógica.", true);
          EspNow_removerPeer(mac); // Remove o peer se falhar ao registrar na lista lógica
          return false;
        }
      }
      else
      {

        Auxiliares::printMSG("Falha ao adicionar PEER.", true);
        return false;
      }
      Auxiliares::printMSG("PEER adicionado com sucesso.", true);
      return true;
    }
    */

    /*
    /// @brief Função para remover um peer
    /// @param macAddress
    /// @return
    bool EspNow_removerPeer(const uint8_t *macAddress)
    {
      if (!EspNowIniciado)
      {
        Auxiliares::printMSG("EspNow não não foi inicializado?", true);
        return false;
      }

      if (esp_now_del_peer(macAddress) == ESP_OK)
      {
        // Obtém o número de peers emparelhados
        esp_now_peer_num_t peerNum;
        esp_now_get_peer_num(&peerNum);

        if (peerNum.total_num > 0)
        {
          EspNowHabilitado = true;
        }
        else
        {
          EspNowHabilitado = false;
        }

        Auxiliares::printMSG("PEER removido com sucesso.", true);
      }
      else
      {
        Auxiliares::printMSG("Falha ao remover PEER.", true);
        return false;
      }
      return true;
    }
  */

    /// @brief Função para criar mensagem de comando de forma padronizada
    /// @param comando Número do comando
    /// @param arg1 Argumento1 (int)
    /// @param arg2 Argumento2 (int)
    /// @param argSTR Argumento3 (Str)
    /// @return String com mensagem formatada padronizada
    String criarMSGcomando(int comando, int arg1, int arg2, const char *argSTR)
    {
      const int maxLength = (250 - 1); // máximo suportado por EspNow (250). Menos 1 para o terminador nulo '\0'
      char tempMsg[250];

      snprintf(tempMsg, sizeof(tempMsg), "%s%c%d%c%d%c%d%c%s",
               cmdIdentificador.c_str(), separador,
               comando, separador, arg1, separador, arg2, separador, argSTR);

      int totalLength = strlen(tempMsg);

      // se a mensagem ficou maior do que o tamanho máximo de pacote suportado
      if (totalLength > maxLength)
      {
        int extraLength = totalLength - maxLength; // obtem o tamanho que ultrapassou o máximo
        int argSTRLength = strlen(argSTR);         // obtem o tamanho do argumento string

        // Calcula o novo comprimento de argSTR removendo o excesso
        int newArgSTRLength = argSTRLength - extraLength;
        if (newArgSTRLength < 0)
        {
          newArgSTRLength = 0;
        }

        // Cria uma nova string para argSTR cortada
        char truncatedArgSTR[newArgSTRLength + 1];
        strncpy(truncatedArgSTR, argSTR, newArgSTRLength);
        truncatedArgSTR[newArgSTRLength] = '\0';

        // Formata novamente a mensagem com o argSTR cortado
        snprintf(tempMsg, maxLength + 1, "%s%c%d%c%d%c%d%c%s", cmdIdentificador.c_str(),
                 separador, comando, separador, arg1, separador, arg2,
                 separador, truncatedArgSTR);
      }

      return String(tempMsg);
    }

    /// @brief Função para enviar dados ao peer, com timeout e retorno de sucesso ou falha
    /// @param peer mac do peer
    /// @param msg mensagem padronizada a ser enviada
    /// @param ACK_timeout_ms timeout com milisegundos
    /// @return true/false = Sucesso/Falha de ENVIO
    bool EspNow_EnviarDados(uint8_t *peer, String msg, unsigned short ACK_timeout_ms)
    {
      // msg.c_str(): Converte a String para const char*.
      // reinterpret_cast<const uint8_t*>: Converte o const char* para const uint8_t*, que é o formato esperado por esp_now_send.
      const uint8_t *data = reinterpret_cast<const uint8_t *>(msg.c_str());

      // Reseta as flags de status
      EspNowEnvioOK = false;
      EspNowACK = false; // alterada no callback de recebimento

      // Envia os dados via ESP-NOW
      esp_err_t result = esp_now_send(peer, data, msg.length());

      if (result != ESP_OK)
      {
        Auxiliares::printDEBUG("ERRO IMEDIATO AO TENTAR ENVIAR DADOS");
        return false;
      }

      EspNowEnvioOK = true;
      return EspNowEnvioOK;
    }

    /// @brief Função de callback para receber os dados enviados pelo peer
    /// @param peer mac do peer que enviou a mensagem
    /// @param incomingData dados recebidos
    /// @param len tamanho dos dados recebidos
    void EspNow_CallbackReceber(const uint8_t *peer, const uint8_t *incomingData, int len)
    {
      /*
      assinatura de comando|comando|arg1|arg2|argSTR
      valores -1 ou "" serão ignorados
      */

      // Verifica se o pacote recebido é "ACK"
      if (len == 3 && memcmp(incomingData, "ACK", 3) == 0)
      {
        EspNowACK = true;
        Auxiliares::printDEBUG("ACK RECEBIDO.");
        return;
      }

      // envia ACK para confirmar o recebimento do pacote
      esp_now_send(peer, (const uint8_t *)"ACK", 3);

      char msgRecebida[250];
      // Verifica se o comprimento da mensagem recebida é menor que o tamanho da variável 'msgRecebida'
      // espnow suporta pacotes de no máximo 250 bytes
      if (len < sizeof(msgRecebida))
      {
        memcpy(msgRecebida, incomingData, len); // Copia a mensagem recebida (armazenada em 'data') para a variável 'msgRecebida'
        msgRecebida[len] = '\0';                // Adiciona o caractere de terminação no final da mensagem recebida para garantir que ela seja uma string válida em C++
      }
      else
      {
        Auxiliares::printDEBUG("ERRO: O PACOTE DE DADOS RECEBIDO É MUITO GRANDE.");
        return;
      }

      Auxiliares::printDEBUG("PACOTE RECEBIDO: " + String(msgRecebida));

      // resultados do comando processado
      switch (processarComando(msgRecebida))
      {
      case -4:
        Auxiliares::printDEBUG("NUMERO INSUFICIENTE DE ARGUMENTOS. (-4)");
        break;
      case -3:
        Auxiliares::printDEBUG("POSSÍVEL ERRO - COMANDO 0. (-3)");
        break;
      case -2:
        Auxiliares::printDEBUG("SEM PREFIXO IDENTIFICADOR DE COMANDO. (-2)");
        break;
      case -1:
        Auxiliares::printDEBUG("O FORMATO DA MENSAGEM DE COMANDO É INVÁLIDO. (-1)");
        break;

      case 0:
        Auxiliares::printDEBUG("A EXECUÇÃO DO COMANDO FALHOU! (0)");
        break;

      case 1:
        Auxiliares::printDEBUG("COMANDO EXECUTADO COM SUCESSO! (1)");
        break;

      default:
        break;
      }
    }

    /// @brief Função para processar o comando recebido
    /// @param msgRecebida mensagem de comando recebida
    /// @return inteiro que representa uma resposta ao processamento
    int processarComando(const char *msgRecebida)
    {
      /*
      Formato esperado: CMD|cmd|arg1|arg2|argSTR
      */

      String msgString(msgRecebida);

      // Verifica se há o identificador de comando no início da mensagem
      if (msgString.substring(0, cmdIdentificador.length()) == cmdIdentificador)
      {
        // Verifica se após o identificador de comando há o caractere separador
        if (msgString.charAt(cmdIdentificador.length()) == separador)
        {
          String restante = msgString.substring(cmdIdentificador.length() + 1);

          // Divide a mensagem usando o separador
          std::vector<String> partes;
          dividirString(restante, separador, partes);

          // A mensagem deve ter pelo menos 4 partes
          if (partes.size() < 4)
          {
            SHtoolsESP32::Auxiliares::printDEBUG("ERRO -4: " + String(partes.size()));
            return -4; // Erro: número insuficiente de argumentos
          }

          // Preenche os dados do comando
          Comando comando;
          comando.cmd = partes[1].toInt();
          comando.arg1 = partes[2].toInt();
          comando.arg2 = partes[3].toInt();
          comando.argSTR = partes[4];

          // Logs de depuração dos argumentos parseados
          SHtoolsESP32::Auxiliares::printDEBUG("CMD: " + String(comando.cmd));
          SHtoolsESP32::Auxiliares::printDEBUG("ARG1: " + String(comando.arg1));
          SHtoolsESP32::Auxiliares::printDEBUG("ARG2: " + String(comando.arg2));
          SHtoolsESP32::Auxiliares::printDEBUG("ARGSTR: " + String(comando.argSTR));

          // Verifica cmd inválido 0 (possível erro de parsing)
          if (comando.cmd == 0)
          {
            return -3; // Erro: cmd zero reservado
          }

          // Chama o router e verifica execução real do handler
          bool sucessoExecucao = cmd_rotas::route(comando.cmd, comando.arg1, comando.arg2, comando.argSTR);

          if (sucessoExecucao)
          {
            return 1; // Sucesso: handler executado corretamente
          }
          else
          {
            return 0; // Falha: handler inexistente ou execução retornou erro
          }
        }
        else
        {
          return -1; // Erro: identificador encontrado mas sem separador
        }
      }
      else
      {
        return -2; // Erro: sem identificador de comando
      }
    }

    /// @brief Função auxiliar a processarComando() para dividir a string e organizar as partes
    /// @param str mensagem a ser dividida
    /// @param sep separador utilizado
    /// @param partes partes a serem organizadas
    void dividirString(const String &str, char sep, std::vector<String> &partes)
    {
      int startPos = 0;
      int pos;

      while ((pos = str.indexOf(sep, startPos)) != -1)
      {
        partes.push_back(str.substring(startPos, pos));
        startPos = pos + 1;
      }
      partes.push_back(str.substring(startPos)); // Adiciona a última parte
    }
  }

  // ****************************************************
  // ***************** Auxiliares ***********************
  // ****************************************************
  namespace Auxiliares
  {
    /// @brief Função para manipular <preferences.h> em escrita,leitura e consulta
    /// @param _opcao -1 = monta _config se não estiver montado,
    /// 0 = cria ou obtem o valor,
    /// 1 = atualiza valor,
    /// 2 = obtém valor
    /// @param _chave Máximo 15 caracteres (14 + 1 terminador nulo '\0')
    /// @param _valor valor a ser
    /// @return bool
    bool preferencias(int8_t _opcao, const char *_chave, bool _valor)
    {
      config.begin(PrefNameSpace_config, false); // Abre o Preferences para escrita
      bool valorRetorno = false;
      switch (_opcao)
      {
      case -1: // monta _config se não estiver montado
        if (!config.isKey(PrefKey_configOK))
        {
          config.putBool(PrefKey_serverMod_auto, false);
          config.putBool(PrefKey_novoFirmware, false);
          config.putBool(PrefKey_testeNovoFirmware, false);
          config.putBool(PrefKey_fezRollback, false);
          config.putBool(PrefKey_configOK, true);
        }
        break;

      case 0: // se não exisitir a chave, cria. Se existir, obtem o valor
        if (!config.isKey(_chave))
        {
          config.putBool(_chave, _valor);
          valorRetorno = _valor;
        }
        else
        {
          valorRetorno = config.getBool(_chave);
        }
        break;

      case 1: // atualiza valor
        config.putBool(_chave, _valor);
        valorRetorno = _valor;
        break;

      case 2: // obtem valor
        valorRetorno = config.getBool(_chave);
        break;

      default:
        break;
      }
      config.end();        // Fecha o Preferences após a gravação
      return valorRetorno; // Atualiza a variável interna
    }

    /// @brief Função para imprimir as informações em serial monitor local e remoto
    /// @param _msg mensagem a ser impressa
    /// @param newline imprimir em nova linha? (apenas local)
    /// @param _debug é mensagem de debug?
    void printMSG(const String &_msg, bool newline, bool _debug)
    {
      String msg = _msg;

      // controle de imprimir mensagens de debug
      if (_debug)
      {
        if (HabilitarDebug)
        {
          msg = "DEBUG >>> " + msg;
          msg.toUpperCase();
        }
        else
        {
          return;
        }
      }

      // Verifica se a mensagem recebida é um comando (case insensitive)
      if (msg.substring(0, 4).equalsIgnoreCase("cmd:"))
      {
        msg = msg + ":resultado=" + String((Servidor::SerialCMD(msg.substring(4)) ? 1 : 0));
      }

      if (Servidor::ServerMod_ON)
        Servidor::ws.textAll(msg); // Enviar para o WebSocket (serial remoto)
                                   // não obedece "newline = false", cada envio é escrito em uma nova linha

      if (newline)
        Serial.println(msg); // Envia com nova linha
      else
        Serial.print(msg); // Envia sem nova linha

      if (Servidor::restartSolicitado)
      {
        ReiniciarESP();
      }
    }

    /// @brief Função para imprimir mensagens exclusivamente de debug
    /// @param _msg mensagem a ser impressa
    void printDEBUG(String _msg)
    {
      if (HabilitarDebug)
      {
        printMSG(_msg, true, true);
      }
    }

    /// @brief Função para verificar o tempo de atividade do sistema e reiniciar se necessário
    /// @param momentoInicial tempo inicial do sistema em milissegundos
    /// Esta função verifica se o sistema está ativo por mais de uma semana ou se o ESP-NOW está desabilitado por mais de 12 horas.
    /// Se qualquer uma dessas condições for verdadeira, o sistema é reiniciado.
    /// @note Esta função deve ser chamada periodicamente no loop principal do programa.
    /// @warning Certifique-se de que o momentoInicial seja definido corretamente no início do programa

    void verificadorGenerico(unsigned long momentoInicial)
    {
      const unsigned long UMA_SEMANA_MS = 7UL * 24 * 60 * 60 * 1000; // 7 dias em milissegundos
      const unsigned long DOZE_HORAS_MS = 12UL * 60 * 60 * 1000;     // 12 horas em milissegundos

      static unsigned long espNowUltimaAtividade = millis();
      unsigned long currentMillis = millis();

      // 1 SEMANA - REINICIAR O SISTEMA
      if ((currentMillis - momentoInicial) >= UMA_SEMANA_MS)
      {
        printMSG("Reiniciando o sistema após uma semana de operações contínuas.", true);
        ReiniciarESP();
      }

      // 12 HORAS - REINICIAR O SISTEMA SE ESPNOW ESTIVER OFFLINE
      if (!SHtoolsESP32::EspNow::EspNowHabilitado)
      {
        if ((currentMillis - espNowUltimaAtividade) >= DOZE_HORAS_MS)
        {
          printMSG("Reiniciando o sistema após 12 horas devido EspNow estar desabilitado.", true);
          delayYield(1000);
          ReiniciarESP(4000);
        }
      }
      else
      {
        espNowUltimaAtividade = currentMillis; // renova quando EspNow está habilitado
      }
    }

    /// @brief Função para delay "assincrono" com uso de Yield()
    /// @param ms tempo de delay em milisegundos
    void delayYield(unsigned long ms)
    {
      unsigned long start = millis();
      while ((unsigned long)millis() - start < ms)
      {
        yield();
      }
    }

    /// @brief Função para restart limpo com finalização prévia de recusroso ativos
    /// @param _tempoDelay tempo de delay antes do restart em milisegundos
    void ReiniciarESP(int _tempoDelay)
    {
      if (!WIFIradio_OFF)
      {
        esp_now_deinit();
      }

      Servidor::ws.closeAll();     // Fecha todas as conexões WebSocket, ignorando erros
      delayYield(50);              // delay de segurança
      Servidor::server.end();      // Para o servidor, ignorando erros
      delayYield(50);              // delay de segurança
      WiFi.disconnect(true, true); // Desconecta o WiFi, ignorando erros
      delayYield(50);              // delay de segurança
      Serial.flush();              // Garante que todos os dados da Serial sejam enviados
      delayYield(_tempoDelay);     // Atraso opcional antes do reinício
      ESP.restart();               // Reinicia o ESP
    }

  }
}
