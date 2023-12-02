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

int set_server_prefix(char* nameprefix) {
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

    // Criar o no efemero
    int ret;
    if ((ret = zoo_create(handler, nodepath, sock, socket_len, 
        & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, node_name, node_name_len)) != ZOK) {
        free(nodepath);
        return NULL;
    }

    free(nodepath);
    return node_name;
}

char* get_head_server(zhandle_t* handler, char* path, watcher_fn watcher) {
    /* \todo */
    return NULL;
}