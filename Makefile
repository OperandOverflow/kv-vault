# /**
#  * SD-07
#  * 
#  * Xiting Wang      
#  * Goncalo Pinto    
#  * Guilherme Wind   
# */

BIN_DIR = binary
INC_DIR = include
OBJ_DIR = object
SRC_DIR = source
LIB_DIR = lib
DEP_DIR = dependencies

CC = gcc
CFLAGS = -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR) -g
ZFLAGS = -D THREADED
LIBFLAGS = -L$(LIB_DIR) -ltable -lprotobuf-c -lzookeeper_mt
ARFLAGS = -rcs

PROTOC = protoc-c
PROTOCFLAGS = --c_out=.
PROTONAME = sdmessage

# Objetos para formar a biblioteca(nao sao para remover)
LIB_OBJ = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
# Objetos gerados
TARGET_OBJ = $(filter-out $(LIB_OBJ), $(wildcard $(OBJ_DIR)/*.o))

# Nome da biblioteca
LIB = libtable

# Nome dos executaveis
TABLE_CLIENT = $(BIN_DIR)/table_client
TABLE_SERVER = $(BIN_DIR)/table_server


# Fontes e objetos do cliente
CLIENT_SRC = $(SRC_DIR)/sdmessage.pb-c.c $(SRC_DIR)/network_client.c $(SRC_DIR)/client_stub.c $(SRC_DIR)/table_client.c $(SRC_DIR)/message.c $(SRC_DIR)/stats.c $(SRC_DIR)/synchronization.c $(SRC_DIR)/zk_adaptor.c $(SRC_DIR)/replica_client_table.c $(SRC_DIR)/client_cmd.c
CLIENT_OBJ = $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(CLIENT_SRC))

# Fontes e objetos do servidor
SERVER_SRC = $(SRC_DIR)/sdmessage.pb-c.c $(SRC_DIR)/network_server.c $(SRC_DIR)/network_client.c $(SRC_DIR)/table_skel.c $(SRC_DIR)/client_stub.c $(SRC_DIR)/table_server.c $(SRC_DIR)/message.c $(SRC_DIR)/stats.c $(SRC_DIR)/synchronization.c $(SRC_DIR)/zk_adaptor.c $(SRC_DIR)/replica_server_table.c
SERVER_OBJ = $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SERVER_SRC))

# Compilar tudo
all: $(LIB_DIR)/$(LIB).a $(TABLE_CLIENT) $(TABLE_SERVER)

# Compilacao biblioteca
$(LIB_DIR)/$(LIB).a: $(LIB_OBJ)
	ar $(ARFLAGS) $@ $^

# Compilacao cliente
$(TABLE_CLIENT): $(LIB_DIR)/$(LIB).a $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(ZFLAGS) -o $(TABLE_CLIENT) $(CLIENT_OBJ) $(LIBFLAGS)

# Compilacao servidor
$(TABLE_SERVER): $(LIB_DIR)/$(LIB).a $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(ZFLAGS) -o $(TABLE_SERVER) $(SERVER_OBJ) $(LIBFLAGS)

# Compilar objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar protobuf
$(SRC_DIR)/$(PROTONAME).pb-c.c: $(PROTONAME).proto
	$(PROTOC) $(PROTOCFLAGS) $(PROTONAME).proto
	mv $(PROTONAME).pb-c.c $(SRC_DIR)
	mv $(PROTONAME).pb-c.h $(INC_DIR)



include $(wildcard $(DEP_DIR)/*.d)
	
clean:
	rm -rf $(TARGET_OBJ) $(BIN_DIR)/* $(LIB_DIR)/* $(DEP_DIR)/*