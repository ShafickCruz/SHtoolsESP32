// src/SHtools_cmd_rotas.cpp

#include "SHtools_cmd_rotas.h"

// Inicializa o mapa de handlers como variável estática
std::unordered_map<int, cmd_rotas::Handler> cmd_rotas::_handlers;
