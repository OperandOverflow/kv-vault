## Projeto fase 4 Sistemas Distribuídos 2023-24

### Grupo 7

### Overview
Esta fase do projeto consiste na replicação de servidores e na utilização do ZooKeeper para guardar as informações sobre a topologia da rede dos servidores replicados. No fundo é uma aproximação aos sistemas distribuídos que existem na realidade.
- Ferramentas: ZooKeeper, Threads, Mutex, Protobuf, C, GCC, GDB, Valgrind, VS Code, Linux APIs, POSIX
- Última atualização em 6 Dezembro, 2023.

### Autores
- Gonçalo Pinto -   [@fc58178](fc58178@alunos.fc.ul.pt)
- Guilherme Wind -  [@fc58640](fc58640@alunos.fc.ul.pt)
- Xiting Wang -     [@fc58183](fc58183@alunos.fc.ul.pt)


### Utilização
1. ### Compilação
    A compilação do projeto requere ambiente Linux, com compilador ``gcc`` e as bibliotecas ``protobuf`` e ``zookeeper`` instalados. 
    - Crie os diretórios ``binary``, ``lib`` e ``dependencies`` caso estes não existirem no diretório raíz do projeto. 
    - Execute ``make clean`` para garantir que não há ficheiros restantes.
    - Execute ``make`` ou ``make all`` para compilar a biblioteca e os executáveis do cliente e do servidor.
    - Os executáveis serão gerados no diretório ``binary``.
2. ### Execução
    - #### Table_server
        A execução do servidor é feita com o seguinte comando:
        ```
        ./binary/table_server <port> <table size> [<zookeeper ip>:<zookeeper port>]
        ```
        onde `<port>` é número do porto a qual o servidor estará a esperar pelas ligações dos clientes e `<table size>` é o tamanho com o qual é inicializado a tabela. O último argumento `<zookeeper ip>:<zookeeper port>` é opcional e quando este argumento não é fornecido, o servidor vai tentar estabelecer ligação ao ZooKeeper através do socket `127.0.0.1:2181`, desta forma caso o ZooKeeper se encontre noutra máquina ou numa interface de rede diferente, será necessário usar este parâmetro para passar o socket.
    - #### Table_client
        A execução do servidor é feita com o seguinte comando:
        ```
        ./binary/table_client <zookeeper ip>:<zookeeper port>
        ```

### Implementação
A implementação do projeto é baseada na fase anterior e usa como a diretriz o [enunciado](https://moodle.ciencias.ulisboa.pt/mod/resource/view.php?id=223064), seguindo as recomendações e cumprindo os requesitos. 

Durante o desenvolvimento, foi identificado a necessidade de criar módulos específicos para a comunicação com o ZooKeeper e para operações sobre as tabelas replicadas, sendo isso, foram criados os seguintes módulos:

- `zk_adaptor.h` e `zk_adaptor.c`

    Este módulo foi desenvolvido com o intuito de abstrair as funções fornecidas pela biblioteca ZooKeeper e adaptar às necessidades deste projeto, assim aumenta a legibilidade do código e a facilidade de depuração, uma vez que é possível realizar testes unitários às funções do módulo.

- `replica_table.h`

    A interface `replica_table.h` apenas contém tipos e constantes que possam ser usados pelas implementações específicas. Foi criada visto que existem elementos em comum entre as operações sobre 

- `replica_server_table.h` e `replica_server_table.c`

    É uma implementação que usa a interface `replica_table.h`, orientado às necessidades do servidor e fornece uma interface semelhante à `client_stub.h`, abstraíndo a comunicação com o ZooKeeper, assim o utilizador(servidor) não precisa de se preocupar com criação de nó, mudanças de estado no ZooKeeper, entre outros.

- `replica_client_table.h` e `replica_client_table.c`

    Semelhante à `replica_server_table.h`, `replica_client_table.h` também fornece operações semelhantes à `client_stub.h` e é desenhada para conseguir direcionar pedidos de leitura/escrita para servidores diferentes.


### Limitações da implementação
- O desenvolvimento desta fase do projeto usa os ficheiros ``.o`` fornecidos pelos docentes.

- **Foi encontrado fugas de memória na biblioteca do ZooKeeper e a quantidade de memória perdida aumenta com o número de pesquisas no ZooKeeper**, este problema já foi reportado no fórum do ZooKeeper no entanto ainda não é resolvido. Para mais detalhes, veja [aqui](https://issues.apache.org/jira/browse/ZOOKEEPER-4020).

- **Esta implementação do projeto não garante a libertação de todos os recursos à saída da execução em casos de interrupção**, quando o servidor recebe o sinal ``SIGINT``, é indeterminado o estado da execução tanto na thread principal, como nas segundárias, por isso não garante a libertação de todos os recursos alocados.

- As funções que acedem à tabela no módulo `table_skel.c` podem ser melhoradas porque na implementação atual, é necessária a aquisição de mutex antes de chamar qualquer função da tabela. No entanto, esta abordagem pode ser excessiva considerando que operações de escrita realizadas em simultâneo sobre listas diferentes da tabela não afetará a integridade dos dados. Assim, é possível usar uma abordagem semelhante à [statistics_t](#abordagem) onde cada lista da tabela guarda mutex, as alterações numa lista não bloqueia as outras operações de escrita. Desta forma consegue um melhor desempenho.

- Foi preciso alterar assinatura de algumas funções das fases anteriores, nomeadamente `network_main_loop()` e `invoke()` e outras funções auxiliares, para poder passar a tabela replicada como argumento, assim quando é realizada uma operação de escrita, ela é propagada sincronamente para os servidores replicados.