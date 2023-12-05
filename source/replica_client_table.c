/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "data.h"
#include "entry.h"
#include "table.h"
#include "client_stub.h"
#include "replica_table.h"
#include "table-private.h"
#include "replica_client_table.h"
#include "client_stub-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funcoes para fazer call-back
node_watcher rptable_watcher = NULL;
failure_handler rptable_fhandler = NULL;

c_rptable_t *rptable_connect(node_watcher watcher, failure_handler handler) {
    if (watcher == NULL || handler == NULL)
        return NULL;

    c_rptable_t *table_ptr = malloc(sizeof(c_rptable_t));
    if (table_ptr == NULL)
        goto err_rptable_malloc;

    // Iniciar a estrutura
    c_rptable_t table = {NULL, NULL, NULL, NULL, NULL};

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    // Iniciar o ZooKeeper
    if ((table.handler = zookeeper_init(RPTABLE_ZK_DEFAULT_SOCKET, 
                    zkconnection_watcher, RPTABLE_ZK_DEFAULT_TIMEOUT, 0, NULL, 0)) == NULL)
        goto err_zk_init;
    
    // Colocar watcher ao no raiz
    set_node_watcher(table.handler, RPTABLE_ZK_ROOT_PATH, zknode_watcher);

    // Obter o socket do servidor na cabeca
    table.rptable_wsocket = get_head_server(table.handler, 
                    RPTABLE_ZK_ROOT_PATH, zknode_watcher);
    
    // Se nao ha nenhum servidor ativo
    if (table.rptable_wsocket == ZDATA_NOT_FOUND || table.rptable_wsocket == NULL)
        goto err_rtable_wsocket;
    
    // Obter o socket do servidor na cauda
    table.rptable_rsocket = get_tail_server(table.handler,
                    RPTABLE_ZK_ROOT_PATH, zknode_watcher);

    if (table.rptable_rsocket == ZDATA_NOT_FOUND || table.rptable_rsocket == NULL)
        goto err_rtable_rsocket;

    // Se ha algum servidor na cabeca
    if (table.rptable_wsocket != NULL) {
        // Se ocorrer erro enquanto tentar ligar
        if ((table.rtable_w = rtable_connect(table.rptable_wsocket)) == NULL)
            goto err_rtable_w_con;
    }

    // Se ha algum servidor na cauda
    if (table.rptable_rsocket != NULL) {
        // Se ocorrer erro enquanto tentar ligar
        if ((table.rtable_r = rtable_connect(table.rptable_rsocket)) == NULL)
            goto err_rtable_r_con;
    }

    // Copiar para o buffer
    memcpy(table_ptr, &table, sizeof(c_rptable_t));

    // Guardar as funcoes para fazer call-back
    rptable_watcher = watcher;
    rptable_fhandler = handler;

    return table_ptr;

    err_rtable_r_con:
    rtable_disconnect(table.rtable_w);
    err_rtable_w_con:
    free(table.rptable_rsocket);
    err_rtable_rsocket:
    free(table.rptable_wsocket);
    err_rtable_wsocket:
    zookeeper_close(table.handler);
    err_zk_init:
    free(table_ptr);
    err_rptable_malloc:
    return NULL;
}

c_rptable_t *rptable_connect_zksock(char* zksock, node_watcher watcher, failure_handler handler) {
    if (watcher == NULL || handler == NULL)
        return NULL;

    c_rptable_t *table_ptr = malloc(sizeof(c_rptable_t));
    if (table_ptr == NULL)
        goto err_rptable_malloc;

    // Iniciar a estrutura
    c_rptable_t table = {NULL, NULL, NULL, NULL, NULL};

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    // Iniciar o ZooKeeper
    if ((table.handler = zookeeper_init(zksock, 
                    zkconnection_watcher, RPTABLE_ZK_DEFAULT_TIMEOUT, 0, NULL, 0)) == NULL)
        goto err_zk_init;
    
    // Colocar watcher ao no raiz
    set_node_watcher(table.handler, RPTABLE_ZK_ROOT_PATH, zknode_watcher);

    // Obter o socket do servidor na cabeca
    table.rptable_wsocket = get_head_server(table.handler, 
                    RPTABLE_ZK_ROOT_PATH, zknode_watcher);
    
    // Se nao ha nenhum servidor ativo
    if (table.rptable_wsocket == ZDATA_NOT_FOUND || table.rptable_wsocket == NULL)
        goto err_rtable_wsocket;
    
    // Obter o socket do servidor na cauda
    table.rptable_rsocket = get_tail_server(table.handler,
                    RPTABLE_ZK_ROOT_PATH, zknode_watcher);

    if (table.rptable_rsocket == ZDATA_NOT_FOUND || table.rptable_rsocket == NULL)
        goto err_rtable_rsocket;

    // Se ha algum servidor na cabeca
    if (table.rptable_wsocket != NULL) {
        // Se ocorrer erro enquanto tentar ligar
        if ((table.rtable_w = rtable_connect(table.rptable_wsocket)) == NULL)
            goto err_rtable_w_con;
    }

    // Se ha algum servidor na cauda
    if (table.rptable_rsocket != NULL) {
        // Se ocorrer erro enquanto tentar ligar
        if ((table.rtable_r = rtable_connect(table.rptable_rsocket)) == NULL)
            goto err_rtable_r_con;
    }

    // Copiar para o buffer
    memcpy(table_ptr, &table, sizeof(c_rptable_t));

    // Guardar as funcoes para fazer call-back
    rptable_watcher = watcher;
    rptable_fhandler = handler;

    return table_ptr;

    err_rtable_r_con:
    rtable_disconnect(table.rtable_w);
    err_rtable_w_con:
    free(table.rptable_rsocket);
    err_rtable_rsocket:
    free(table.rptable_wsocket);
    err_rtable_wsocket:
    zookeeper_close(table.handler);
    err_zk_init:
    free(table_ptr);
    err_rptable_malloc:
    return NULL;
}

int rptable_disconnect(c_rptable_t *rptable) {
    if (rptable == NULL)
        return -1;
    int res = 0;

    if(rptable->handler != NULL)
        zookeeper_close(rptable->handler);
    else 
        res = -1;

    if (rptable->rptable_wsocket != NULL)
        free(rptable->rptable_wsocket);
    else 
        res = -1;
    
    if (rptable->rptable_rsocket != NULL)
        free(rptable->rptable_rsocket);
    else 
        res = -1;
    
    if (rptable->rtable_w != NULL)
        rtable_disconnect(rptable->rtable_w);
    else 
        res = -1;
    
    if (rptable->rtable_r != NULL)
        rtable_disconnect(rptable->rtable_r);
    else 
        res = -1;
    
    free(rptable);
    return res;
}

int rptable_put(c_rptable_t *rptable, char *key, struct data_t *value) {
    if (rptable == NULL || key == NULL || value == NULL)
        return -1;
    if (rptable->rptable_wsocket == NULL || rptable->rtable_w == NULL)
        return -1;
    
    char *key_dup = strdup(key);
    if (key_dup == NULL)
        return -1;
    struct data_t *value_dup = data_dup(value);
    if (value_dup == NULL) {
        free(key_dup);
        return -1;
    }
    struct entry_t *entry = entry_create(key_dup, value_dup);
    if (entry == NULL) {
        data_destroy(value_dup);
        free(key_dup);
        return -1;
    }
    int res = rtable_put(rptable->rtable_w, entry);
    entry_destroy(entry);
    return res;
}

struct data_t *rptable_get(c_rptable_t *rptable, char *key) {
    if (rptable == NULL || key == NULL)
        return NULL;
    if (rptable->rptable_rsocket == NULL || rptable->rtable_r == NULL)
        return NULL;
    return rtable_get(rptable->rtable_r, key);
}

int rptable_del(c_rptable_t *rptable, char *key) {
    if (rptable == NULL || key == NULL)
        return -1;
    if (rptable->rptable_wsocket == NULL || rptable->rtable_w == NULL)
        return -1;
    return rtable_del(rptable->rtable_w, key);
}

int rptable_size(c_rptable_t *rptable) {
    if (rptable == NULL)
        return -1;
    if (rptable->rptable_rsocket == NULL || rptable->rtable_r == NULL)
        return -1;
    return rtable_size(rptable->rtable_r);
}

struct statistics_t *rptable_stats(c_rptable_t *rptable) {
    if (rptable == NULL)
        return NULL;
    if (rptable->rptable_rsocket == NULL || rptable->rtable_r == NULL)
        return NULL;
    return rtable_stats(rptable->rtable_r);
}

char **rptable_get_keys(c_rptable_t *rptable) {
    if (rptable == NULL)
        return NULL;
    if (rptable->rptable_rsocket == NULL || rptable->rtable_r == NULL)
        return NULL;
    return rtable_get_keys(rptable->rtable_r);
}

void rptable_free_keys(char **keys) {
    if (keys == NULL)
        return;
    return rtable_free_keys(keys);
}

struct entry_t **rptable_get_table(c_rptable_t *rptable) {
    if (rptable == NULL)
        return NULL;
    if (rptable->rptable_rsocket == NULL || rptable->rtable_r == NULL)
        return NULL;
    return rtable_get_table(rptable->rtable_r);
}

void rptable_free_entries(struct entry_t **entries) {
    if (entries == NULL)
        return;
    return rtable_free_entries(entries);
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
    
    c_rptable_t *table = rptable_watcher();
    if (table == NULL || table->handler == NULL || 
        table->rptable_wsocket == NULL || table->rtable_w == NULL ||
        table->rptable_rsocket == NULL || table->rtable_r == NULL) {
        rptable_fhandler(RPTABLE_INVALID_ARG);
        return;
    }
    
    // Tentar obter o descritor do proximo servidor
    char *next_headtable = get_head_server(table->handler, RPTABLE_ZK_ROOT_PATH, 
                                zknode_watcher);
    
    char *next_tailtable = get_tail_server(table->handler, RPTABLE_ZK_ROOT_PATH, 
                                zknode_watcher);

    // Se nao foi encontrado algum dos servidores
    if (next_headtable == NULL || next_tailtable == NULL) {
        rptable_fhandler(RPTABLE_CONNECTION_FAILED);
        return;
    }
    
    // Se o novo servidor na cabeca for diferente do atual
    if (strcmp(next_headtable, table->rptable_wsocket) != 0) {
        free(table->rptable_wsocket);
        rtable_disconnect(table->rtable_w);

        table->rptable_wsocket = next_headtable;
        // Se nao conseguir ligar ao novo servidor
        if ((table->rtable_w = rtable_connect(next_headtable)) == NULL) {
            free(next_headtable);
            free(next_tailtable);
            rptable_fhandler(RPTABLE_CONNECTION_FAILED);
        }
        free(next_tailtable);
        return;
    }
    
    // Se o novo servidor na cauda for diferente do atual
    if (strcmp(next_tailtable, table->rptable_rsocket) != 0) {
        free(table->rptable_rsocket);
        rtable_disconnect(table->rtable_r);

        table->rptable_rsocket = next_tailtable;
        // Se nao conseguir ligar ao novo servidor
        if ((table->rtable_r = rtable_connect(next_tailtable)) == NULL) {
            free(next_headtable);
            free(next_tailtable);
            rptable_fhandler(RPTABLE_CONNECTION_FAILED);
        }
        free(next_headtable);
        return;
    }

    free(next_headtable);
    free(next_tailtable);
    return;
}