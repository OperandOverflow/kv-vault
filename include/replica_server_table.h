/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

/**
 * Módulo que contém definicao de estrutura e de métodos 
 * que podem ser usados pelo servidor para fazer operacoes   
 * sobre a tabela replicada, abstraindo a comunicação  
 * com o ZooKeeper e fornece uma interface semelhante ao 
 * client_stub.
*/

#ifndef _REPLICA_SERVER_TABLE_H
#define _REPLICA_SERVER_TABLE_H

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
typedef struct server_rptable_t {
    zhandle_t *handler;
    char *znode;

    char *rptable_socket;
    struct rtable_t *rtable;
} s_rptable_t;

/**
 * Funcao que deve ser implementada pelo servidor,
 * apenas deve retornar o objeto.
*/
typedef s_rptable_t *(*node_watcher)();

// =========================================================
//                        Funcoes
// =========================================================

/**
 * Estabelece ligacao a tabela replicada, usando o socket
 * do ZooKeeper de omissao.
 * \param sock
 *      Descritor do socket do servidor.
 * \param watcher
 *      Funcao que escuta dos eventos 
 * \param handler
 *      Funcao que faz o tratamento da falha da 
 *      tabela replicada.RPTABLE_ZK_ROOT_PATH
 * \return
 *      Apontador a s_rptable_t ou NULL em caso de erro.
*/
s_rptable_t *rptable_connect(int sock, node_watcher watcher, failure_handler handler);

/**
 * Estabelece ligacao a tabela replicada, especificando o 
 * socket do servidor ZooKeeper.
 * \param zksock
 *      String que descreve o socket do servidor ZooKeeper.
 * \param sock
 *      Descritor do socket do servidor.
 * \param watcher
 *      Funcao que escuta dos eventos 
 * \param handler
 *      Funcao que faz o tratamento da falha da 
 *      tabela replicada.
 * \return
 *      Apontador a s_rptable_t ou NULL em caso de erro.
*/
s_rptable_t *rptable_connect_zksock(char* zksock, int sock, node_watcher watcher, failure_handler handler);

/**
 * Sincroniza a tabela local com a tabela no servidor anterior.
 * \return
 *      0 (OK) ou -1 em caso de erro.
*/
int rptable_sync(s_rptable_t *rptable, struct table_t *table);

/**
 * Desliga a ligacao com a tabela replicada.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int rptable_disconnect(s_rptable_t *rptable);

/** 
 * Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \param key
 *      Chave associada a entrada.
 * \param data
 *      Conteudo para ser colocado na entrada.
 * \return
 *      0 (OK) ou -1 em caso de erro.
 */
int rptable_put(s_rptable_t *rptable, char *key, struct data_t *value);

/** 
 * Retorna o elemento da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \param key
 *      Chave associada a entrada.
 * \return
 *      Estrutura data_t que contem o conteudo da entrada ou NULL
 *      caso nao exista ou se ocorreu algum erro.
 */
struct data_t *rptable_get(s_rptable_t *rptable, char *key);

/**
 * Função para remover um elemento da tabela. Vai libertar 
 * toda a memoria alocada na respetiva operação rptable_put().
 * \param rptable
 *      Apontador a estrutura c_rptable_t.
 * \param key
 *      Chave da entrada para ser removida.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
 */
int rptable_del(s_rptable_t *rptable, char *key);

/**
 *  Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return
 *      Numero de elementos na tabela ou -1 em caso de erro.
 */
int rptable_size(s_rptable_t *rptable);

/** 
 * Retorna uma estrutura statistics_t que contem informacoes estatisticas
 * sobre o servidor seguinte.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return
 *      Estrutura statistics_t que contem as informacoes estatisticas
 *      da tabela remota ou NULL em caso de erro.
 */
struct statistics_t *rptable_stats(s_rptable_t *rptable);

/**
 * Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return
 *      Array de keys ou NULL em caso de erro.
 */
char **rptable_get_keys(s_rptable_t *rptable);

/**
 * Liberta a memória alocada por rptable_get_keys().
 * \param keys
 *      Array de keys.
*/
void rptable_free_keys(char **keys);

/**
 * Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL.
 * \param rptable
 *      Apontador a estrutura s_rptable_t.
 * \return
 *      Array de entry_t* ou NULL em caso de erro.
*/
struct entry_t **rptable_get_table(s_rptable_t *rptable);

/**
 * Liberta a memória alocada por rptable_get_table().
 * \param entries
 *      Array de entry_t*.
*/
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