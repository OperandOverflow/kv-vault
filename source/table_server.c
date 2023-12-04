/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "table.h"
#include "table_skel.h"
#include "replica_table.h"
#include "network_server.h"
#include "replica_server_table.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zookeeper/zookeeper.h>

struct table_t *table;
int sockt;

s_rptable_t *repl_table;

void inthandler() {
    rptable_disconnect(repl_table);
    network_server_close(sockt);
    table_skel_destroy(table);
    exit(-1);
}

s_rptable_t *table_watcher() {
    return repl_table;
}

void table_fhandler(int errcode) {
    inthandler();
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        printf("Wrong number of arguments!\n");
        printf("Usage: <port> <table size>\n");
        return -1;
    }
 
    // Obter o numero do porto
    unsigned int port = atoi(argv[1]);
    if (port <= 1023) {
        printf("Port not allowed!\n");
        return -1;
    }

    // Obter o tamanho da tabela
    int tablesize = atoi(argv[2]);
    if (tablesize <= 0) {
        printf("Invalid table size!\n");
        return -1;
    }

    // Definir o tratamento dos sinais
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, inthandler);

    // Inicializar o socket do servidor
    if ((sockt = network_server_init((unsigned short) port)) == -1) {
        perror("Error while initializing server!\n");
        return -1;
    }

    // Inicializar a tabela
    if ((table = table_skel_init(tablesize)) == NULL) {
        perror("Error while initializing table!\n");
        network_server_close(sockt);
        return -1;
    }

    // Inicializar a tabela replicada
    if ((repl_table = rptable_connect(sockt, table_watcher, table_fhandler)) == NULL) {
        perror("Error while initializing replicated table!\n");
        table_skel_destroy(table);
        network_server_close(sockt);
        return -1;
    }

    // Atender clientes (so nos dias uteis, das 9h ate as 16h)
    network_main_loop(sockt, table);
    network_server_close(sockt);
    table_skel_destroy(table);
    return 0;
}