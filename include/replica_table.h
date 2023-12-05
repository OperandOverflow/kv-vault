/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

/**
 * Módulo que define tipos e códigos que possam ser 
 * usados em diferentes implementações de tabelas 
 * replicadas
*/

#ifndef _REPLICA_TABLE_H
#define _REPLICA_TABLE_H

#include "data.h"
#include "entry.h"
#include "stats.h"
#include "client_stub-private.h"

#include <zookeeper/zookeeper.h>

// =========================================================
//                   Funcoes call-back
// =========================================================

/**
 * Funcao que deve ser implementada pelo cliente/servidor
 * para tratar da falha na tabela replicada.
*/
typedef void (*failure_handler)(int errcode);


// =========================================================
//                   Constantes globais
// =========================================================

#define RPTABLE_ZK_DEFAULT_SOCKET "127.0.0.1:2181"
#define RPTABLE_ZK_ROOT_PATH "/chain"
#define RPTABLE_ZK_NODE_PREFIX "server"
#define RPTABLE_ZK_DEFAULT_TIMEOUT 2000

enum RPTABLE_ERROR {
    ZKCONNECTION_LOST           = -1,
    RPTABLE_INVALID_ARG         = -2,
    RPTABLE_CONNECTION_FAILED   = -3
};

#endif