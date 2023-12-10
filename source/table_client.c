/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "data.h"
#include "entry.h"
#include "stats.h"
#include "client_cmd.h"
#include "client_stub.h"
#include "replica_table.h"
#include "client_stub-private.h"
#include "replica_client_table.h"
#include "table_client-private.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <stdbool.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>

c_rptable_t *connection;

void inthandler() {
    printf(SUCCESS_EXIT_TROLL);
    clear_history();
    rptable_disconnect(connection);
    exit(0);
}

c_rptable_t *table_watcher() {
    return connection;
}

void table_fhandler(int errcode) {
    inthandler();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR_ARGS);
        return -1;
    }

    // Estabelecer ligacao ao servidor
    connection = rptable_connect_zksock(argv[1], table_watcher, table_fhandler);
    if (connection == NULL) {
        printf(ERROR_CONNECTION);
        return -1;
    }

    // Definir o tratamento dos sinais
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, inthandler);

    char finished = false;
    while (!finished) {
        
        // Ler o input
        char input[100];
        printf(CLIENT_SHELL);
        // fgets(input, 99, stdin);
        getinput(input, 99);

        // Verificar se e igual e NACK
        if (input[0] == '\03')
            inthandler();
        
        // Ignorar enters
        while (strlen(input) == 0) {
            // Verificar se e igual e NACK
            if (input[0] == '\03')
                inthandler();
            memset(input, 0, sizeof(input));
            printf(CLIENT_SHELL);
            fflush(stdin);
            getinput(input, 99);
        }

        // Verificar se e igual e NACK
        if (input[0] == '\03')
            inthandler();
        
        // Separar em tokens
        char *command = strtok(input, " \n");

        // Reconhecer operacoes
        if (strcasecmp(command, "p") == 0 ||
            strcasecmp(command, "put") == 0) {
            
            // Obter argumentos da operacao
            char *key = strtok(NULL, " \n");
            char *value = strtok(NULL, "\n");
            if (key == NULL) {
                printf(ERROR_MISSING_ARGS, "<key> and <value>", "PUT");
                goto end;
            }
            if (value == NULL) {
                printf(ERROR_MISSING_ARGS, "<value>", "PUT");
                goto end;
            }
            
            int result = put(connection, key, value);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "PUT");

        } else 
        if (strcasecmp(command, "g") == 0 ||
            strcasecmp(command, "get") == 0) {
            char *key = strtok(NULL, "\n");
            if (key == NULL) {
                printf(ERROR_MISSING_ARGS, "<key>", "GET");
                goto end;
            }

            int result = get(connection, key);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "GET");
        } else
        if (strcasecmp(command, "d") == 0 ||
            strcasecmp(command, "del") == 0) {
            char *key = strtok(NULL, "\n");
            if (key == NULL) {
                printf(ERROR_MISSING_ARGS, "<key>", "DEL");
                goto end;
            }

            int result = delete(connection, key);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "DEL");
        } else
        if (strcasecmp(command, "s") == 0 ||
            strcasecmp(command, "size") == 0) {
            int result = size(connection);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "SIZE");
        } else
        if (strcasecmp(command, "a") == 0 ||
            strcasecmp(command, "stats") == 0) {
            int result = stats(connection);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "STATS");
        } else
        if (strcasecmp(command, "k") == 0 ||
            strcasecmp(command, "getkeys") == 0) {
            int result = getkeys(connection);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "GETKEYS");
        } else
        if (strcasecmp(command, "t") == 0 ||
            strcasecmp(command, "gettable") == 0) {
            int result = gettable(connection);
            if (result == -1)
                goto end;
            printf(SUCCESS_OPERATION, "GETTABLE");
        } else
        if (strcasecmp(command, "q") == 0 ||
            strcasecmp(command, "quit") == 0) {
            clear_history();
            rptable_disconnect(connection);
            printf(SUCCESS_EXIT);
            exit(0);
        } else
        if (strcasecmp(command, "h") == 0 ||
            strcasecmp(command, "help") == 0) {
            printf(AUX_HELP);
        } else
            printf(ERROR_UNKNOWN_COMMAND);

        end:
        command = NULL;
        fflush(stdin);
        continue;
    }
    
    return 0;
}


int put(c_rptable_t *rtable, char *key, char* value) {
    if (rtable == NULL || key == NULL || value == NULL)
        return -1;
    
    // Alocar espaco para o conteudo
    void *contentptr = malloc(strlen(value));
    if (contentptr == NULL) {
        printf("Error creating data structure to send!");
        return -1;
    }

    // Copiar o conteudo
    memcpy(contentptr, value, strlen(value));

    // Criar estrutura data_t
    struct data_t *dataptr = data_create(strlen(value), contentptr);
    if (dataptr == NULL) {
        printf("Error creating data structure to send!\n");
        free(contentptr);
        return -1;
    }
    
    // Duplicar a chave
    char *keyptr = strdup(key);
    if (keyptr == NULL) {
        printf("Error creating data structure to send!\n");
        data_destroy(dataptr);
        return -1;
    }

    // Enviar para o servidor
    int result = rptable_put(rtable, keyptr, dataptr);
    if (result == -1) {
        printf("Error while sending put request!\n");
        data_destroy(dataptr);
        free(keyptr);
        return -1;
    }
    data_destroy(dataptr);
    free(keyptr);
    return 0;
}

int get(c_rptable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return -1;
    
    // Obter o valor
    struct data_t *dataptr = rptable_get(rtable, key);
    if (dataptr == NULL) {
        printf(ERROR_GET);
        return -1;
    }

    // Declarar buffer para 
    char data[dataptr->datasize + 1];
    // Colocar null terminator
    data[dataptr->datasize] = '\0';
    // Copiar os caracteres para buffer
    memcpy(&data, dataptr->data, dataptr->datasize);
    // Imprimir o conteudo
    printf(AUX_GET,key, data, dataptr->datasize);
    
    // Libertar o espaco
    data_destroy(dataptr);
    return 0;
}

int delete(c_rptable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return -1;
    
    int result = rptable_del(rtable, key);
    if (result == -1) {
        printf(ERROR_DEL);
        return -1;
    }
    return 0;
}

int size(c_rptable_t *rtable) {
    if (rtable == NULL)
        return -1;
    int result = -1;
    result = rptable_size(rtable);
    if (result == -1){
        printf(ERROR_SIZE);
        return -1;
    }

    printf(AUX_SIZE, result);

    return 0;
}

int stats(c_rptable_t *rtable) {
    if (rtable == NULL)
        return -1;

    struct statistics_t *stats = rptable_stats(rtable);
    if (stats == NULL) {
        printf(ERROR_STATS);
        return -1;
    }
    printf(AUX_STATS, stats_get_n_op(stats), 
        stats_get_time_lasted(stats), stats_get_n_client(stats));

    stats_destroy(stats);
    return 0;
}

int getkeys(c_rptable_t *rtable) {
    if (rtable == NULL)
        return -1;
    char** keys = NULL;
    keys = rptable_get_keys(rtable);
    if (keys == NULL) {
        printf(ERROR_GETKEYS);
        return -1;
    }

    // Iterar pela array de apontadores de strings
    int index = 0;
    printf(AUX_GETKEYS);
    while (keys[index] != NULL) {
        char str[strlen(keys[index]) + 1];
        str[strlen(keys[index])] = '\0';
        memcpy(str, keys[index], strlen(keys[index]));
        printf(AUX_GETKEYS_LINE, str);
        index++;
    }

    rptable_free_keys(keys);

    return 0;
}

int gettable(c_rptable_t *rtable) {
    if (rtable == NULL)
        return -1;
    struct entry_t **entries = rptable_get_table(rtable);
    if (entries == NULL) {
        printf(ERROR_GETTABLE);
        return -1;
    }

    int index = 0;
    printf(AUX_GETTABLE);
    while (entries[index] != NULL) {
        char value[(entries[index]->value->datasize) + 1];
        value[entries[index]->value->datasize] = '\0';
        memcpy(value, entries[index]->value->data, entries[index]->value->datasize);
        printf(AUX_GETTABLE_LINE, entries[index]->key, value);
        index++;
    }
    rptable_free_entries(entries);
    return 0;
}