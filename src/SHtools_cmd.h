// src/SHtools_cmd.h

#pragma once
#include <cstddef>

struct cmd
{
    int cmdId;             // ID numérico único do comando
    const char *nome;      // Nome do comando
    const char *descricao; // Descrição do que o comando faz
};

// Aqui você define todos os comandos permitidos no sistema:
const cmd ComandosDisponiveis[] = {
    //{0, <- comando zero é reservado e não deve ser usado},
    {1, "toBANHEIRA_SOLICITAR_ESTADO_LED", "Para solicitar o estado atual do LED da banheira"},
    {2, "toBANHEIRA_ALTERAR_ESTADO_LED", "Para solicitar alteração do estado do LED da banheira"},
    {3, "fromBANHEIRA_ESTADO_LED_ATUALIZADO", "Banheira informa o estado atual do LED"},
    {1, "REGA1_ONOFF", "Liga ou desliga a rega 1"},
    {2, "REGA2_ONOFF", "Liga ou desliga a rega 2"},
    {3, "BOMBA_ONOFF", "Liga ou desliga a bomba do lago"},
    // Adicione novos comandos aqui conforme for expandindo o sistema
};

// Quantidade de comandos definidos, útil para iteração:
const size_t QNT_COMANDOS_DISPONIVEIS = sizeof(ComandosDisponiveis) / sizeof(cmd);
