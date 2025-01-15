/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#ifndef _SYNCHRONIZATION_H
#define _SYNCHRONIZATION_H

#include <pthread.h>

/**
 * Uma estrutura que contem ferramentas da gestao de 
 * concorrencia para problemas de single writer, multiple
 * readers usando variaveis condicionais.
*/
typedef struct rwconcurrency_ctrl_t {
    pthread_mutex_t *rwmutex;   /* read-write mutex */
    pthread_cond_t *rwcond;     /* variavel condicional */
    int num_readers;            /* numero de leitores */
    int num_writers;            /* numero de escritores */
} rwcctrl_t;

/**
 * Cria uma estrutura rwcctrl_t e inicializa-a com valores
 * todos a 0.
 * \return
 *      Apontador a estrutura ou NULL em caso de erro.
*/
rwcctrl_t *cctrl_init();

/**
 * Destroi a estrutura, libertando todos os recursos.
 * \param ctrl
 *      Estrutura para ser destruida.
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int cctrl_destroy(rwcctrl_t *ctrl);

/**
 * Inicia o processo de leitura, fazendo lock dos mutexes adequados.
 * \param rwmutex
 *      Mutex para leitura e escrita
 * \param n_readers
 *      Numero de leitores de momento
 * \param rmutex
 *      Mutex para aceder a variavel n_readers
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int read_begin(rwcctrl_t *ctrl);

/**
 * Termina o processo de leitura, fazendo unlock nos mutexes adequados.
 * \param rwmutex
 *      Mutex para leitura e escrita
 * \param n_readers
 *      Numero de leitores de momento
 * \param rmutex
 *      Mutex para aceder a variavel n_readers
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int read_end(rwcctrl_t *ctrl);

/**
 * Inicia o processo de escrita, fazendo lock dos mutexes adequados
 * \param rwmutex
 *      Mutex para leitura e escrita
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int write_begin(rwcctrl_t *ctrl);

/**
 * Termina o processo de escrita, fazendo unlock nos mutexes adequados.
 * \param rwmutex
 *      Mutex para leitura e escrita
 * \return 
 *      0 (OK) ou -1 em caso de erro.
*/
int write_end(rwcctrl_t *ctrl);

#endif