/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#ifndef _TABLE_CLIENT_PRIVATE_H
#define _TABLE_CLIENT_PRIVATE_H

#include "client_stub.h"
#include "client_stub-private.h"

// ==================================================================
//                            Shell
// ==================================================================
#define CLIENT_SHELL "\033[1;32mtable_client:~/\033[0m$ \033[?12;25h"

// ==================================================================
//                     Mensagens Auxiliares
// ==================================================================
#define AUX_HELP    "\033[0;33m[?] Help:\033[0m\n"\
                    "   `\033[4;93mp\033[0mut` <key> <value>     - Puts the key and value to the table\n"\
                    "   `\033[4;93mg\033[0met` <key>             - Retrieves the value associated with the key\n"\
                    "   `\033[4;93md\033[0mel` <key>             - Deletes the value associated with the key\n"\
                    "   `\033[4;93ms\033[0mize`                  - Gets the number of elements in the table\n"\
                    "   `st\033[4;93ma\033[0mts`                 - Gets the statistics of the server\n"\
                    "   `get\033[4;93mk\033[0meys`               - Retrieves all the keys contained in the table\n"\
                    "   `get\033[4;93mt\033[0mable`              - Retrieves all the keys and values in the table\n"\
                    "   `\033[4;93mq\033[0muit`                  - Closes the connection with the table and quits\n"\
                    "   `\033[4;93mh\033[0melp`                  - Shows all available commands and their usage\n"
                    // "   \033[4m \033[24m"

#define AUX_GET "\033[0;33m[i] Info:\033[0m Key: %s\n"\
                "   Value: %s\n"\
                "   Length: %d\n"
                
#define AUX_SIZE "\033[0;33m[i] Info:\033[0m The table has %d entries.\n"

#define AUX_STATS   "\033[0;33m[i] Info:\033[0m Stats:\n"\
                    "   Completed operations: %d\n"\
                    "   Total time used: %ld µsec\n"\
                    "   Connected users: %d\n"

#define AUX_GETKEYS "\033[0;33m[i] Info:\033[0m Keys:\n"
#define AUX_GETKEYS_LINE "  %s\n"

#define AUX_GETTABLE "\033[0;33m[i] Info:\033[0m Table:\n"
#define AUX_GETTABLE_LINE   "   %s::%s\n"
// ==================================================================
//                        Mensagens Erro
// ==================================================================
#define ERROR_ARGS  "\033[0;31m[!] Error:\033[0m Number of arguments"\
                    " should be 1 with the following format: <server ip>:<server port>.\n"
                
#define ERROR_CONNECTION "\033[0;31m[!] Error:\033[0m Failed to estabilish connection with server.\n"

#define ERROR_UNKNOWN_COMMAND "\033[0;31m[!] Error:\033[0m Unrecognized command! Use `\033[4;93mh\033[0melp` command for help.\n"

#define ERROR_MISSING_ARGS  "\033[0;31m[!] Error:\033[0m Missing %s while calling %s.\n"

#define ERROR_GET "\033[0;31m[!] Error:\033[0m Failed to retrieve the value or nothing associated with the key.\n"

#define ERROR_DEL "\033[0;31m[!] Error:\033[0m Failed to delete the value associated with the key.\n"

#define ERROR_SIZE  "\033[0;31m[!] Error:\033[0m Failed to obtain the size of the table.\n"

#define ERROR_STATS  "\033[0;31m[!] Error:\033[0m Failed to obtain the statisticas of the server.\n"

#define ERROR_GETKEYS "\033[0;31m[!] Error:\033[0m Failed to retrieve keys.\n"

#define ERROR_GETTABLE  "\033[0;31m[!] Error:\033[0m Failed to retrieve table.\n"
// ==================================================================
//                      Mensagens Sucesso
// ==================================================================
#define SUCCESS_OPERATION   "\033[0;32m[√] Success:\033[0m %s executed successfully.\n"

#define SUCCESS_EXIT    "Thanks for using.\n"
// ==================================================================
//                      Funcoes Auxiliares
// ==================================================================

/**
 * Faz tratamento de sinal de interrupcao, libertando
 * os recursos.
*/
void inthandler();

/**
 * Coloca uma nova entrada ou substitui a ja
 * existente na tabela.
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \param key
 *      Apontador para a chave.
 * \param value
 *      Apontador para o valor a ser colocado.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int put(struct rtable_t *rtable, char *key, char* value);



/**
 * Pede ao servidor o valor associado a uma chave
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \param key
 *      Apontador para a chave.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int get(struct rtable_t *rtable, char *key);


/**
 * Apaga uma entrada da tabela
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \param key
 *      Apontador para a chave.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int delete(struct rtable_t *rtable, char *key);

/**
 * Imprime o tamanho da tabela.
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int size(struct rtable_t *rtable);

/**
 * Imprime as estatisticas do servidor e da tabela.
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int stats(struct rtable_t *rtable);

/**
 * Imprime todas as chaves contidas na tabela.
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/  
int getkeys(struct rtable_t *rtable);

/**
 * Imprime todas as entradas da tabela.
 * \param rtable
 *      Estrutura rtable_t que contem informacao da conexao.
 * \return
 *      0 se a operacao foi concluida com sucesso, -1
 *      caso contrario.
*/
int gettable(struct rtable_t *rtable);

#endif