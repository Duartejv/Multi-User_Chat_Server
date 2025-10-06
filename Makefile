# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I./libtslog -I./comum -I./servidor -I./cliente

# Diretórios
LIBTSLOG_DIR = libtslog
COMUM_DIR = comum
SERVIDOR_DIR = servidor
CLIENTE_DIR = cliente
BUILD_DIR = build

# Arquivos objeto
TSLOG_OBJS = $(BUILD_DIR)/tslog.o
COMUM_OBJS = $(BUILD_DIR)/message.o
SERVIDOR_OBJS = $(BUILD_DIR)/servidor.o $(BUILD_DIR)/main_servidor.o
CLIENTE_OBJS = $(BUILD_DIR)/cliente.o $(BUILD_DIR)/main_cliente.o

# Executáveis
SERVIDOR_BIN = servidor
CLIENTE_BIN = cliente
TEST_CONCURRENT_BIN = test_concurrent_logging

# Alvos principais
all: directories $(SERVIDOR_BIN) $(CLIENTE_BIN) $(TEST_CONCURRENT_BIN)

directories:
	@mkdir -p $(BUILD_DIR)

# Biblioteca tslog
$(BUILD_DIR)/tslog.o: $(LIBTSLOG_DIR)/tslog.cpp $(LIBTSLOG_DIR)/tslog.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Comum
$(BUILD_DIR)/message.o: $(COMUM_DIR)/message.cpp $(COMUM_DIR)/message.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Servidor
$(BUILD_DIR)/servidor.o: $(SERVIDOR_DIR)/servidor.cpp $(SERVIDOR_DIR)/servidor.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/main_servidor.o: $(SERVIDOR_DIR)/main_servidor.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SERVIDOR_BIN): $(TSLOG_OBJS) $(COMUM_OBJS) $(SERVIDOR_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Cliente
$(BUILD_DIR)/cliente.o: $(CLIENTE_DIR)/cliente.cpp $(CLIENTE_DIR)/cliente.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/main_cliente.o: $(CLIENTE_DIR)/main_cliente.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENTE_BIN): $(TSLOG_OBJS) $(COMUM_OBJS) $(CLIENTE_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Teste concorrente
$(TEST_CONCURRENT_BIN): $(TSLOG_OBJS) test_concurrent_logging.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# Testes
test: $(TEST_CONCURRENT_BIN)
	./$(TEST_CONCURRENT_BIN)

test_chat: $(SERVIDOR_BIN) $(CLIENTE_BIN)
	@echo "Execute './servidor' em um terminal e './cliente' em outros terminais"

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(SERVIDOR_BIN) $(CLIENTE_BIN) $(TEST_CONCURRENT_BIN) *.log

# Executar servidor
run_servidor: $(SERVIDOR_BIN)
	./$(SERVIDOR_BIN)

# Executar cliente
run_cliente: $(CLIENTE_BIN)
	./$(CLIENTE_BIN)

.PHONY: all directories test test_chat clean run_servidor run_cliente
