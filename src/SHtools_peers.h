/*
 * SHtools_peers.h
 * Interface pública para gerenciamento de peers EspNow centralizado
 */

#ifndef SHTOOLS_PEERS_H
#define SHTOOLS_PEERS_H

#include <Arduino.h>

namespace SHtoolsESP32
{
    namespace EspNow
    {
        /// @brief Informa ao módulo se EspNow foi iniciado
        void configurarEstado(bool iniciado);

        /// @brief Registra peer no mapa lógico e no hardware
        /// @param nome Nome identificador lógico do peer
        /// @param mac Endereço MAC do peer (6 bytes)
        /// @return 0 = sucesso, -1 = já existe, -2 = erro HW, -3 = não iniciado
        int registrar(const String &nome, const uint8_t mac[6]);

        /// @brief Remove peer do mapa lógico e do hardware
        /// @param nome Nome do peer a remover
        /// @return true = removido, false = erro ou inexistente
        bool remover(const String &nome);

        /// @brief Verifica se o peer existe no mapa lógico
        /// @param nome Nome a consultar
        /// @return true/false
        bool existe(const String &nome);

        /// @brief Obtém ponteiro para MAC registrado no mapa lógico
        /// @param nome Nome do peer
        /// @return ponteiro para array[6] ou nullptr se não existir
        const uint8_t *getMAC(const String &nome);

        /// @brief Lista todos os peers em formato JSON
        /// @return String JSON
        String listarJSON();
    }
}

#endif
