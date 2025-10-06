# Servidor de Chat Multiusuário Concorrente (TCP)

## Descrição
Sistema completo de chat cliente-servidor implementando conceitos avançados de programação concorrente em C++, incluindo threads, exclusão mútua, variáveis de condição, monitores e sockets TCP.

## Estrutura do Projeto

- libtslog.hpp: Biblioteca de logging thread-safe com timestamps e níveis de log
- thread_safe_queue.hpp: Monitor implementando fila thread-safe com variáveis de condição
- servidor_chat.hpp: Servidor TCP concorrente com broadcast de mensagens
- servidor_main.cpp: Ponto de entrada do servidor
- cliente_chat.cpp: Cliente CLI com interface interativa
- teste_libtslog.cpp: Teste de concorrência com múltiplas threads
- CMakeLists.txt / Makefile: Scripts de build
- teste_multiplos_clientes.sh: Script para testes automatizados

## Compilação

### Usando CMake:
```
mkdir build
cd build
cmake ..
make
```

### Usando Makefile:
```
make
```

## Execução

### Iniciar o Servidor:
```
./servidor [porta]
# Exemplo: ./servidor 8080
```

### Iniciar Cliente:
```
./cliente [ip_servidor] [porta]
# Exemplo: ./cliente 127.0.0.1 8080
```

### Teste Automatizado:
```
chmod +x teste_multiplos_clientes.sh
./teste_multiplos_clientes.sh
```
