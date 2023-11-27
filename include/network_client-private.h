/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

// ==================================================================
//                        Mensagens Erro
// ==================================================================
#define ERROR_SOCKET "\033[0;31m[!] Error network:\033[0m Failed to create socket.\n"

#define ERROR_INET_PTON "\033[0;31m[!] Error network:\033[0m Failed to obtain server address.\n"

#define ERROR_CONNECT "\033[0;31m[!] Error network:\033[0m Failed to connect to server.\n"

#define ERROR_SEND_SIZE "\033[0;31m[!] Error network:\033[0m Failed to send request size.\n"

#define ERROR_SEND_MSG "\033[0;31m[!] Error network:\033[0m Failed to send request.\n"

#define ERROR_READ_SIZE "\033[0;31m[!] Error network:\033[0m Failed to read response size.\n"

#define ERROR_READ_MSG "\033[0;31m[!] Error network:\033[0m Failed to read response.\n"

#define ERROR_WRITE "\033[0;31m[!] Error network:\033[0m Failed to write from pipe"

#define ERROR_READ "\033[0;31m[!] Error network:\033[0m Failed to read from pipe"

#endif