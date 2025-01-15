/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#include "network_client.h"
#include "network_client-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int network_connect(struct rtable_t *rtable) {
    if (rtable == NULL)
        return -1;

    struct sockaddr_in server;

    // Criar socket
    int skt = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (skt < 0) {
        printf(ERROR_SOCKET);
        return -1;
    }
    rtable->sockfd = skt;
    
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);
    

    // Converter o endereco IP para o formato binario
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) < 1) {
        printf(ERROR_INET_PTON);
        close(skt);
        return -1;
    }
    
    // Estabelecer ligacao
    if (connect(skt, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf(ERROR_CONNECT);
        close(skt);
        return -1;
    }
    
    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (rtable == NULL || msg == NULL)
        return NULL;

    // Obter o tamanho da mensagem
    size_t msgsize = message_t__get_packed_size(msg);

    // Alocar espaco para o buffer
    uint8_t *buffer = (uint8_t *) malloc(msgsize);
    if (buffer == NULL) {
        return NULL;
    }
    // Serializar a mensagem para o buffer
    message_t__pack(msg, buffer);
    // Escrever o tamanho
    unsigned short msgsize_bign = htons(msgsize);
    if (write_all(rtable->sockfd, &msgsize_bign, sizeof(msgsize_bign)) != sizeof(msgsize_bign)) {
        printf(ERROR_SEND_SIZE);
        free(buffer);
        return NULL;
    }
    // Escrever o buffer
    if (write_all(rtable->sockfd, (void *)buffer, msgsize) != msgsize) {
        printf(ERROR_SEND_MSG);
        free(buffer);
        return NULL;
    }
    free(buffer);

    // Ler o tamanho da resposta
    uint16_t respsize_bign = 0;
    if (read_all(rtable->sockfd, &respsize_bign, sizeof(respsize_bign)) != sizeof(respsize_bign)) {
        printf(ERROR_READ_SIZE);
        free(buffer);
        return NULL;
    }

    unsigned short respsize = ntohs(respsize_bign);
    // Alocar espaco para a resposta
    uint8_t *respbuffer = malloc(respsize);
    if (respbuffer == NULL) {
        return NULL;
    }
    // Ler a resposta
    if (read_all(rtable->sockfd, respbuffer, respsize) != respsize) {
        printf(ERROR_READ_MSG);
        free(respbuffer);
        return NULL;
    }
    // Deserializar a mensagem
    MessageT *msgptr = message_t__unpack(NULL, respsize, respbuffer);
    free(respbuffer);

    return msgptr;
}

int network_close(struct rtable_t *rtable) {
    return close(rtable->sockfd);
}