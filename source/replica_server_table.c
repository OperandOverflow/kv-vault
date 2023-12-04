/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "client_stub.h"
#include "replica_table.h"
#include "replica_server_table.h"
#include "client_stub-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funcoes para fazer call-back
node_watcher rptable_watcher = NULL;
failure_handler rptable_fhandler = NULL;

s_rptable_t *rptable_connect(int sock, node_watcher watcher, failure_handler handler) {
    if (sock < 0 || watcher == NULL || handler == NULL)
        return NULL;

    s_rptable_t *table_ptr = malloc(sizeof(s_rptable_t));
    if (table_ptr == NULL)
        goto err_rptable_malloc;

    // Iniciar a estrutura
    s_rptable_t table = {NULL, NULL, NULL, NULL};

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    // Iniciar o ZooKeeper
    if ((table.handler = zookeeper_init(RPTABLE_ZK_DEFAULT_SOCKET, 
                    zkconnection_watcher, 2000, 0, NULL, 0)) == NULL)
        goto err_zk_init;

    // Definir o prefixo do no
    set_server_prefix(RPTABLE_ZK_NODE_PREFIX);
    
    // Criar o no raiz
    if (create_root(table.handler, RPTABLE_ZK_ROOT_PATH) < 0)
        goto err_zk_create_root;

    // Criar um no efemero no zk
    if ((table.znode = register_server(table.handler, 
                    RPTABLE_ZK_ROOT_PATH, sock)) == NULL)
        goto err_zk_reg_server;
    
    // Colocar watcher ao no raiz
    set_node_watcher(table.handler, RPTABLE_ZK_ROOT_PATH, zknode_watcher);

    // Obter o socket do servidor seguinte
    table.rptable_socket = get_next_server(table.handler, 
                    RPTABLE_ZK_ROOT_PATH, table.znode, zknode_watcher);
    
    // Se ha algum servidor seguinte
    if (table.rptable_socket != NULL) {
        // Se ocorrer erro enquanto tentar ligar
        if ((table.rtable = rtable_connect(table.rptable_socket)) == NULL)
            goto err_rtable_con;
    }

    // Copiar para o buffer
    memcpy(table_ptr, &table, sizeof(s_rptable_t));

    // Guardar as funcoes para fazer call-back
    rptable_watcher = watcher;
    rptable_fhandler = handler;

    return table_ptr;

    err_rtable_con:
    free(table.rptable_socket);
    err_zk_reg_server:
    zookeeper_close(table.handler);
    err_zk_create_root:
    err_zk_init:
    free(table_ptr);
    err_rptable_malloc:
    return NULL;
}

int rptable_disconnect(s_rptable_t *rptable) {
    set_server_prefix(NULL);
    if (rptable == NULL)
        return -1;
    int res = 0;
    if(rptable->handler != NULL)
        zookeeper_close(rptable->handler);
    else 
        res = -1;

    if (rptable->znode != NULL)
        free(rptable->znode);
    else 
        res = -1;

    if (rptable->rptable_socket != NULL)
        free(rptable->rptable_socket);

    if (rptable->rtable != NULL)
        rtable_disconnect(rptable->rtable);

    free(rptable);
    return res;
}

int rptable_put(s_rptable_t *rptable, struct entry_t *entry) {
    if (rptable == NULL || entry == NULL)
        return -1;
    if (rptable->rtable == NULL)
        return -1;
    return rtable_put(rptable->rtable, entry);
}

struct data_t *rptable_get(s_rptable_t *rptable, char *key) {
    if (rptable == NULL || key == NULL)
        return NULL;
    if (rptable->rtable == NULL)
        return NULL;
    return rtable_get(rptable->rtable, key);
}

int rptable_size(s_rptable_t *rptable) {
    if (rptable == NULL || rptable->rtable == NULL)
        return -1;
    return rtable_size(rptable->rtable);
}

struct statistics_t *rptable_stats(s_rptable_t *rptable) {
    if (rptable == NULL || rptable->rtable == NULL)
        return NULL;
    return rtable_stats(rptable->rtable);
}

char **rptable_get_keys(s_rptable_t *rptable) {
    if (rptable == NULL || rptable->rtable == NULL)
        return NULL;
    return rtable_get_keys(rptable->rtable);
}

void rptable_free_keys(char **keys) {
    if (keys == NULL)
        return;
    rtable_free_keys(keys);
}

struct entry_t **rptable_get_table(s_rptable_t *rptable) {
    if (rptable == NULL || rptable->rtable == NULL)
        return NULL;
    return rtable_get_table(rptable->rtable);
}

void rptable_free_entries(struct entry_t **entries) {
    if (entries == NULL)
        return;
    rtable_free_entries(entries);
}


void zkconnection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state != ZOO_CONNECTED_STATE) {
            rptable_fhandler(ZKCONNECTION_LOST);
            return;
		}
	} 
}

void zknode_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
    if (state != ZOO_CONNECTED_STATE) {
        rptable_fhandler(ZKCONNECTION_LOST);
        return;
    }
    if (type != ZOO_CHILD_EVENT)
        return;
    
    s_rptable_t *table = rptable_watcher();
    if (table == NULL || table->handler == NULL || table->znode == NULL) {
        rptable_fhandler(RPTABLE_INVALID_ARG);
        return;
    }
    
    // Tentar obter o descritor do proximo servidor
    char *next_table = get_next_server(table->handler, RPTABLE_ZK_ROOT_PATH, 
                                table->znode, zknode_watcher);

    // Se nao foi encontrado nenhum servidor seguinte 
    // e o atual nao existe
    if (next_table == NULL && 
        table->rptable_socket == NULL &&
        table->rtable == NULL)
        return;
    
    // Se nao esta ligado a nenhum servidor e o novo nao e' NULL (apareceu um novo)
    if (table->rptable_socket == NULL && table->rtable == NULL) {
        table->rptable_socket = next_table;
        // Se nao conseguir ligar ao novo servidor
        if ((table->rtable = rtable_connect(next_table)) == NULL)
            rptable_fhandler(RPTABLE_CONNECTION_FAILED);
        return;
    }

    // Se esta ligado a um servidor e o novo e' NULL (o atual foi abaixo)
    if (next_table == NULL && 
        table->rptable_socket != NULL && 
        table->rtable != NULL) {
        rtable_disconnect(table->rtable);
        table->rtable = NULL;
        free(table->rptable_socket);
        table->rptable_socket = NULL;
        return;
    }

    rptable_fhandler(RPTABLE_INVALID_ARG);
    return;
}