#ifndef _ZK_ADAPTOR_H
#define _ZK_ADAPTOR_H

#include <zookeeper/zookeeper.h>

// =========================================================
//                  Operacoes do servidor
// =========================================================
/**
 * Define o prefixo do nome de servidor para registar servidor.
 * \param nameprefix
 *      Apontador para o prefixo.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int set_server_prefix(char* nameprefix);

/**
 * Retorna o prefixo do nome de servidor.
 * \return
 *      Apontador para a string que contem o prefixo, ou NULL
 *      caso nenhum prefixo ainda tenha sido definido ou ocorreu 
 *      um erro.
*/
char* get_server_prefix();

/**
 * Regista o servidor atual no ZooKeeper, criando um no efemero
 * com o numero de sequencia e o prefixo, coloca o socket do 
 * servidor no no criado.
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

char* get_next_server(zhandle_t* handler, char* node, watcher_fn watcher);

// =========================================================
//                  Operacoes do cliente
// =========================================================

char* get_head_server(zhandle_t* handler, char* path, watcher_fn watcher);

char* get_tail_server(zhandle_t* handler, char* path, watcher_fn watcher);

#endif