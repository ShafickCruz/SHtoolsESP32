
#include <SHtoolsESP32.h>

#define PIN_LED LED_BUILTIN
#define PIN_UPDATE GPIO_NUM_32 // Botão de setup

// Definições do peer
static uint8_t PEER_mac[]{0x34, 0x86, 0x5D, 0x3A, 0x3C, 0x70};
static String PEER_nome = "BACKEND";

void setup()
{
    SHtoolsESP32::HabilitarDebug = true; // exibir mensagens definidas como debug?

    pinMode(PIN_UPDATE, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    SHtoolsESP32::setup(PIN_LED, PIN_UPDATE, "frontend"); // Inicializa a biblioteca

    // Registra comando 8 (receber status do backend)
    SHtoolsESP32::registrarComando(8, [](int estadoRegas, int estadoBomba, String temposStr) -> bool
                                   {
        SHtoolsESP32::Auxiliares::printDEBUG("STATUS RECEBIDO DO BACKEND");
        Serial.print("Estado Regas: ");
        Serial.println(estadoRegas);
        Serial.print("Estado Bomba: ");
        Serial.println(estadoBomba);
        Serial.print("Tempos: ");
        Serial.println(temposStr);
        return true; });

    // inicializa EspNow e adiciona o peer
    if (SHtoolsESP32::EspNow::EspNow_init())
    {
        int res = SHtoolsESP32::EspNow::registrarPeer(PEER_nome, PEER_mac);

        if (res == 0)
        {
            // Sucesso
        }
        else if (res == -1)
        {
            // Peer já existe (nome ou MAC duplicado)
        }
        else if (res == -2)
        {
            // Falha ao adicionar no ESP-NOW (erro HW)
        }
        else if (res == -3)
        {
            // EspNow não iniciado (erro interno)
        }
    }

    // Led aceso indica que EspNow está ativo; apagado indica desabilitado
    digitalWrite(PIN_LED, SHtoolsESP32::EspNow::EspNowHabilitado);

    // Exemplo de envio de comando 4 (liga Rega1 por 180 segundos)
    String mensagem = SHtoolsESP32::EspNow::criarMSGcomando(4, 180, -1, "");
    SHtoolsESP32::EspNow::EspNow_EnviarDados(PEER_nome, mensagem);
}

void loop()
{
    SHtoolsESP32::Servidor::loop();
}
