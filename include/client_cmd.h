/**
 * SD-07
 * 
 * Xiting Wang      
 * Goncalo Pinto    
 * Guilherme Wind   
*/

#ifndef _CLIENT_CMD_H
#define _CLIENT_CMD_H

#define ERROR_MAX_INPUT "\033[0;31m(Max input size reached!)\033[0m"

#define SUGG_GET "\033[2met <key>\033[0m"
#define SUGG_PUT "\033[2mut <key> <value>\033[0m"
#define SUGG_DEL "\033[2mel <key>\033[0m"
#define SUGG_SIZE "\033[2mize\033[0m"
#define SUGG_STATS "\033[2mats\033[0m"
#define SUGG_GKEYS "\033[2meys\033[0m"
#define SUGG_GTABLE "\033[2mable\033[0m"
#define SUGG_HELP "\033[2melp\033[0m"
#define SUGG_QUIT "\033[2muit\033[0m"

#define AUTHORS " \033[2mXiting Wang<58183>, Gonçalo Pinto<58178>, Guilherme Wind<58640>\033[0m"

void clear_history();

void shiftr(char *array, int size, int position);

void shiftl(char *array, int size, int position);


/**
 * Atualizar e imprimir o texto no ecra.
*/
void render(char *buffer, int buffersize, int cursorpos);

/**
 * Coloca o terminal no modo raw para pode ler
 * cada tecla que o utilizador pressiona.
*/
void set_term_quiet_input();

/**
 * Lê o input o do utilizador pela linha de comandos
 * e guarda-lo no buffer passado no argumento.
 * \param buffer
 *      Buffer para guardar o input.
 * \param buffersize
 *      Tamanho do buffer.
*/
void getinput(char *buffer, int buffersize);

#endif