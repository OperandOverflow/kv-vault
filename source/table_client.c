/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "table_client-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "stats.h"

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

struct rtable_t *connection;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR_ARGS);
        return -1;
    }

    // Estabelecer ligacao ao servidor
    connection = rtable_connect(argv[1]);
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
        fgets(input, 99, stdin);
        
        // Ignorar enters
        while (strlen(input) == 1) {
            memset(input, 0, sizeof(input));
            printf(CLIENT_SHELL);
            fflush(stdin);
            fgets(input, 99, stdin);
        }
        
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
            rtable_disconnect(connection);
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

void inthandler() {
    rtable_disconnect(connection);
    exit(0);
}

int put(struct rtable_t *rtable, char *key, char* value) {
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
    
    // Criar estrutura entry_t
    struct entry_t *entryptr = entry_create(keyptr, dataptr);
    if (entryptr == NULL) {
        free(keyptr);
        data_destroy(dataptr);
        printf("Error creating entry_t structure to send!\n");
        return -1;
    }

    // Enviar para o servidor
    int result = rtable_put(rtable, entryptr);
    if (result == -1) {
        printf("Error while sending put request!\n");
        entry_destroy(entryptr);
        return -1;
    }
    entry_destroy(entryptr);
    return 0;
}

int get(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return -1;
    
    // Obter o valor
    struct data_t *dataptr = rtable_get(rtable, key);
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

int delete(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL)
        return -1;
    
    int result = rtable_del(rtable, key);
    if (result == -1) {
        printf(ERROR_DEL);
        return -1;
    }
    return 0;
}

int size(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;
    int result = -1;
    result = rtable_size(rtable);
    if (result == -1){
        printf(ERROR_SIZE);
        return -1;
    }

    printf(AUX_SIZE, result);

    return 0;
}

int stats(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;

    struct statistics_t *stats = rtable_stats(rtable);
    if (stats == NULL) {
        printf(ERROR_STATS);
        return -1;
    }
    printf(AUX_STATS, stats_get_n_op(stats), 
        stats_get_time_lasted(stats), stats_get_n_client(stats));

    stats_destroy(stats);
    return 0;
}

int getkeys(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;
    char** keys = NULL;
    keys = rtable_get_keys(rtable);
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

    rtable_free_keys(keys);

    return 0;
}

int gettable(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;
    struct entry_t **entries = rtable_get_table(rtable);
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
    rtable_free_entries(entries);
    return 0;
}