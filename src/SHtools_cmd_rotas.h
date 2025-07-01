// src/SHtools_cmd_rotas.h

#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <Arduino.h>
// #include "SHtoolsESP32.h" // Para Auxiliares::printDEBUG
#include "SHtools_cmd.h" // Importa a lista oficial de comandos

class cmd_rotas
{
public:
    // Tipo da função que cada handler deve ter: executa comando com seus parâmetros
    // Agora retorna bool: true = ação concluída com sucesso; false = falha na execução
    using Handler = std::function<bool(int arg1, int arg2, String argStr)>;

    // Registra um handler para o cmdId. Só permite se cmdId existir no catálogo central
    static int addHandler(int cmdId, Handler handler)
    {
        if (cmdIdValido(cmdId))
        {
            _handlers[cmdId] = handler; // associa cmdId ao handler fornecido
            return 0;                   // registro bem-sucedido
        }
        else
        {
            // SHtoolsESP32::Auxiliares::printDEBUG("[ERRO] Tentativa de registrar cmdId inexistente no catálogo: " + String(cmdId));
            return -1; // falha: cmdId não existe no catálogo
        }
    }

    // Executa o handler correspondente ao cmdId, se registrado
    // Retorna true apenas se o handler existir e confirmar sucesso na execução
    static bool route(int cmdId, int arg1, int arg2, String argStr)
    {
        auto it = _handlers.find(cmdId);
        if (it != _handlers.end())
        {
            bool sucesso = it->second(arg1, arg2, argStr); // chama handler e obtém retorno
            return sucesso;                                // retorna resultado da execução real
        }
        else
        {
            // SHtoolsESP32::Auxiliares::printDEBUG("[ERRO] Nenhum handler registrado para cmdId: " + String(cmdId));
            return false; // falha: handler inexistente
        }
    }

    // Retorna uma lista com os cmdIds atualmente registrados neste dispositivo
    // Útil para relatórios ou enviar lista de comandos suportados em tempo de execução
    static std::vector<int> listarComandosRegistrados()
    {
        std::vector<int> lista;
        for (const auto &par : _handlers)
        {
            lista.push_back(par.first); // adiciona cmdId registrado
        }
        return lista;
    }

private:
    static std::unordered_map<int, Handler> _handlers; // Mapa cmdId → handler registrado

    // Verifica se o cmdId informado existe no catálogo de comandos disponíveis
    static bool cmdIdValido(int cmdId)
    {
        for (size_t i = 0; i < QNT_COMANDOS_DISPONIVEIS; ++i)
        {
            if (ComandosDisponiveis[i].cmdId == cmdId)
                return true;
        }
        return false; // cmdId não encontrado no catálogo
    }
};
