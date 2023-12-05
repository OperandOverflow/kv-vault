/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#ifndef _REPLICA_CLIENT_TABLE_H
#define _REPLICA_CLIENT_TABLE_H

#include "data.h"
#include "entry.h"
#include "stats.h"
#include "zk_adaptor.h"
#include "replica_table.h"
#include "client_stub-private.h"

#include <zookeeper/zookeeper.h>

/**
 * Estrutura que contem dados para fazer comunicacao
 * com o ZooKeeper e invocar metodos sobre a tabela remota.
*/
typedef struct client_rptable_t {
    zhandle_t *handler;

    char *rptable_wsocket;
    struct rtable_t *rtable_w;

    char *rptable_rsocket;
    struct rtable_t *rtable_r;
} c_rptable_t;

/**
 * Funcao que deve ser implementada pelo cliente,
 * apenas deve retornar o objeto.
*/
typedef c_rptable_t *(*node_watcher)();


// =========================================================
//                        Funcoes
// =========================================================

/**
 * Estabelece ligacao as tabelas replicadas, usando o socket
 * do ZooKeeper de omissao.
 * \param watcher
 *      Funcao que escuta dos eventos 
 * \param handler
 *      Funcao que faz o tratamento da falha da 
 *      tabela replicada.RPTABLE_ZK_ROOT_PATH
 * \return
 *      Apontador a s_rptable_t ou NULL em caso de erro.
*/
c_rptable_t *rptable_connect(node_watcher watcher, failure_handler handler);

/**
 * Estabelece ligacao as tabelas replicadas, especificando o 
 * socket do servidor ZooKeeper.
 * \param zksock
 *      String que descreve o socket do servidor ZooKeeper.
 * \param watcher
 *      Funcao que escuta dos eventos 
 * \param handler
 *      Funcao que faz o tratamento da falha da 
 *      tabela replicada.
 * \return
 *      Apontador a s_rptable_t ou NULL em caso de erro.
*/
c_rptable_t *rptable_connect_zksock(char* zksock, node_watcher watcher, failure_handler handler);

/**
 * Desliga a ligacao com a tabela replicada e liberta 
 * a estrutura.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int rptable_disconnect(c_rptable_t *rptable);

/** 
 * Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * \param rptable
 * \param key
 * \param data
 * \return
 *      0 (OK) ou -1 em caso de erro.
 */
int rptable_put(c_rptable_t *rptable, char *key, struct data_t *value);

/* Retorna o elemento da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct data_t *rptable_get(c_rptable_t *rptable, char *key);

/* Função para remover um elemento da tabela. Vai libertar 
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rptable_del(c_rptable_t *rptable, char *key);

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rptable_size(c_rptable_t *rptable);

/* Retorna uma estrutura statistics_t que contem informacoes estatisticas
 * sobre o servidor de leitura.
 */
struct statistics_t *rptable_stats(c_rptable_t *rptable);

char **rptable_get_keys(c_rptable_t *rptable);

void rptable_free_keys(char **keys);

struct entry_t **rptable_get_table(c_rptable_t *rptable);

void rptable_free_entries(struct entry_t **entries);


/**
 * Funcao privada que faz tratamento dos eventos da ligacao
*/
void zkconnection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

/**
 * Funcao privada que faz tratamento dos eventos dos nos
*/
void zknode_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

#endif