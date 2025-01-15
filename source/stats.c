/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#include "stats.h"
#include "synchronization.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// =========================================================
//                  Operacoes de escrita
// =========================================================

stats_t *stats_init() {
    // Obter a estrutura para a gestao de concorrencia
    rwcctrl_t *ctrl = cctrl_init();
    if (ctrl == NULL)
        return NULL;
    
    // Alocar espaco para a estrutura stats_t
    stats_t *statsptr = malloc(sizeof(stats_t));
    if (statsptr == NULL) {
        cctrl_destroy(ctrl);
        return NULL;
    }
        
    
    // Inicializar a estrutura
    stats_t stats = {0, 0, 0, ctrl};

    // Copiar para o espaco alocado
    memcpy(statsptr, &stats, sizeof(stats_t));

    return statsptr;
}

stats_t *stats_init_args(int op, long time, int client) {
    // Obter a estrutura para a gestao de concorrencia
    rwcctrl_t *ctrl = cctrl_init();
    if (ctrl == NULL)
        return NULL;
    
    // Alocar espaco para a estrutura stats_t
    stats_t *statsptr = malloc(sizeof(stats_t));
    if (statsptr == NULL) {
        cctrl_destroy(ctrl);
        return NULL;
    }
    
    // Inicializar a estrutura
    stats_t stats = {op, time, client, ctrl};

    // Copiar para o espaco alocado
    memcpy(statsptr, &stats, sizeof(stats_t));

    return statsptr;
}

int stats_inc_op(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    write_begin(stats->cctrl);
    stats->n_op++;
    write_end(stats->cctrl);
    return 0;
}

int stats_add_time(stats_t *stats, long time) {
    if (stats == NULL || 
        stats->cctrl == NULL ||
        time < 0)
        return -1;
    write_begin(stats->cctrl);
    stats->time_lasted += time;
    write_end(stats->cctrl);
    return 0;
}

int stats_inc_client(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    write_begin(stats->cctrl);
    stats->n_client++;
    write_end(stats->cctrl);
    return 0;
}

int stats_dec_client(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    write_begin(stats->cctrl);
    stats->n_client--;
    write_end(stats->cctrl);
    return 0;
}

int stats_op_finish(stats_t *stats, long time) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    write_begin(stats->cctrl);
    stats->n_op++;
    stats->time_lasted += time;
    write_end(stats->cctrl);
    return 0;
}

stats_t *stats_dup(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return NULL;
    stats_t *new_stats = NULL;
    read_begin(stats->cctrl);
    new_stats = stats_init_args(stats->n_op, stats->time_lasted, stats->n_client);
    read_end(stats->cctrl);
    if (new_stats == NULL)
        return NULL;
    return new_stats;
}

int stats_destroy(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    int result = 0;
    if (cctrl_destroy(stats->cctrl) != 0)
        result = -1;
    free(stats);
    return result;
}

// =========================================================
//                  Operacoes de leitura
// =========================================================

int stats_get_n_op(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    
    read_begin(stats->cctrl);
    int num_op = stats->n_op;
    read_end(stats->cctrl);

    return num_op < 0 ? -1 : num_op;
}

long stats_get_time_lasted(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;

    read_begin(stats->cctrl);
    long time = stats->time_lasted;
    read_end(stats->cctrl);

    return time < 0 ? -1 : time;
}

int stats_get_n_client(stats_t *stats) {
    if (stats == NULL || stats->cctrl == NULL)
        return -1;
    
    read_begin(stats->cctrl);
    int num_client = stats->n_client;
    read_end(stats->cctrl);

    return num_client < 0 ? -1 : num_client;
}