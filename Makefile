# Configurações do compilador
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG

# Diretórios com novos nomes
CODIGO_DIR = codigo
TESTE_DIR = testes
BUILD_DIR = build

# Arquivos fonte
TSLOG_SRC = $(CODIGO_DIR)/libtslog/tslog.cpp
TSLOG_HDR = $(CODIGO_DIR)/libtslog/tslog.h
TESTE_SRC = $(TESTE_DIR)/test_concurrent_logging.cpp

# Nome do executável (adiciona .exe automaticamente no Windows se necessário)
ifeq ($(OS),Windows_NT)
    EXECUTABLE = $(BUILD_DIR)/test_concurrent_logging.exe
    RM_CMD = del /Q
    MKDIR_CMD = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
    EXECUTABLE = $(BUILD_DIR)/test_concurrent_logging
    RM_CMD = rm -f
    MKDIR_CMD = mkdir -p $(BUILD_DIR)
endif

# Alvos principais
.PHONY: all debug release test clean help

all: debug

# Compilação debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(EXECUTABLE)

# Compilação release
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(EXECUTABLE)

# Compila o executável de teste
$(EXECUTABLE): $(TESTE_SRC) $(TSLOG_SRC) $(TSLOG_HDR)
	$(MKDIR_CMD)
	$(CXX) $(CXXFLAGS) -I$(CODIGO_DIR) $(TESTE_SRC) $(TSLOG_SRC) -o $(EXECUTABLE)

# Executa o teste
test: $(EXECUTABLE)
	@echo "=== Executando teste de logging concorrente ==="
	$(EXECUTABLE)
	@echo "=== Teste concluido! Verifique o arquivo teste_concorrente.log ==="

# Limpeza
clean:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	if exist teste_concorrente.log del teste_concorrente.log
else
	rm -rf $(BUILD_DIR)
	rm -f teste_concorrente.log
endif

# Compilação rápida (sem criar diretório build)
rapido:
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -I$(CODIGO_DIR) $(TESTE_SRC) $(TSLOG_SRC) -o test_concurrent_logging

# Ajuda
help:
	@echo "Alvos disponíveis:"
	@echo "  debug    - Compila com símbolos de debug (padrão)"
	@echo "  release  - Compila otimizado para produção"
	@echo "  test     - Compila e executa o teste"
	@echo "  rapido   - Compilação rápida no diretório atual"
	@echo "  clean    - Remove arquivos gerados"
	@echo "  help     - Mostra esta ajuda"