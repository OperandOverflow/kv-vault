## Projeto fase 3 Sistemas Distribuídos 2023-24

### Grupo 7

### Overview
Esta fase do projeto consiste na adição da capacidade multi-threading e o método ``stats`` à implementação da 2ª fase. Desta forma, o servidor consegue atender vários clientes ao mesmo tempo, lançando uma thread para cada cliente, ao mesmo tempo usando mecanismos de mutex para evitar situações de concorrência e, assim, mantendo a integridade dos dados guardados no servidor.
- Ferramentas: Threads, Mutex, Protobuf, C, GCC, GDB, Valgrind, VS Code, Linux APIs, POSIX
- Última atualização em 5 Novembro, 2023.

### Autores
- Gonçalo Pinto -   [@fc58178](fc58178@alunos.fc.ul.pt)
- Guilherme Wind -  [@fc58640](fc58640@alunos.fc.ul.pt)
- Xiting Wang -     [@fc58183](fc58183@alunos.fc.ul.pt)


### Utilização
1. ### Compilação
    A compilação do projeto requere ambiente Linux, com compilador ``gcc`` e a biblioteca ``protobuf`` instalados. 
    - Crie os diretórios ``binary``, ``lib`` e ``dependencies`` caso estes não existirem no diretório raíz do projeto. 
    - Execute ``make clean`` para garantir que não há ficheiros restantes.
    - Execute ``make`` ou ``make all`` para compilar a biblioteca e os executáveis do cliente e do servidor.
    - Os executáveis serão gerados no diretório ``binary``.
2. ### Execução
    - #### Table_server
        A execução do servidor é feita com o seguinte comando:
        ```
        ./binary/table_server <port> <table size>
        ```
        onde `<port>` é número do porto a qual o servidor estará a esperar pelas ligações dos clientes e `<table size>s` é o tamanho com o qual é inicializado a tabela.
    - #### Table_client
        A execução do servidor é feita com o seguinte comando:
        ```
        ./binary/table_client <server ip>:<server port>
        ```
        onde `<server ip>` é o endereço IP do servidor, caso esteja na mesma máquina, o endereço deve ser 127.0.0.1 e `<server port>` é o número do porto onde o servidor está à escuta.

### Implementação
A implementação do projeto é baseada na fase anterior e usa como a diretriz o [enunciado](https://moodle.ciencias.ulisboa.pt/mod/resource/view.php?id=223060), seguindo as recomendações e cumprindo os requesitos. 

Foi criado um novo ficheiro header `stats.h` onde está definida a nova estrutura `statistics_t` para guardar dados estatísticos do servidor e as assinaturas de funções thread-safe para manipulação desses dados.

<a id="abordagem"></a>
A nova estrutura, `statistics_t`, além de conter campos para guardar dados estatísticos, tem também uma estrutura `rwcctrl_t`, definida no módulo `synchronization.h`, que contém os dados para a gestão de concorrência no acesso aos dados. Esta decisão teve como inspiração o mecanismo de Object Level Lock do Java, que está associada diretamente ao objeto em vez da função que realiza leitura/escrita de dados. Deste modo a mesma função pode ser executada por outras threads ao mesmo tempo desde que não estejam a aceder ao mesmo objeto.

### Limitações da implementação
- O desenvolvimento desta fase do projeto usa os ficheiros ``.o`` fornecidos pelos docentes.
- **Esta implementação do projeto não garante a libertação de todos os recursos à saída da execução em casos de interrupção**, quando o servidor recebe o sinal ``SIGINT``, é indeterminado o estado da execução tanto na thread principal, como nas segundárias, por isso não garante a libertação de todos os recursos alocados.
- As funções que acedem à tabela no módulo `table_skel.c` podem ser melhoradas porque na implementação atual, é necessária a aquisição de mutex antes de chamar qualquer função da tabela. No entanto, esta abordagem pode ser excessiva considerando que operações de escrita realizadas em simultâneo sobre listas diferentes da tabela não afetará a integridade dos dados. Assim, é possível usar uma abordagem semelhante à [statistics_t](#abordagem) onde cada lista da tabela guarda mutex, as alterações numa lista não bloqueia as outras operações de escrita. Desta forma consegue um melhor desempenho.
- Mantivemos as assinaturas de funções das fases anteriores do projeto.