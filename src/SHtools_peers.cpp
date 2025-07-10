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

        // ****************************************************
        // ****************** FUNÇÕES PÚBLICAS ****************
        // ****************************************************

        void configurarEstado(bool iniciado)
        {
            _espnowIniciado = iniciado;
        }

        int registrar(const String &nome, const uint8_t mac[6])
        {
            if (!_espnowIniciado)
            {
                Auxiliares::printDEBUG("ESP-NOW não iniciado. Não é possível registrar peer.");
                return -3;
            }

            if (_peers.find(nome) != _peers.end())
            {
                Auxiliares::printDEBUG("Peer já registrado no mapa lógico: " + nome);
                return -1;
            }

            if (esp_now_is_peer_exist(mac))
            {
                Auxiliares::printDEBUG("Peer já registrado no ESP-NOW (MAC duplicado).");
                return -1;
            }

            esp_now_peer_info_t peerInfo;
            configurarPeerInfo(peerInfo, mac);

            if (esp_now_add_peer(&peerInfo) != ESP_OK)
            {
                Auxiliares::printDEBUG("Falha ao adicionar peer no ESP-NOW.");
                return -2;
            }

            std::array<uint8_t, 6> macArr;
            memcpy(macArr.data(), mac, 6);
            _peers[nome] = macArr;

            Auxiliares::printDEBUG("Peer registrado com sucesso: " + nome);
            return 0;
        }

        bool remover(const String &nome)
        {
            auto it = _peers.find(nome);
            if (it == _peers.end())
                return false;

            if (esp_now_del_peer(it->second.data()) != ESP_OK)
                return false;

            _peers.erase(it);
            return true;
        }

        bool existe(const String &nome)
        {
            return _peers.find(nome) != _peers.end();
        }

        const uint8_t *getMAC(const String &nome)
        {
            auto it = _peers.find(nome);
            if (it == _peers.end())
                return nullptr;

            return it->second.data();
        }

        String listarJSON()
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
