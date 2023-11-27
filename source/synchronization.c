/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "synchronization.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

rwcctrl_t *cctrl_init() {
    // Inicializar mutex de leitura e escrita
    pthread_mutex_t *rwmutex = malloc(sizeof(pthread_mutex_t));
    if (rwmutex == NULL)
        goto err_rwm_malloc;
    if (pthread_mutex_init(rwmutex, NULL) != 0)
        goto err_rwm_init;
    
    // Inicializar variavel condicional 
    pthread_cond_t *rwcond = malloc(sizeof(pthread_cond_t));
    if (rwcond == NULL)
        goto err_rwc_malloc;
    if (pthread_cond_init(rwcond, NULL) != 0)
        goto err_rwc_init;

    // Alocar espaco para a estrutura stats_t
    rwcctrl_t *conctrlptr = malloc(sizeof(rwcctrl_t));
    if (conctrlptr == NULL)
        goto err_conctrl_malloc;
    
    rwcctrl_t conctrl = {rwmutex, rwcond, 0, 0};

    memcpy(conctrlptr, &conctrl, sizeof(rwcctrl_t));

    return conctrlptr;
    
    err_conctrl_malloc:
    pthread_cond_destroy(rwcond);
    err_rwc_init:
    free(rwcond);
    err_rwc_malloc:
    pthread_mutex_destroy(rwmutex);
    err_rwm_init:
    free(rwmutex);
    err_rwm_malloc:
    return NULL;
}

int cctrl_destroy(rwcctrl_t *ctrl) {
    if (ctrl == NULL)
        return -1;
    if (ctrl->rwmutex == NULL || ctrl->rwcond == NULL)
        return -1;
    int result = 0;
    if (pthread_mutex_destroy(ctrl->rwmutex) != 0)
        result = -1;
    if (pthread_cond_destroy(ctrl->rwcond) != 0)
        result = -1;
    free(ctrl->rwmutex);
    free(ctrl->rwcond);
    free(ctrl);
    return result;
}

int read_begin(rwcctrl_t *ctrl) {
    if (ctrl == NULL 
        || ctrl->num_readers < 0 
        || ctrl->num_writers < 0 
        || ctrl->rwcond == NULL
        || ctrl->rwmutex == NULL)
        return -1;
    // Obter o mutex
    pthread_mutex_lock(ctrl->rwmutex);
    // Enquanto ainda ha escritor
    while (ctrl->num_writers > 0)
        // Espera na condicao
        pthread_cond_wait(ctrl->rwcond, ctrl->rwmutex);
    // Atualizar o numero de leitores
    ctrl->num_readers++;
    // Libertar o mutex
    pthread_mutex_unlock(ctrl->rwmutex);
    return 0;
    
}

int read_end(rwcctrl_t *ctrl) {
    if (ctrl == NULL 
        || ctrl->num_readers < 0 
        || ctrl->num_writers < 0 
        || ctrl->rwcond == NULL
        || ctrl->rwmutex == NULL)
        return -1;
    pthread_mutex_lock(ctrl->rwmutex);
    ctrl->num_readers--;
    
    if(ctrl->num_readers == 0) 
        pthread_cond_broadcast(ctrl->rwcond);
    
    pthread_mutex_unlock(ctrl->rwmutex);
    return 0;
}

int write_begin(rwcctrl_t *ctrl) {
    if (ctrl == NULL 
        || ctrl->num_readers < 0 
        || ctrl->num_writers < 0 
        || ctrl->rwcond == NULL
        || ctrl->rwmutex == NULL)
        return -1;
        
    pthread_mutex_lock(ctrl->rwmutex);
    // Enquanto ainda ha leitores ou escritores
    while (ctrl->num_readers > 0 ||
           ctrl->num_writers > 0)
        // Espera na condicao
        pthread_cond_wait(ctrl->rwcond, ctrl->rwmutex);

    ctrl->num_writers++;

    pthread_mutex_unlock(ctrl->rwmutex);
    return 0;
}

int write_end(rwcctrl_t *ctrl) {
    if (ctrl == NULL 
        || ctrl->num_readers < 0 
        || ctrl->num_writers < 0 
        || ctrl->rwcond == NULL
        || ctrl->rwmutex == NULL)
        return -1;
    pthread_mutex_lock(ctrl->rwmutex);
    ctrl->num_writers--;
    pthread_cond_broadcast(ctrl->rwcond);
    pthread_mutex_unlock(ctrl->rwmutex);
    return 0;
}
