#ifndef SERVIDOR_CHAT_HPP
#define SERVIDOR_CHAT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "libtslog.hpp"
#include "thread_safe_queue.hpp"

struct InfoCliente {
    int socket_fd;
    std::string nome_usuario;
    std::string endereco_ip;
    bool ativo;

    InfoCliente(int fd, const std::string& ip)
        : socket_fd(fd), endereco_ip(ip), ativo(true) {}
};

class ServidorChat {
private:
    int socket_servidor{-1};
    int porta;
    std::atomic<bool> executando;
    std::shared_ptr<ThreadSafeLogger> logger;

    std::vector<std::shared_ptr<InfoCliente>> clientes;
    std::mutex mutex_clientes;

    ThreadSafeQueue<std::string> fila_mensagens;

    std::thread thread_broadcast;

    void configurar_socket() {
        socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_servidor < 0) {
            logger->critical("Erro ao criar socket do servidor");
            throw std::runtime_error("Falha ao criar socket");
        }

        int opcao = 1;
        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR,
                       &opcao, sizeof(opcao)) < 0) {
            logger->error("Erro ao configurar SO_REUSEADDR");
        }

        struct sockaddr_in endereco_servidor;
        std::memset(&endereco_servidor, 0, sizeof(endereco_servidor));
        endereco_servidor.sin_family = AF_INET;
        endereco_servidor.sin_addr.s_addr = INADDR_ANY;
        endereco_servidor.sin_port = htons(porta);

        if (bind(socket_servidor, (struct sockaddr*)&endereco_servidor,
                 sizeof(endereco_servidor)) < 0) {
            logger->critical("Erro ao fazer bind na porta " + std::to_string(porta));
            close(socket_servidor);
            throw std::runtime_error("Falha no bind");
        }

        if (listen(socket_servidor, 10) < 0) {
            logger->critical("Erro ao escutar conexões");
            close(socket_servidor);
            throw std::runtime_error("Falha no listen");
        }

        logger->info("Servidor configurado na porta " + std::to_string(porta));
    }

    void processar_broadcast() {
        logger->info("Thread de broadcast iniciada");

        while (executando) {
            auto msg_opcional = fila_mensagens.pop();
            if (!msg_opcional.has_value()) continue;

            std::string mensagem = msg_opcional.value();

            std::lock_guard<std::mutex> lock(mutex_clientes);
            for (auto& cliente : clientes) {
                if (cliente->ativo) {
                    ssize_t enviado = send(cliente->socket_fd, mensagem.c_str(),
                                          mensagem.length(), MSG_NOSIGNAL);
                    if (enviado < 0) {
                        logger->warning("Erro ao enviar para " + cliente->nome_usuario);
                        cliente->ativo = false;
                    }
                }
            }
        }

        logger->info("Thread de broadcast encerrada");
    }

    void tratar_cliente(std::shared_ptr<InfoCliente> cliente) {
        char buffer[4096];

        // Receber nome do usuário
        ssize_t bytes = recv(cliente->socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            logger->warning("Cliente desconectou antes de enviar nome");
            close(cliente->socket_fd);
            return;
        }

        buffer[bytes] = '\0';
        cliente->nome_usuario = std::string(buffer);

        logger->info("Cliente conectado: " + cliente->nome_usuario +
                    " (" + cliente->endereco_ip + ")");

        std::string msg_boas_vindas = "Bem-vindo ao chat, " +
                                     cliente->nome_usuario + "!\n";
        send(cliente->socket_fd, msg_boas_vindas.c_str(),
             msg_boas_vindas.length(), 0);

        std::string msg_entrada = "[SERVIDOR] " + cliente->nome_usuario +
                                 " entrou no chat\n";
        fila_mensagens.push(msg_entrada);

        while (executando && cliente->ativo) {
            bytes = recv(cliente->socket_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes <= 0) {
                logger->info("Cliente desconectado: " + cliente->nome_usuario);
                cliente->ativo = false;
                break;
            }

            buffer[bytes] = '\0';
            std::string mensagem_recebida(buffer);

            logger->info("Mensagem de " + cliente->nome_usuario + ": " +
                        mensagem_recebida);

            std::string msg_formatada = "[" + cliente->nome_usuario + "] " +
                                       mensagem_recebida;
            fila_mensagens.push(msg_formatada);
        }

        std::string msg_saida = "[SERVIDOR] " + cliente->nome_usuario +
                               " saiu do chat\n";
        fila_mensagens.push(msg_saida);

        close(cliente->socket_fd);
        remover_cliente(cliente);
    }

    void remover_cliente(std::shared_ptr<InfoCliente> cliente) {
        std::lock_guard<std::mutex> lock(mutex_clientes);
        clientes.erase(
            std::remove_if(clientes.begin(), clientes.end(),
                [&cliente](const std::shared_ptr<InfoCliente>& c) {
                    return c->socket_fd == cliente->socket_fd;
                }),
            clientes.end()
        );
        logger->info("Cliente removido da lista: " + cliente->nome_usuario);
    }

public:
    ServidorChat(int porta_servidor)
        : porta(porta_servidor), executando(false) {
        logger = std::make_shared<ThreadSafeLogger>("servidor_chat.log",
                                                     LogLevel::DEBUG, true);
        logger->info("ServidorChat inicializado");
    }

    ~ServidorChat() {
        parar();
    }

    void iniciar() {
        configurar_socket();
        executando = true;

        thread_broadcast = std::thread(&ServidorChat::processar_broadcast, this);

        logger->info("Servidor iniciado. Aguardando conexões...");

        while (executando) {
            struct sockaddr_in endereco_cliente;
            socklen_t tamanho_endereco = sizeof(endereco_cliente);

            int socket_cliente = accept(socket_servidor,
                                       (struct sockaddr*)&endereco_cliente,
                                       &tamanho_endereco);

            if (socket_cliente < 0) {
                if (executando) {
                    logger->error("Erro ao aceitar conexão");
                }
                continue;
            }

            std::string ip_cliente = inet_ntoa(endereco_cliente.sin_addr);
            auto novo_cliente = std::make_shared<InfoCliente>(socket_cliente, ip_cliente);

            {
                std::lock_guard<std::mutex> lock(mutex_clientes);
                clientes.push_back(novo_cliente);
            }

            std::thread(&ServidorChat::tratar_cliente, this, novo_cliente).detach();
        }
    }

    void parar() {
        if (!executando) return;

        logger->info("Encerrando servidor...");
        executando = false;

        {
            std::lock_guard<std::mutex> lock(mutex_clientes);
            for (auto& cliente : clientes) {
                close(cliente->socket_fd);
            }
            clientes.clear();
        }

        fila_mensagens.shutdown();
        if (thread_broadcast.joinable()) {
            thread_broadcast.join();
        }

        if (socket_servidor >= 0) close(socket_servidor);
        logger->info("Servidor encerrado");
    }
};

#endif // SERVIDOR_CHAT_HPP
