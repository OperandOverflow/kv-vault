/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H
/**
 * Enviar o conteudo para o servidor atraves do socket.
 * \param sock
 *      Descritor do socket.
 * \param buf
 *      Apontador para o buffer onde fica o conteudo.
 * \param len
 *      Tamanho do conteudo a ser enviado.
 * \return
 *      Numero de bytes escritos com sucesso, ou -1 
 *      em caso de erro.
*/
int write_all(int sock, void *buf, int len);

/**
 * Le a resposta do servidor.
 * \param sock
 *      Descritor do socket.
 * \param buf 
 *      Apontador para o buffer para onde ficara guardado 
 *      o conteudo.
 * \param len
 *      Tamanho do buffer.
 * \return
 *      Numero de bytes recebidos, ou -1 em caso de erro.
*/
int read_all(int sock, void *buf, int len);
#endif