/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "data.h"
#include "entry.h"
#include "stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rtable_t *rtable_connect(char *address_port) {
    if (address_port == NULL)
        return NULL;
    
    // Separar string em ip e porto
    char *ip = strtok(address_port, ":");
    char *port = strtok(NULL, ":");
    if (ip == NULL || port == NULL)
        return NULL;

    // Alocar espaco para a tabela
    struct rtable_t *table = malloc(sizeof(struct rtable_t));
    if (table == NULL)
        return NULL;

    table->server_address = ip;
    table->server_port = atoi(port);

    // Estabelecer a ligacao
    if (network_connect(table) == -1) {
        free(table);
        return NULL;
    }

    return table;
}

int rtable_disconnect(struct rtable_t *rtable) {
    int result = network_close(rtable);
    free(rtable);
    return result;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    if (rtable == NULL || entry == NULL)
        return -1;

    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    
    // Inicializar a entry
    EntryT entryt;
    entry_t__init(&entryt);
    entryt.key = entry->key;
    entryt.value.len = entry->value->datasize;
    entryt.value.data = entry->value->data;

    msg.entry = &entryt;

    // Enviar a mensagem
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return -1;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_PUT + 1) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    if (resp->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    message_t__free_unpacked(resp, NULL);
    
    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return NULL;
    
    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    // Enviar e receber a resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return NULL;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_GET + 1) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }
    if (resp->c_type != MESSAGE_T__C_TYPE__CT_VALUE) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }

    // Alocar o espaco para o conteudo
    int size = resp->value.len;
    void *data = malloc(size);
    if (data== NULL) {
        message_t__free_unpacked(resp, NULL);
        return data;
    }
    memcpy(data, resp->value.data, size);
    
    // Inicializar a esturutura data_t
    struct data_t *result = data_create(size, data);
    if (result == NULL) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }
    
    // Libertar a estrutura da resposta
    message_t__free_unpacked(resp, NULL);

    return result;
}

//gajo
int rtable_del(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return -1;

    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg.key = key;

    // Enviar e receber resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return -1;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_DEL + 1) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    if (resp->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    message_t__free_unpacked(resp, NULL);
    
    return 0;
}

//gajo
int rtable_size(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;

    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    
    // Enviar e receber resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return -1;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_SIZE + 1) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    if (resp->c_type != MESSAGE_T__C_TYPE__CT_RESULT) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }
    // Salva-guarda o resultado
    int size = resp->result;
    // Libertar a mensagem de resposta
    message_t__free_unpacked(resp, NULL);
    return size;
    
}

struct statistics_t *rtable_stats(struct rtable_t *rtable) {
    if (rtable == NULL)
        return NULL;

    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_STATS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Enviar e receber resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return NULL;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_STATS + 1 ||
        resp->c_type != MESSAGE_T__C_TYPE__CT_STATS) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }

    // Inicializar a estrutura
    struct statistics_t* stats = stats_init_args(resp->stats->n_op,
                                                 resp->stats->time,
                                                 resp->stats->n_clients);
    if (stats == NULL) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }

    message_t__free_unpacked(resp, NULL);

    return stats;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    if (rtable == NULL)
        return NULL;

    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Enviar e receber resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return NULL;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_GETKEYS + 1 ||
        resp->c_type != MESSAGE_T__C_TYPE__CT_KEYS) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }

    // Alocar espaco para apontadores de strings
    int numkeys = resp->n_keys;
    if (numkeys < 0)
        return NULL;
    char **reskeylist = (char**) malloc((numkeys + 1) * sizeof(char*));
    if (reskeylist == NULL) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }
    // Colocar NULL terminator no fim
    reskeylist[numkeys] = NULL;

    char **keysptr = resp->keys;
    // Iterar pela array de apontadores de strings
    for (int i = 0; i < numkeys; i++) {
        // Duplicar cada string
        reskeylist[i] = strdup(keysptr[i]);
        if (reskeylist[i] == NULL) {
            /* Libertar espaco de strings anteriores */
            for (int j = i - 1; j >= 0; j--)
                free(reskeylist[j]);
            /* Libertar array de apontadores */
            free(reskeylist);
            message_t__free_unpacked(resp, NULL);
            return NULL;
        }
    }

    message_t__free_unpacked(resp, NULL);
    return reskeylist;
}

void rtable_free_keys(char **keys) {
    if (keys == NULL)
        return;
    int index = 0;
    char *ptr = NULL;
    ptr = keys[index];
    // Iterar pelo cada apontador
    while (ptr != NULL) {
        free(keys[index]);
        index++;
        ptr = keys[index];
    }
    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    if (rtable == NULL)
        return NULL;
    
    // Inicializar a mensagem
    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Enviar e receber resposta
    MessageT *resp = network_send_receive(rtable, &msg);
    if (resp == NULL)
        return NULL;
    if (resp->opcode != MESSAGE_T__OPCODE__OP_GETTABLE + 1 ||
        resp->c_type != MESSAGE_T__C_TYPE__CT_TABLE) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }

    // Allocar espaco para array de apontadores
    int numentries = resp->n_entries;
    if (numentries < 0) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }
    struct entry_t **resentrlist = malloc((numentries + 1) * sizeof(struct entry_t*));
    if (resentrlist == NULL) {
        message_t__free_unpacked(resp, NULL);
        return NULL;
    }
    
    EntryT **entriesptr = resp->entries;
    // Iterar pela array de apontadores de entries
    for (int i = 0; i < numentries; i++) {
        // Alocar espaco para o conteudo e copia-lo
        void *contentptr = malloc(entriesptr[i]->value.len);
        if (contentptr == NULL) {
            for (int j = i-1; j >= 0; j--)
                entry_destroy(resentrlist[i]);
            free(resentrlist);
            message_t__free_unpacked(resp, NULL);
            return NULL;
        }
        memcpy(contentptr, entriesptr[i]->value.data, entriesptr[i]->value.len);

        // Criar estrutura data_t
        struct data_t *dataptr = data_create(entriesptr[i]->value.len, contentptr);
        if (dataptr == NULL) {
            // Libertar o espaco do conteudo 
            free(contentptr); 
            // Libertar entry_t anteriores
            for (int j = i - 1; j >= 0; j--)
                entry_destroy(resentrlist[i]);
            // Libertar array de apontadores
            free(resentrlist);
            message_t__free_unpacked(resp, NULL);
            return NULL;
        }

        // Criar estrutura entry_t
        struct entry_t *entryptr = entry_create(strdup(entriesptr[i]->key), dataptr);
        if (entryptr == NULL) {
            /* Libertar data_t */
            data_destroy(dataptr);
            /* Libertar entry_t anteriores */
            for (int j = i-1; j >= 0; j--)
                entry_destroy(resentrlist[i]);
            /* Libertar array de apontadores*/
            free(resentrlist);
            message_t__free_unpacked(resp, NULL);
            return NULL;
        }

        resentrlist[i] = entryptr;
    }
    // Colocar NULL terminator no fim
    resentrlist[numentries] = NULL;

    message_t__free_unpacked(resp, NULL);
    return resentrlist;
}

void rtable_free_entries(struct entry_t **entries) {
    if (entries == NULL)
        return;
    int index = 0;
    while (entries[index] != NULL) {
        entry_destroy(entries[index]);
        index++;
    }
    free(entries);
}