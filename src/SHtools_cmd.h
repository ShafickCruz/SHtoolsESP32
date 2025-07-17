// src/SHtools_cmd.h

#pragma once
#include <cstddef>

struct cmd
{
    int cmdId;             // ID numérico único do comando
    const char *nome;      // Nome do comando
    const char *descricao; // Descrição do que o comando faz
};

// Comandos permitidos no sistema:
const cmd ComandosDisponiveis[] = {
    //{0, <- comando zero é reservado e não deve ser usado},
    // BANHEIRA
    {1, "toBANHEIRA_SOLICITAR_ESTADO_LED", "Para solicitar o estado atual do LED da banheira"},
    {2, "toBANHEIRA_ALTERAR_ESTADO_LED", "Para solicitar alteração do estado do LED da banheira"},
    {3, "fromBANHEIRA_ESTADO_LED_ATUALIZADO", "Banheira informa o estado atual do LED"},
    // REGAJARDIM
    {4, "toREGAJARDIM_BACKEND_REGA1_ALTERAR_ESTADO", "Solicita alternância da rega 1 e envia tempo do ciclo"},
    {5, "toREGAJARDIM_BACKEND_REGA2_ALTERAR_ESTADO", "Solicita alternância da rega 2 e envia tempo do ciclo"},
    {6, "toREGAJARDIM_BACKEND_BOMBAFILTRO_ALTERAR_ESTADO", "Solicita alternância da bomba filtro e envia tempo do ciclo"},
    {7, "toREGAJARDIM_BACKEND_SOLICITAR_STATUS", "Solicita o status atual dos recursos"},
    {8, "fromREGAJARDIM_BACKEND_STATUS", "Informa o status atual dos recursos"}};

// Quantidade de comandos definidos, útil para iteração:
const size_t QNT_COMANDOS_DISPONIVEIS = sizeof(ComandosDisponiveis) / sizeof(cmd);
