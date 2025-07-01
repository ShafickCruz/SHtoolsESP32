// src/SHtools_cmd.h

#pragma once
#include <cstddef>

struct cmd
{
    int cmdId;             // ID numérico único do comando
    int executar;          // padrão de executar (0/1) esperado para o comando
    const char *nome;      // Nome do comando
    const char *descricao; // Descrição do que o comando faz
};

// Aqui você define todos os comandos permitidos no sistema:
const cmd ComandosDisponiveis[] = {
    {1, 1, "REGA1_ONOFF", "Liga ou desliga a rega 1"},
    {2, 1, "REGA2_ONOFF", "Liga ou desliga a rega 2"},
    {3, 1, "BOMBA_ONOFF", "Liga ou desliga a bomba do lago"},
    // Adicione novos comandos aqui conforme for expandindo o sistema
};

// Quantidade de comandos definidos, útil para iteração:
const size_t QNT_COMANDOS_DISPONIVEIS = sizeof(ComandosDisponiveis) / sizeof(cmd);
