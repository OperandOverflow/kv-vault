/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "zk_adaptor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <zookeeper/zookeeper.h>

char *server_name_prefix = NULL;

int set_node_watcher(zhandle_t* handler, char* node, watcher_fn watcher) {
    if (handler == NULL || node == NULL || watcher == NULL)
        return -1;
    
    int result;
    if (ZOK == (result = zoo_wexists(handler, node, watcher, NULL, NULL)))
        return 0;
    if (result == ZNONODE)
        return 1;
    return -1;
}

int set_server_prefix(char* nameprefix) {
    if (server_name_prefix != NULL)
        free(server_name_prefix);
    if (nameprefix == NULL) {
        server_name_prefix = NULL;
        return 0;
    }
    server_name_prefix = strdup(nameprefix);
    if (server_name_prefix == NULL)
        return -1;
    return 0;
}

char* get_server_prefix() {
    if (server_name_prefix == NULL)
        return NULL;
    return strdup(server_name_prefix);
}

int create_root(zhandle_t *handler, char *path) {
    if (handler == NULL || path == NULL)
        return -1;

    if (ZOK == zoo_exists(handler, path, 0, NULL))
        return 1;
    
    while (ZNONODE == zoo_exists(handler, path, 0, NULL)) {
        int res = zoo_create(handler, path, NULL, -1, 
                    &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        switch (res) {
        case ZNONODE:
            return -1;
            break;
        case ZBADARGUMENTS:
            return -1;
            break;
        
        default:
            continue;
            break;
        }
    }
    return 0;
}

/**
 * <a>https://man7.org/linux/man-pages/man3/getifaddrs.3.html</a>
*/
char* register_server(zhandle_t* handler, char* path, int socket) {
    if (handler == NULL || path == NULL || socket < 0)
        return NULL;
    
    // Copiar o caminho
    char* nodepath = malloc((strlen(path) + strlen(server_name_prefix) + 3)* sizeof(char));
    if (nodepath == NULL)
        return NULL;
    strcpy(nodepath, path);

    // Verificar se tem / no fim do diretorio
    if (nodepath[strlen(path) - 1] != '/')
        strcat(nodepath, "/");

    // Concatenar o prefixo ao diretorio
    if (server_name_prefix != NULL)
        strcat(nodepath, server_name_prefix);

    // Obter o ip e porto atraves do descritor de socket
    struct sockaddr_in localAddress;
    socklen_t addressLength = sizeof(struct sockaddr_in);
    getsockname(socket, (struct sockaddr*)&localAddress, &addressLength);

    // Converter o porto para string
    short port = ntohs(localAddress.sin_port);
    char port_str[20];
    snprintf(port_str, sizeof(port_str), "%d", port);

    // Concatenar o ip e porto
    char sock[32];
    strcpy(sock, inet_ntoa(localAddress.sin_addr));
    strcat(sock, ":");
    strcat(sock, port_str);

    int socket_len = strlen(sock) + 1;

    // Preparar o buffer do nome retornado
    int node_name_len = 1024;
    char* node_name = malloc(node_name_len * sizeof(char));

    // Criar um no efemero
    int ret;
    if ((ret = zoo_create(handler, nodepath, sock, socket_len, 
        & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, node_name, node_name_len)) != ZOK) {
        free(nodepath);
        return NULL;
    }

    free(nodepath);
    return node_name;
}

// Procura o no imediatamente com o identificador maior
/**
 * \todo
 * Procurar o no com identificador seguinte em vez de 
*/
char* get_next_server(zhandle_t* handler, char* rootpath, char* node, watcher_fn watcher) {
    if (handler == NULL || rootpath == NULL || node == NULL)
        return NULL;
    
    // Alocar memoria para o buffer
    zoo_string* node_list = malloc(sizeof(zoo_string));
    if (node_list == NULL)
        return NULL;
    
    // Obter a lista de nos
    if (ZOK != zoo_wget_children(handler, rootpath, watcher, NULL, node_list)) {
        free(node_list);
        return NULL;
    }

    char* next_node = NULL;
    // Iterar pela lista de nos
    for (size_t i = 0; i < node_list->count; i++) {
        // Alocar espaco para o caminho completo do no
        char* node_fullpath = malloc(strlen(rootpath) + strlen(node_list->data[i]) + 2);
        if (node_fullpath == NULL) { 
            free(node_list);
            return NULL;
        }
        strcpy(node_fullpath, rootpath);

        // Verificar se tem / no fim do diretorio
        if (node_fullpath[strlen(rootpath) - 1] != '/')
            strcat(node_fullpath, "/");
        
        // Concatenar o nome do no ao diretorio raiz
        strcat(node_fullpath, node_list->data[i]);

        // Se o novo e igual ou menor do que o atual
        // strcmp("002", "001") = 1
        if (strcmp(node_fullpath, node) <= 0) {
            free(node_fullpath);
            continue;
        }

        // Salva-guardar o indice do no seguinte
        next_node = node_fullpath;
        break;
    }
    
    // Se nada encontrou
    if (next_node == NULL) {
        free(node_list);
        return NULL;
    }

    // Alocar buffer para obter o conteudo do no
    char* zdata_buf = malloc(ZDATALEN * sizeof(char));
    int zdata_len = ZDATALEN * sizeof(char);
    if (ZOK != zoo_wget(handler, next_node, NULL, NULL, zdata_buf, &zdata_len, NULL)){
        free(zdata_buf);
        free(next_node);
        free(node_list);
        return NULL;
    }

    free(next_node);
    free(node_list);
    return zdata_buf;
}

char* get_head_server(zhandle_t* handler, char* path, watcher_fn watcher) {
    if (handler == NULL || path == NULL)
        return NULL;
    
    // Alocar memoria para o buffer
    zoo_string* node_list = malloc(sizeof(zoo_string));
    if (node_list == NULL)
        return NULL;
    
    // Obter a lista de nos
    if (ZOK != zoo_wget_children(handler, path, watcher, NULL, node_list)) {
        free(node_list);
        return NULL;
    }

    char* head_node = NULL;
    // Iterar pela lista de nos, procurar o no com o menor identificador
    for (size_t i = 0; i < node_list->count; i++) {
        // Alocar espaco para o caminho completo do no
        char* node_fullpath = malloc(strlen(path) + strlen(node_list->data[i]) + 2);
        if (node_fullpath == NULL) { 
            free(node_list);
            return NULL;
        }
        strcpy(node_fullpath, path);

        // Verificar se tem / no fim do diretorio
        if (node_fullpath[strlen(path) - 1] != '/')
            strcat(node_fullpath, "/");
        
        // Concatenar o nome do no ao diretorio raiz
        strcat(node_fullpath, node_list->data[i]);

        // Se nao ainda ha nenhum no com o menor descritor
        if (head_node == NULL) {
            head_node = node_fullpath;
            continue;
        }

        // Se o novo e menor do que o atual
        // strcmp("002", "001") = 1
        if (strcmp(node_fullpath, head_node) < 0) {
            // Liberta o atual e substitui pelo novo
            free(head_node);
            head_node = node_fullpath;
            continue;
        }

        free(node_fullpath);
    }
    
    // Se nada encontrou
    if (head_node == NULL) {
        free(node_list);
        return NULL;
    }

    // Alocar buffer para obter o conteudo do no
    char* zdata_buf = malloc(ZDATALEN * sizeof(char));
    int zdata_len = ZDATALEN * sizeof(char);
    if (ZOK != zoo_wget(handler, head_node, NULL, NULL, zdata_buf, &zdata_len, NULL)){
        free(head_node);
        free(node_list);
        return NULL;
    }

    free(head_node);
    free(node_list);
    return zdata_buf;
}

char* get_tail_server(zhandle_t* handler, char* path, watcher_fn watcher) {
    if (handler == NULL || path == NULL)
        return NULL;
    
    // Alocar memoria para o buffer
    zoo_string* node_list = malloc(sizeof(zoo_string));
    if (node_list == NULL)
        return NULL;
    
    // Obter a lista de nos
    if (ZOK != zoo_wget_children(handler, path, watcher, NULL, node_list)) {
        free(node_list);
        return NULL;
    }

    char* tail_node = NULL;
    // Iterar pela lista de nos, procurar o no com o menor identificador
    for (size_t i = 0; i < node_list->count; i++) {
        // Alocar espaco para o caminho completo do no
        char* node_fullpath = malloc(strlen(path) + strlen(node_list->data[i]) + 2);
        if (node_fullpath == NULL) { 
            free(node_list);
            return NULL;
        }
        strcpy(node_fullpath, path);

        // Verificar se tem / no fim do diretorio
        if (node_fullpath[strlen(path) - 1] != '/')
            strcat(node_fullpath, "/");
        
        // Concatenar o nome do no ao diretorio raiz
        strcat(node_fullpath, node_list->data[i]);

        // Se nao ainda ha nenhum no com o menor descritor
        if (tail_node == NULL) {
            tail_node = node_fullpath;
            continue;
        }

        // Se o novo e maior do que o atual
        // strcmp("002", "001") = 1
        // dst - src
        if (strcmp(node_fullpath, tail_node) > 0) {
            // Liberta o atual e substitui pelo novo
            free(tail_node);
            tail_node = node_fullpath;
            continue;
        }

        free(node_fullpath);
    }
    
    // Se nada encontrou
    if (tail_node == NULL) {
        free(node_list);
        return NULL;
    }

    // Alocar buffer para obter o conteudo do no
    char* zdata_buf = malloc(ZDATALEN * sizeof(char));
    int zdata_len = ZDATALEN * sizeof(char);
    if (ZOK != zoo_wget(handler, tail_node, NULL, NULL, zdata_buf, &zdata_len, NULL)){
        free(tail_node);
        free(node_list);
        return NULL;
    }

    free(tail_node);
    free(node_list);
    return zdata_buf;
}

// #define search_node(zhandle_t, char, watcher_fn) (search_node_dispatcher)(__func__, zhandle_t, char, watcher_fn)

// static void search_node_dispatcher(char const * caller_name, zhandle_t* handler, char* path, watcher_fn watcher) {

// }

// static void (search_node)(zhandle_t* handler, char* path, watcher_fn watcher) {

// }
