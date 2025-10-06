#!/bin/bash
set -e

echo "Compilando projeto..."
make clean || true
make

echo "Iniciando servidor em background..."
./servidor 8080 &
SERVIDOR_PID=$!

sleep 2

echo "Iniciando 3 clientes simulados..."

(echo "Usuario1"; sleep 1; echo "Ola pessoal!"; sleep 2; echo "sair") | ./cliente 127.0.0.1 8080 &

(echo "Usuario2"; sleep 1; echo "Oi!"; sleep 1; echo "Como vao?"; sleep 2; echo "sair") | ./cliente 127.0.0.1 8080 &

(echo "Usuario3"; sleep 2; echo "Tudo bem aqui!"; sleep 2; echo "sair") | ./cliente 127.0.0.1 8080 &

wait

echo "Encerrando servidor..."
kill $SERVIDOR_PID || true

echo "Teste concluído!"
