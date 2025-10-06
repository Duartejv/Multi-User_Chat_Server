#!/bin/bash

# Script de teste para simular múltiplos clientes
# Requisito da Etapa 2: Scripts de teste (simulação de múltiplos clientes)

NUM_CLIENTES=${1:-3}
SERVIDOR="127.0.0.1"
PORTA="8080"

echo "========== Teste de Múltiplos Clientes =========="
echo "Iniciando $NUM_CLIENTES clientes simultâneos..."
echo "Servidor: $SERVIDOR:$PORTA"
echo ""

# Array para armazenar PIDs dos clientes
declare -a PIDS

# Função para enviar mensagens automaticamente
enviar_mensagens_automaticas() {
    local id=$1
    local num_mensagens=5
    
    sleep 2  # Aguardar conexão
    
    for i in $(seq 1 $num_mensagens); do
        echo "Mensagem $i do Cliente $id"
        sleep 1
    done
    
    sleep 2
    echo "/sair"
}

# Iniciar clientes em background
for i in $(seq 1 $NUM_CLIENTES); do
    echo "Iniciando Cliente $i..."
    (
        echo "TestUser_$i"
        enviar_mensagens_automaticas $i
    ) | ./cliente $SERVIDOR $PORTA &
    
    PIDS[$i]=$!
    sleep 0.5  # Delay entre inicializações
done

echo ""
echo "Todos os clientes iniciados. Aguardando conclusão..."
echo ""

# Aguardar todos os clientes terminarem
for pid in "${PIDS[@]}"; do
    wait $pid
done

echo ""
echo "========== Teste Concluído =========="
echo "Verifique os logs em teste_concorrente.log"
