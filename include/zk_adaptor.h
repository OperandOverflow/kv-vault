/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

/**
 * Módulo que adaptam as funcionalidades fornecidas pela
 * biblioteca de ZooKeeper às operações necessárias no 
 * projeto.
*/

#ifndef _ZK_ADAPTOR_H
#define _ZK_ADAPTOR_H

#include <zookeeper/zookeeper.h>

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024
#define ZDATA_NOT_FOUND ((void *)1)

#define LOOPBACK_IP "127.0.0.1"
typedef struct String_vector zoo_string; 

// =========================================================
//                    Operacoes gerais
// =========================================================

/**
 * Colocar uma funcao para observer alteracoes da estrutura
 * de um no.
 * \param handler
 *      ZooKeeper handler.
 * \param node
 *      Caminho completo para o no.
 * \param watcher
 *      Funcao que e chamada quando detetar alguma alteracao.
 * \return
 *      0 (OK), 1 se o no nao existe e -1 em caso de erro.
*/
int set_node_watcher(zhandle_t* handler, char* node, watcher_fn watcher);

// =========================================================
//                  Operacoes do servidor
// =========================================================
/**
 * Define o prefixo do no usado para registar o servidor.
 * \param nameprefix
 *      Apontador para o prefixo.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int set_server_prefix(char* nameprefix);

/**
 * Retorna o prefixo do nome de servidor.
 * \return
 *      Apontador para a string que contem o prefixo, ou 
 *      NULL caso nenhum prefixo ainda tenha sido definido  
 *      ou ocorreu um erro.
*/
char* get_server_prefix();

/**
 * Cria o no raiz.
 * \param handler
 *      ZooKeeper handler.
 * \param path
 *      Caminho completo para o no que pretende criar,
 *      inclui o nome do proprio no.
 * \return 
 *      0 (OK), 1 se o no ja existe ou -1 em caso de erro.
*/
int create_root(zhandle_t *handler, char *path);

/**
 * Regista o servidor atual no ZooKeeper, criando um no 
 * efemero com o numero de sequencia e o prefixo, coloca  
 * o socket do servidor no no criado.
 * \param handler
 *      ZooKeeper handler.
 * \param path
 *      Caminho para o diretorio.
 * \param socket
 *      Socket do servidor.
 * \return 
 *      Nome do no criado ou NULL em caso de erro.
*/
char* register_server(zhandle_t* handler, char* path, int socket);

/**
 * Retorna o descritor do proximo servidor em string 
 * no formato <ip>:<porto>.
 * \param handler
 *      ZooKeeper handler.
 * \param rootpath
 *      Caminho completo ao diretorio que contem os nos.
 * \param node
 *      Caminho completo ao no atual.
 * \param watcher
 *      Funcao watcher para observer o diretorio que tem os nos.
 * \return 
 *      Descritor do socket do servidor seguinte ou NULL
 *      caso nao tenha encontrado o proximo servidor ou NULL em 
 *      caso de erro.
*/
char* get_next_server(zhandle_t* handler, char* rootpath, char* node, watcher_fn watcher);

/**
 * Retorna o descritor do servidor anterior em string 
 * no formato <ip>:<porto>.
 * \param handler
 *      ZooKeeper handler.
 * \param rootpath
 *      Caminho completo ao diretorio que contem os nos.
 * \param node
 *      Caminho completo ao no atual.
 * \param watcher
 *      Funcao watcher para observer o diretorio que tem os nos.
 * \return 
 *      Descritor do socket do servidor seguinte ou ZDATA_NOT_FOUND
 *      caso nao tenha encontrado o proximo servidor ou NULL em 
 *      caso de erro.
*/
char* get_prev_server(zhandle_t* handler, char* rootpath, char* node, watcher_fn watcher);
// =========================================================
//                  Operacoes do cliente
// =========================================================

/**
 * Retorna o socket do servidor com o menor identificador 
 * (mais antigo) em string no formato <ip>:<porto>.
 * \param handler
 *      ZooKeeper handler.
 * \param path
 *      Caminho ao no que contem os nos dos servidores.
 * \param watcher
 *      Funcao que e invocada quando houver alguma alteracao no no.
 * \return
 *      Descritor do socket do servidor na cabeca ou ZDATA_NOT_FOUND
 *      caso nao tenha encontrado o proximo servidor ou NULL em 
 *      caso de erro.
*/
char* get_head_server(zhandle_t* handler, char* path, watcher_fn watcher);

/**
 * Retorna o socket do servidor com o maior identificador 
 * (mais novo) em string no formato <ip>:<porto>.
 * \param handler
 *      ZooKeeper handler.
 * \param path
 *      Caminho ao no que contem os nos dos servidores.
 * \param watcher
 *      Funcao que e invocada quando houver alguma alteracao no no.
 * \return
 *      Descritor do socket do servidor na cauda ou ZDATA_NOT_FOUND
 *      caso nao tenha encontrado o proximo servidor ou NULL em 
 *      caso de erro.
*/
char* get_tail_server(zhandle_t* handler, char* path, watcher_fn watcher);




// ================== Funcoes privadas ====================
// static void search_node(zhandle_t* handler, char* path, watcher_fn watcher);

// static void search_node_dispatcher(char const * caller_name, zhandle_t* handler, char* path, watcher_fn watcher);

#endif