#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

// ==================================================================
//                     Mensagens Auxiliares
// ==================================================================

#define AUX_INVOKE_GET "%s - %s: \n"

// Metodos thread-safe para imprimir

/**
 * Incrementa o numero de clientes conectados.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int inc_num_clients();

/**
 * Decrementa o numero de clientes conectados.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int dec_num_clients();

/**
 * Retorna o numero de clientes conectados.
 * \return 
 *      Numero de clientes conectados ou -1 em caso de erro.
*/
int get_num_clients();

/**
 * Retorna o numero de operacoes realizadas sobre a tabela.
 * \return 
 *      Numero de operacoes realizadas ou -1 em caso de erro.
*/
int get_num_ops();

/**
 * Retorna o tempo que demorou para realizar as operacoes.
 * \return 
 *      Tempo gasto ou -1 em caso de erro.
*/
int get_time_used();
#endif