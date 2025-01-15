/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#ifndef _STATS_H
#define _STATS_H

#include "synchronization.h"
#include <pthread.h>

/**
 * Define o tipo statistics_t e as operacoes thread-safe que 
 * podem ser realizadas sobre ela.
*/

typedef struct statistics_t {
    // Dados
    int n_op;           /* n operacoes realizadas */
    long time_lasted;   /* tempo total demorou nas operacoes */
    int n_client;       /* n clientes conectados */
    // Controlo da concorrencia
    rwcctrl_t *cctrl;   /* controlo de concorrencia de leitura e escrita */
} stats_t;

// =========================================================
//                  Operacoes de escrita
// =========================================================

/**
 * Cria uma estrutura stats_t e inicializa-a com valores
 * todos a 0.
 * \return
 *      Apontador a estrutura ou NULL em caso de erro.
*/
stats_t *stats_init();

/**
 * Cria uma estrutura stats_t e inicializa-a com valores
 * passados.
 * \param op
 *      Numero de operacoes realizadas.
 * \param time
 *      Tempo total gasto na realizacao das operacoes.
 * \param client
 *      Numero de clientes conectados.
 * \return
 *      Apontador a estrutura ou NULL em caso de erro.
*/
stats_t *stats_init_args(int op, long time, int client);

/**
 * Incrementa o numero de operacoes
 * realizadas sobre a tabela.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura sobre qual fazer o incremento.
 * \return
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_inc_op(stats_t *stats);

/**
 * Adiciona o tempo gasto nas operacoes.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura sobre qual fazer a adicao do tempo.
 * \param time
 *      Tempo a ser adicionado ao existente.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_add_time(stats_t *stats, long time);

/**
 * Incrementa o numero de clientes conectados.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura sobre qual incrementar o numero.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_inc_client(stats_t *stats);

/**
 * Decrementa o numero de clientes conectados.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura sobre qual decrementar o numero.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_dec_client(stats_t *stats);

/**
 * Regista o fim da execucao de uma operacao,
 * incrementando o n_op e adiciona ao time_lasted
 * o tempo passado como argumento.
 * \attention
 *      Thread-safe
 * \param stats
 *      Estrutura sobre qual realizar a alteracao.
 * \param time
 *      Tempo a ser adicionado.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_op_finish(stats_t *stats, long time);

/**
 * Duplica a estrutura e o seu conteúdo, fazendo
 * uma cópia profunda do objeto.
 * \attention
 *      Thread-safe
 * \param stats
 *      Estrutura para ser duplicada.
 * \return
 *      Apontador a nova estrutura ou NULL em caso 
 *      de erro.
*/
stats_t *stats_dup(stats_t *stats);

/**
 * Destroi a estrutura, libertando todos os recursos.
 * \param stats
 *      Estrutura para ser destruida.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int stats_destroy(stats_t *stats);

// =========================================================
//                  Operacoes de leitura
// =========================================================

/**
 * Retorna o numero de operacoes realizadas
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura stats_t.
 * \return
 *      Numero de operacoes realizadas sobre a tabela,
 *      -1 em caso de erro.
*/
int stats_get_n_op(stats_t *stats);

/**
 * Retorna o tempo total gasto nas operacoes da tabela em
 * microsegundos.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura stats_t.
 * \return
 *      Tempo em microsegundos, -1 em caso de erro.
*/
long stats_get_time_lasted(stats_t *stats);

/**
 * Retorna o numero de clientes conectados ao servidor.
 * \attention
 *      Thread-safe.
 * \param stats
 *      Estrutura stats_t.
 * \return
 *      Numero de clientes conectados, -1 em caso de erro.
*/
int stats_get_n_client(stats_t *stats);

#endif