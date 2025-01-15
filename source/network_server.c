/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#include "network_server.h"
#include "table.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "table_skel-private.h"
#include "message-private.h"
#include "network_client-private.h"
#include "replica_table.h"
#include "replica_server_table.h"

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Variaveis globais para as threads poderem aceder
struct table_t *hashtable;
s_rptable_t *replicatedtable;
// Variavel global para imprimir no ecra
pthread_mutex_t printmutex;

// Identificador da thread main
pthread_t mainthread;

/**
 * Funcao auxiliar para imprimir, adicionando a estampilha de tempo
 * e o socket do cliente.
*/
void network_server_print(char* ip, int port, const char *msg, ...) {
    // Tempo
    time_t current_time;
    struct tm * time_info;
    char timeString[9];  // space for "HH:MM:SS\0"
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

    // Thread id
    pthread_t id = pthread_self();

    pthread_mutex_lock(&printmutex);
    printf("\033[1A\033[2K\r");
    
    if (id == mainthread)
        printf("%s - main: ", timeString);
    else 
        printf("%s - \033[4;36m%s\033[0m-\033[4;32m%hu\033[0m: ", timeString, ip, port);

    va_list args;
    va_start(args, msg);
    vfprintf(stdout, msg, args);
    va_end(args);
    printf("\033[1;30;103m Info: \033[30;102m %d active users \033[0m\n", get_num_clients());
    pthread_mutex_unlock(&printmutex);
}

int network_server_init(short port) {

    // Criar um socket
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server_socket < 0) {
        perror("Error while creating socket\n");
        return -1;
    }

    // Definir opcoes do socket
    int sockopt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0) {
        perror("Error on setting socket options!\n");
        close(server_socket);
        return -1;
    }

    // Configurar a estrutura de endereço do servidor
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Ligar o socket ao endereço e porta especificados
    if (bind(server_socket, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Error while binding!\n");
        close(server_socket);
        return -1;
    }

    // Colocar o socket no modo de escuta
    if (listen(server_socket, 0) < 0) {
        perror("Error while listening to the port!\n");
        close(server_socket);
        return -1;
    }

    // Inicializar mutexes para controlo de concorrencia
    if (pthread_mutex_init(&printmutex, NULL) != 0) {
        perror("Error while creating mutex!\n");
        close(server_socket);
        return -1;
    }

    mainthread = pthread_self();

    printf("Server network ready.\n");

    return server_socket;
}

/**
 * Funcao que vai ser executacao por cada thread.
*/
void *thread_loop(void *arg) {
    // Incrementar o numero de clientes ligados
    inc_num_clients();
    // Salva-guardar o file descriptor do socket
    in_port_t sock = *(in_port_t *)arg;
    // Libertar o espaco
    free(arg);

    // Obter o endereco ip a partir do file descriptor
    struct sockaddr_in clientaddr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sock, (struct sockaddr *)&clientaddr, &addr_size);
    char ip[20];
    if (res >= 0)
        strcpy(ip, inet_ntoa(clientaddr.sin_addr));
    unsigned  port = ntohs(clientaddr.sin_port);
    network_server_print(ip, port, "Client connection estabilished!\n");

    // Recebe pedidos do cliente usando a função network_receive
    MessageT *request = network_receive(sock);
    while (request != NULL) {
        network_server_print(ip, port, "Request received.\n");
        // Processa a mensagem na tabela
        if (invoke(request, hashtable, replicatedtable) == -1) {
            message_t__free_unpacked(request, NULL);
            break;
        }
        // Enviar a resposta ao cliente
        if (network_send(sock, request) == -1) {
            message_t__free_unpacked(request, NULL);
            continue;
        }
        network_server_print(ip, port, "Answer sent.\n");
        message_t__free_unpacked(request, NULL);
        // Tentar ler o proximo pedido
        request = network_receive(sock);
    }
    dec_num_clients();
    network_server_print(ip, port, "Client connection closed.\n");
    close(sock);
    return NULL;
}


int network_main_loop(int listening_socket, struct table_t *table, s_rptable_t *rptable) {
    if (table == NULL)
        return -1;
    
    network_server_print(NULL, 0, "Server ready.\n");
    struct sockaddr_in client;
    socklen_t size_client = sizeof(client);
    int connsockfd;
    signal(SIGPIPE, SIG_IGN);
    hashtable = table;
    replicatedtable = rptable;

    // O loop principal continua a aceitar conexões de clientes
    while ((connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client)) != -1) {
        char *ip = inet_ntoa(client.sin_addr);
        network_server_print(NULL, 0, "Client connecting from ip \033[4;36m%s\033[0m, port \033[4;32m%hu\033[0m\n", ip, htons(client.sin_port));

        in_port_t *sock = malloc(sizeof(in_port_t));
        if (sock == NULL) {
            network_server_print(NULL, 0, "Error allocating space for thread!");
            continue;
        }
        *sock = connsockfd;

        // Lancar uma thread
        pthread_t thr;
        if (pthread_create(&thr, NULL, &thread_loop, sock) != 0) {
            network_server_print(NULL, 0, "Error creating thread for client!");
            continue;
        }
        pthread_detach(thr);
    }
    return -1;
}

MessageT *network_receive(int client_socket) {
    unsigned short resqsize_bign; 

    // Ler o tamanho do pedido
    int read_size = read_all(client_socket, &resqsize_bign, sizeof(short)); 
    if (read_size != sizeof(short))
        return NULL;
    
    // Converter para little endian
    unsigned short size = ntohs(resqsize_bign); 

    // Alocar espaco para o pedido
    void * buffer = malloc(size);

    // Ler a mensagem do pedido
    if (read_all(client_socket, buffer, size) != size) {
        free(buffer);
        return NULL;
    }

    MessageT *req = message_t__unpack(NULL, size, buffer);
    free(buffer);

    return req;
}

int network_send(int client_socket, MessageT *msg) {
    // Obter o tamanho da resposta
    int msgsize = message_t__get_packed_size(msg); 
    unsigned short msgsize_bign = htons((short)msgsize); 

    // Enviar o tamanho da resposta
    int write_size = write_all(client_socket, &msgsize_bign, sizeof(short)); 
    if (write_size != sizeof(short))
        return -1;
    
    // Alocar espaco para a mensagem
    void * buffer = malloc(msgsize); 
    if (buffer == NULL)
        return -1;
    message_t__pack(msg, buffer);

    // Enviar a resposta
    write_size = write_all(client_socket, buffer, msgsize); 
    if (write_size != msgsize) {
        free(buffer);
        return -1;
    }
    free(buffer);

    return 0;
}

int network_server_close(int socket) {
    int result;
    if (pthread_mutex_destroy(&printmutex) != 0)
        result = -1;
    if (close(socket) != 0)
        result = -1;
    return result;
}