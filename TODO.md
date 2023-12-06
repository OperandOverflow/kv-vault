## TODO

### `zk_adaptor`
- Obter o ip da maquina local (done)
- Adaptar ao `ZDATA_NOT_FOUND` 

### `replica_server_table`
- Sincronizar a tabela com o servidor anterior (done)

### `replica_client_table`
- Definir estrutura `c_rptable_t` (done)
- Definir funcoes que a inicializam e manipulam (done)
- rptable_stats retornar estatisticas dos 2 servidores

### `table_client`
- Usar o modulo `replica_client_table` (done)
- Testes

## `table_server`
- Arranjar 2 servidores ir abaixo quando um vai abaixo

### `README`
- Completar a documentacao

### Corrigir erros
- Para o input `put æßð€łŧ€§££@ł adeus` dá erro de segmentação no cliente e nos servidores

### Opcional
- Interface