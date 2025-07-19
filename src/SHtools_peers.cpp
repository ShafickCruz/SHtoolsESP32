/*
 * SHtools_peers.cpp
 * Implementação do gerenciamento de peers EspNow centralizado
 */

#include "SHtools_peers.h"
#include "SHtoolsESP32.h"
#include <unordered_map>
#include <array>
#include <map>
#include <esp_now.h>

namespace SHtoolsESP32
{
    namespace EspNow
    {

        // ****************************************************
        // ****************** VARIÁVEIS INTERNAS ***************
        // ****************************************************

        static std::map<String, std::array<uint8_t, 6>> _peers;
        static bool _espnowIniciado = false;

        // ****************************************************
        // ****************** FUNÇÕES INTERNAS ****************
        // ****************************************************

        /// @brief Preenche estrutura peerInfo
        void configurarPeerInfo(esp_now_peer_info_t &info, const uint8_t mac[6])
        {
            memset(&info, 0, sizeof(info));
            memcpy(info.peer_addr, mac, 6);
            info.channel = 1;
            info.encrypt = false;
        }

        void configurarEstado(bool iniciado)
        {
            _espnowIniciado = iniciado;
        }

        // ****************************************************
        // ****************** FUNÇÕES PÚBLICAS ****************
        // ****************************************************

        /**
         * @brief Inicializa o EspNow e configura o callback de recebimento
         * @param nome Nome do peer a ser registrado
         * @param mac Endereço MAC do peer a ser registrado
         * @details Esta função registra um novo peer no ESP-NOW.
         * @note O ESP-NOW deve estar iniciado antes de registrar um peer.
         * @note O peer deve ser adicionado no mapa lógico antes de enviar dados.
         * @return 1 sucesso, 0 falha ao adicionar no ESP-NOW, -1 ESP-NOW não iniciado, -2 peer já existe no mapa lógico, -3 peer já existe no ESP-NOW
         */
        int registrarPeer(const String &nome, const uint8_t mac[6])
        {
            if (!_espnowIniciado)
            {
                Auxiliares::printDEBUG("ESP-NOW NÃO INICIADO. NÃO É POSSÍVEL REGISTRAR PEER.");
                return -1; // erro: espnow não iniciado
            }

            if (_peers.find(nome) != _peers.end())
            {
                Auxiliares::printDEBUG("PEER JÁ REGISTRADO NO MAPA LÓGICO: " + nome);
                return -2; // erro: peer já existe
            }

            if (esp_now_is_peer_exist(mac))
            {
                Auxiliares::printDEBUG("PEER JÁ REGISTRADO NO ESP-NOW (MAC DUPLICADO).");
                return -3; // erro: peer já existe no esp_now
            }

            esp_now_peer_info_t peerInfo;
            configurarPeerInfo(peerInfo, mac);

            if (esp_now_add_peer(&peerInfo) != ESP_OK)
            {
                Auxiliares::printDEBUG("FALHA AO ADICIONAR PEER NO ESP-NOW.");
                return 0; // erro: falha ao adicionar peer no esp_now
            }

            std::array<uint8_t, 6> macArr;
            memcpy(macArr.data(), mac, 6);
            _peers[nome] = macArr;

            Auxiliares::printDEBUG("PEER REGISTRADO COM SUCESSO: " + nome);
            return 1; // sucesso
        }

        bool removerPeer(const String &nome)
        {
            auto it = _peers.find(nome);
            if (it == _peers.end())
                return false;

            if (esp_now_del_peer(it->second.data()) != ESP_OK)
                return false;

            _peers.erase(it);
            return true;
        }

        bool existePeer(const String &nome)
        {
            return _peers.find(nome) != _peers.end();
        }

        const uint8_t *getMAC_Peer(const String &nome)
        {
            auto it = _peers.find(nome);
            if (it == _peers.end())
                return nullptr;

            return it->second.data();
        }

        String listarPeer_JSON()
        {
            String json = "[";
            bool primeiro = true;

            for (const auto &par : _peers)
            {
                if (!primeiro)
                    json += ",";
                char macStr[18];
                sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                        par.second[0], par.second[1], par.second[2],
                        par.second[3], par.second[4], par.second[5]);
                json += "{\"nome\":\"" + par.first + "\",\"mac\":\"" + String(macStr) + "\"}";
                primeiro = false;
            }

            json += "]";
            return json;
        }

    } // namespace EspNow
} // namespace SHtoolsESP32
