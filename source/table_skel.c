/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "table_skel.h"
#include "table_skel-private.h"
#include "table.h"
#include "data.h"
#include "sdmessage.pb-c.h"
#include "stats.h"
#include "synchronization.h"
#include "replica_table.h"
#include "replica_server_table.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

// Controlo da concorrencia no acesso a tabela
rwcctrl_t *cctrl;

// Estatisticas da tabela
stats_t *stats;

int inc_num_clients() {
    return stats_inc_client(stats);
}

int dec_num_clients() {
    return stats_dec_client(stats);
}

int get_num_clients() {
    return stats_get_n_client(stats);
}

int get_num_ops() {
    return stats_get_n_op(stats);
}

int get_time_used() {
    return stats_get_time_lasted(stats);
}

/**
 * Retorna o tempo atual em microssegundos.
*/
long get_time() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000000) + now.tv_usec;
}

/**
 * Preenche a mensagem com codigos de erro.
 * \param msg
 *      Mensagem para preencher com codigos de erro.
 * \return 
 *      0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_error(MessageT *msg) {
    if (msg == NULL)
        return -1;
    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    return 0;
}

/**
 * Coloca a entrada no pedido para tabela e prepara a 
 * mensagem da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feita a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_put(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_ENTRY)
        return invoke_error(msg);
    if (msg->entry == NULL ||
        msg->entry->key == NULL)
        return invoke_error(msg);
    if (msg->entry->value.data == NULL || 
        msg->entry->value.len < 0)
        return invoke_error(msg);
    
    // Registar o tempo do inicio
    long start_time = get_time();

    // Alocar espaco para o conteudo
    void *buf_dup = malloc(msg->entry->value.len);
    if (buf_dup == NULL) {
        return invoke_error(msg);
    }

    // Copiar o conteudo para o buffer
    memcpy(buf_dup, msg->entry->value.data, msg->entry->value.len);

    // Inicializar estrutura data_t
    struct data_t *data = data_create(msg->entry->value.len, buf_dup);
    if (data == NULL) {
        free(buf_dup);
        return invoke_error(msg);
    }

    // ============== SECCAO CRITICA ==============
    write_begin(cctrl);

    // Colocar o conteudo na tabela
    int result = table_put(table, msg->entry->key, data);
    if(result == -1) {
        write_end(cctrl);
        data_destroy(data);
        return invoke_error(msg);
    }

    write_end(cctrl);
    // ============================================

    // Libertar os dados temporarios
    data_destroy(data);

    // Preencher os campos da resposta
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    stats_op_finish(stats, get_time() - start_time);

    return result;
}

/**
 * Obtem uma entrada da tabela e coloca-a na mensagem
 * da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_get(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY) 
        return invoke_error(msg);
    if (msg->key == NULL)
        return invoke_error(msg);
    
    // Registar o tempo do inicio
    long start_time = get_time();

    // ============== SECCAO CRITICA ==============
    read_begin(cctrl);

    // Obter a entrada da tabela
    struct data_t *data = table_get(table, msg->key);
    if (data == NULL) {
        read_end(cctrl);
        return invoke_error(msg);
    }
    
    read_end(cctrl);
    // ============================================

    char value[data->datasize + 1];
    value[data->datasize] = '\0';
    memcpy(value, data->data, data->datasize);

    // Alocar espaco para o conteudo
    void *content = malloc(data->datasize);
    if (content == NULL) {
        data_destroy(content);
        return invoke_error(msg);
    }

    // Copiar o conteudo para o buffer
    memcpy(content, data->data, data->datasize);
    int size = data->datasize;
    if (data_destroy(data) == -1) {
        free(content);
        return invoke_error(msg);
    }
    
    msg->value.data = content;
    msg->value.len = size;

    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

    stats_op_finish(stats, get_time() - start_time);

    return 0;
}

/**
 * Apaga uma entrada da tabela e prepara a mensagem
 * da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_delete(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY)
        return invoke_error(msg);
    if (msg->key == NULL)
        return invoke_error(msg);
    
    // Registar o tempo do inicio
    long start_time = get_time();

    // ============== SECCAO CRITICA ==============
    write_begin(cctrl);

    // Remover a entrada da tabela
    int result = table_remove(table, msg->key);
    if (result == -1) {
        write_end(cctrl);
        return invoke_error(msg);
    }

    write_end(cctrl);
    // ============================================

    msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    stats_op_finish(stats, get_time() - start_time);

    return 0;
}

/**
 * Obtem o tamanho da tabela e coloca-o na mensagem
 * da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_size(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return invoke_error(msg);
    
    // Registar o tempo do inicio
    long start_time = get_time();

    // ============== SECCAO CRITICA ==============
    read_begin(cctrl);

    // Obter o tamanho da tabela
    int size = table_size(table);
    if (size == -1) {
        read_end(cctrl);
        return invoke_error(msg);
    }
    
    read_end(cctrl);
    // ============================================

    msg->result = size;
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

    stats_op_finish(stats, get_time() - start_time);

    return 0;
}

/**
 * Obtem todas as chaves da tabela e coloca-as 
 * na mensagem da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_getkeys(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return invoke_error(msg);
    
    long start_time = get_time();

    // ============== SECCAO CRITICA ==============
    read_begin(cctrl);

    // Obter a array de chaves
    char **keys = table_get_keys(table);
    if (keys == NULL) {
        read_end(cctrl);
        return invoke_error(msg);
    }
    
    // Obter o tamanho da tabela
    int keyarraysize = table_size(table);
    if (keyarraysize == -1) {
        read_end(cctrl);
        table_free_keys(keys);
        return invoke_error(msg);
    }

    read_end(cctrl);
    // ============================================

    // Reservar espaco para a array de chaves
    char** keysptr = malloc(keyarraysize * sizeof(char*));
    if (keysptr == NULL) {
        table_free_keys(keys);
        return invoke_error(msg);
    }

    // Iterar pela array e duplicar chaves
    for (int i = 0; i < keyarraysize; i++) {
        keysptr[i] = strdup(keys[i]);
        if (keysptr[i] == NULL) {
            // Libertar strings copiadas anteriormente
            for (int j = i - 1; j >= 0; j--)
                free(keysptr[j]);
            free(keysptr);
            table_free_keys(keys);
            return invoke_error(msg);
        }
    }
    
    table_free_keys(keys);

    msg->n_keys = keyarraysize;
    msg->keys = keysptr;
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

    stats_op_finish(stats, get_time() - start_time);

    // printf("    table skel: GETKEYS successful.\n");

    return 0;
}

/**
 * Obtem todas as entradas da tabela e coloca-as 
 * na mensagem da resposta.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_gettable(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return invoke_error(msg);
    
    // Registar o tempo do inicio
    long start_time = get_time();
    
    // ============== SECCAO CRITICA ==============
    read_begin(cctrl);
    
    // Obter array de chaves
    char **keys = table_get_keys(table);
    if (keys == NULL) {
        read_end(cctrl);
        return invoke_error(msg);
    }

    // Obter o tamanho da tabela
    int entryarraysize = table_size(table);
    if (entryarraysize == -1) {
        read_end(cctrl);
        table_free_keys(keys);
        return invoke_error(msg);
    }

    read_end(cctrl);
    // ============================================

    // Alocar espaco para array de apontadores de entradas
    EntryT **entriesptr = malloc(entryarraysize * sizeof(EntryT *));
    if (entriesptr == NULL) {
        table_free_keys(keys);
        return invoke_error(msg);
    }

    // Obter cada entrada da tabela e passar para EntryT
    for (int i = 0; i < entryarraysize; i++) {
        // ============== SECCAO CRITICA ==============
        read_begin(cctrl);

        struct data_t *data = table_get(table, keys[i]);
        if (data == NULL) {
            read_end(cctrl);
            free(entriesptr);
            table_free_keys(keys);
            return invoke_error(msg);
        }

        read_end(cctrl);
        // ============================================

        // Duplicar o conteudo
        void *content = malloc(data->datasize);
        if (content == NULL) {
            data_destroy(data);
            free(entriesptr);
            table_free_keys(keys);
            return invoke_error(msg);
        }
        memcpy(content, data->data, data->datasize);

        // Inicializar EntryT
        entriesptr[i] = malloc(sizeof(EntryT));
        entry_t__init(entriesptr[i]);
        entriesptr[i]->key = strdup(keys[i]);
        entriesptr[i]->value.len = data->datasize;
        entriesptr[i]->value.data = content;
        if (entriesptr[i]->key == NULL) {
            // Libertar posicoes anteriores
            for (int j = i - 1; j >= 0; j--)
                entry_t__free_unpacked(entriesptr[i], NULL);
            free(content);
            data_destroy(data);
            free(entriesptr);
            table_free_keys(keys);
            return invoke_error(msg);
        }

        data_destroy(data);
    }

    table_free_keys(keys);

    msg->n_entries = entryarraysize;
    msg->entries = entriesptr;
    msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;

    stats_op_finish(stats, get_time() - start_time);

    return 0;
}

/**
 * Retorna as estatisticas do servidor.
 * \param msg
 *      Mensagem que contem o pedido.
 * \param table
 *      Tabela sobre qual sera feira a operacao.
 * \return
 *      Retorna 0 se concluiu com sucesso, -1 caso contrario.
*/
int invoke_stats(MessageT *msg, struct table_t *table) {
    // Validacao do pedido
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return invoke_error(msg);
    
    StatsT *statis = malloc(sizeof(StatsT));
    if (statis == NULL)
        return invoke_error(msg);
    
    stats_t__init(statis);
    statis->n_clients = stats_get_n_client(stats);
    statis->n_op = stats_get_n_op(stats);
    statis->time = stats_get_time_lasted(stats);

    msg->stats = statis;
    msg->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;

    return 0;
}

struct table_t *table_skel_init(int n_lists) {
    if (n_lists <= 0)
        return NULL;
    struct table_t *table = table_create(n_lists);
    if (table == NULL)
        return NULL;
    // Inicializar a estrutura para controlo de concorrencia
    if ((cctrl = cctrl_init()) == NULL) {
        table_destroy(table);
        return NULL;
    }    
    // Inicializar a estrutura stats_t
    if ((stats = stats_init()) == NULL) {
        table_destroy(table);
        cctrl_destroy(cctrl);
        return NULL;
    }

    return table;
}

int table_skel_destroy(struct table_t *table) {
    int result = 0;
    if (table == NULL)
        result = -1;
    if (table_destroy(table) != 0)
        result = -1;
    if (cctrl_destroy(cctrl) != 0)
        result = -1;
    if (stats_destroy(stats) != 0)
        result = -1;
    return result;
}

int invoke(MessageT *msg, struct table_t *table, s_rptable_t *rptable) {
    if (msg == NULL)
        return -1;
    if (table == NULL)
        return invoke_error(msg);
        
    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_PUT:
            return invoke_put(msg, table);
            break;
        
        case MESSAGE_T__OPCODE__OP_GET:
            return invoke_get(msg, table);
            break;
        
        case MESSAGE_T__OPCODE__OP_DEL:
            return invoke_delete(msg, table);
            break;

        case MESSAGE_T__OPCODE__OP_SIZE:
            return invoke_size(msg, table);
            break;
        
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            return invoke_getkeys(msg, table);
            break;
        
        case MESSAGE_T__OPCODE__OP_GETTABLE:
            return invoke_gettable(msg, table);
            break;
        
        case MESSAGE_T__OPCODE__OP_STATS:
            return invoke_stats(msg, table);
            break;

        default:
            invoke_error(msg);
            return 0;
    }
}