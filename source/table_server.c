/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "table.h"
#include "table_skel.h"
#include "zk_adaptor.h"
#include "network_server.h"

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
struct table_t *next_table;

// Variaveis globais para ZooKeeper
const char *zsocket = "127.0.0.1:2181";
zhandle_t *handler;
char *zroot_path = "/table_servers";
char* znode;

void inthandler() {
    free(znode);
    network_server_close(sockt);
    table_skel_destroy(table);
    zookeeper_close(handler);
    exit(-1);
}

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
		} else { 
            inthandler();
		}
	} 
}

int main(int argc, char ** argv) {
    if (argc != 3 && argc != 4) {
        printf("Wrong number of arguments!\n");
        printf("Usage: <port> <table size> or <port> <table size> <zoo ip>:<zoo port>\n");
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

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    // Ligar ao ZooKeeper
    if ((handler = zookeeper_init(zsocket, connection_watcher, 200000, 0, NULL, 0)) == NULL) {
        perror("Error while connecting to ZooKeeper!\n");
        network_server_close(sockt);
        table_skel_destroy(table);
        return -1;
    }

    // Tentar criar o no raiz
    int new_root_len = 1024;
	char *created_zroot_path = malloc(new_root_len * sizeof(char));
    while (ZNONODE == zoo_exists(handler, zroot_path, 0, NULL)) {
        if (ZOK != zoo_create(handler, zroot_path, NULL, -1, 
            & ZOO_OPEN_ACL_UNSAFE, 0, created_zroot_path, new_root_len)) {
            printf("Error creating root!\n");
            network_server_close(sockt);
            table_skel_destroy(table);
            zookeeper_close(handler);
            return -1;
        }
    }

    set_server_prefix("server");

    // Criar um no efemero
    if ((znode = register_server(handler, zroot_path, sockt)) == NULL) {
        perror("Failed to register server on ZK!");
        network_server_close(sockt);
        table_skel_destroy(table);
        zookeeper_close(handler);
        return -1;
    }

    // Atender clientes (so nos dias uteis, das 9h ate as 16h)
    network_main_loop(sockt, table);
    
    free(znode);

    network_server_close(sockt);
    table_skel_destroy(table);
    zookeeper_close(handler);
    return 0;
}