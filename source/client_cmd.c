/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "client_cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <strings.h>
#include <termios.h>

struct termios defsettings = {0};

char *history = NULL;

void clear_history() {
    if (history != NULL)
        free(history);
}

/**
 * Shifts all characters in the buffer to the right since the given
 * position.
*/
void shiftr(char *array, int size, int position) {
    if (array == NULL || size <= 0)
        return;
    
    if (position < 0 || position >= size)
        return;

    for (int i = size - 1; i > position; i--) {
        array[i] = array[i - 1];
    }
    array[position] = ' ';
}

/**
 * Shifts all characters in the buffer to the left until the given
 * position, the character at given position will be replaced
 * by the one in the next position.
*/
void shiftl(char *array, int size, int position) {
    if (array == NULL || size <= 0 || position < 0 || position >= size) {
        return;
    }

    int i;
    for (i = position; i < size - 1; i++) {
        array[i] = array[i + 1];
    }
    array[i] = '\0';
}

void render(char *buffer, int buffersize, int cursorpos) {
    printf("\033[s");
    if (cursorpos > 0)
        printf("\033[%dD", cursorpos);
    printf("\033[K");
    printf("%s", buffer);
    printf("\033[u");
}

void set_term_quiet_input() {
    struct termios tc;
    tcgetattr(0, &tc);
    cfmakeraw( &tc );
    tcsetattr(0, TCSANOW, &tc);
}

void print_suggestions(char *buffer, int buffersize) {
    if (strcasecmp(buffer, "g") == 0) {
        printf("\033[s");
        printf(SUGG_GET);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "p") == 0) {
        printf("\033[s");
        printf(SUGG_PUT);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "s") == 0) {
        printf("\033[s");
        printf(SUGG_SIZE);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "p") == 0) {
        printf("\033[s");
        printf(SUGG_PUT);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "st") == 0) {
        printf("\033[s");
        printf(SUGG_STATS);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "d") == 0) {
        printf("\033[s");
        printf(SUGG_DEL);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "q") == 0) {
        printf("\033[s");
        printf(SUGG_QUIT);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "h") == 0) {
        printf("\033[s");
        printf(SUGG_HELP);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "getk") == 0) {
        printf("\033[s");
        printf(SUGG_GKEYS);
        printf("\033[u");
    } else
    if (strcasecmp(buffer, "gett") == 0) {
        printf("\033[s");
        printf(SUGG_GTABLE);
        printf("\033[u");
    }
}

void getinput(char *buffer, int buffersize) {
    if (buffersize <= 0 || buffer == NULL)
        return;
    
    tcgetattr(0, &defsettings);
    set_term_quiet_input();

    int cursorpos = 0;
    int cntpos = -1;

    bool finished = false;
    while (!finished) {

        char ch = getchar();

        switch (ch) {
        case EOF:
        case 3:
            // Ctrl c
            buffer[cursorpos] = '\03';
            cntpos++;
        case '\n':
        case '\r':
            finished = true;
            break;

        case '\033':
            // ESC
            ch = getchar();
            if (ch != '[' && ch != 'O')
                break;
            ch = getchar();
            switch (ch) {
            case 'A':
                // up
                if (history == NULL)
                    break;
                memcpy(buffer, history, strlen(history) > buffersize ? buffersize : strlen(history));
                buffer[strlen(history)] = '\0';
                cntpos = strlen(history);
                if (cursorpos > 0)
                    printf("\033[%dD", cursorpos);
                cursorpos = cntpos + 1;
                printf("\033[%ldC", strlen(history) + 1);
                render(buffer, buffersize, cursorpos);
                break;
            case 'B':
                // down
                break;
            case 'C':
                // right
                if (cursorpos >= 0 && cursorpos < cntpos + 1) {
                    printf("\033[C");
                    cursorpos++;
                    render(buffer, buffersize, cursorpos);
                }
                break;
            case 'D':
                // left
                if (cursorpos > 0) {
                    printf("\033[D");
                    cursorpos--;
                    render(buffer, buffersize, cursorpos);
                }
                break;
            case '3':
                if ((ch = getchar()) != 126)
                    break;
                // delete
                if (cursorpos < cntpos + 1 && cursorpos >= 0) {
                    shiftl(buffer, buffersize, cursorpos);
                    cntpos--;
                    render(buffer, buffersize, cursorpos);
                }
                break;
            default:
                break;
            }
            break;
        
        case '\177':
            // backspace
            // Se cursor esta no fim do input
            if (cursorpos == cntpos+1 && cursorpos > 0) {
                cursorpos--;
                buffer[cntpos] = '\0';
                cntpos--;
                printf("\033[D");
                render(buffer, buffersize, cursorpos);
            // Se o cursor estiver no meio do input
            } else if (cursorpos > 0 && cursorpos != cntpos) {
                shiftl(buffer, buffersize, cursorpos-1);
                cntpos--;
                cursorpos--;
                printf("\033[D");
                render(buffer, buffersize, cursorpos);
            
            } else if (cursorpos == 1 && cntpos == 0) {
                buffer[cntpos] = '\0';
                cntpos = -1;
                cursorpos = 0;
            }   
            break;
        
        default:
            // Se o buffer estiver vazio
            if (cntpos < 0) {
                cntpos = 0;
                buffer[cntpos] = ch;
                cursorpos = 1;
                buffer[cursorpos] = '\0';
                printf("\033[C");
                render(buffer, buffersize, cursorpos);
            // Se o cursor esta no fim do input
            } else if (cursorpos == cntpos + 1 && cursorpos < buffersize - 1) {
                buffer[cursorpos] = ch;
                cntpos++;
                cursorpos++;
                buffer[cursorpos] = '\0';
                printf("\033[C");
                render(buffer, buffersize, cursorpos);
            // Se o cursor esta no meio do input
            } else if (cursorpos <= cntpos && cursorpos >= 0) {
                shiftr(buffer, buffersize, cursorpos);
                buffer[cursorpos] = ch;
                cursorpos++;
                cntpos++;
                printf("\033[C");
                render(buffer, buffersize, cursorpos);
            } else if (cursorpos == buffersize - 1) {
                printf("\033[s");
                printf(ERROR_MAX_INPUT);
                printf("\033[u");
            }
            if (strcasecmp(buffer, "authors") == 0) {
                printf("\033[s");
                printf(AUTHORS);
                printf("\033[u");
            }
            print_suggestions(buffer, buffersize);
            break;
        }
    }

    clear_history();
    history = strdup(buffer);

    buffer[cntpos+1] = '\0';
    printf("\r\n");   
    tcsetattr(0, TCSANOW, &defsettings);
}