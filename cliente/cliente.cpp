#include "cliente.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

ClienteTCP::ClienteTCP(const std::string& endereco, int porta)
    : socket_cliente_(-1), endereco_servidor_(endereco), porta_servidor_(porta), conectado_(false) {
}

ClienteTCP::~ClienteTCP() {
    desconectar();
}

bool ClienteTCP::conectar_ao_servidor() {
    // Criar socket
    socket_cliente_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_cliente_ < 0) {
        TSLog::error("Erro ao criar socket do cliente");
        return false;
    }
    
    // Configurar endereço do servidor
    struct sockaddr_in endereco_servidor;
    std::memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(porta_servidor_);
    
    if (inet_pton(AF_INET, endereco_servidor_.c_str(), &endereco_servidor.sin_addr) <= 0) {
        TSLog::error("Endereço inválido: " + endereco_servidor_);
        close(socket_cliente_);
        return false;
    }
    
    // Conectar ao servidor
    if (connect(socket_cliente_, (struct sockaddr*)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
        TSLog::error("Erro ao conectar ao servidor " + endereco_servidor_ + ":" + std::to_string(porta_servidor_));
        close(socket_cliente_);
        return false;
    }
    
    return true;
}

bool ClienteTCP::conectar(const std::string& nome_usuario) {
    if (conectado_.load()) {
        TSLog::warning("Cliente já está conectado");
        return false;
    }
    
    nome_usuario_ = nome_usuario;
    
    if (!conectar_ao_servidor()) {
        return false;
    }
    
    conectado_.store(true);
    
    // Iniciar thread de recebimento
    thread_recebimento_ = std::thread(&ClienteTCP::processar_mensagens_recebidas, this);
    
    TSLog::info("Conectado ao servidor como " + nome_usuario_);
    
    return true;
}

void ClienteTCP::desconectar() {
    if (!conectado_.exchange(false)) {
        return;
    }
    
    TSLog::info("Desconectando do servidor...");
    
    // Fechar socket
    if (socket_cliente_ >= 0) {
        shutdown(socket_cliente_, SHUT_RDWR);
        close(socket_cliente_);
        socket_cliente_ = -1;
    }
    
    // Aguardar thread de recebimento
    if (thread_recebimento_.joinable()) {
        thread_recebimento_.join();
    }
    
    TSLog::info("Desconectado do servidor");
}

void ClienteTCP::processar_mensagens_recebidas() {
    while (conectado_.load()) {
        try {
            ChatMessage mensagem = receber_mensagem();
            
            if (mensagem.conteudo_.empty()) {
                // Conexão fechada ou erro
                break;
            }
            
            // Exibir mensagem recebida
            std::cout << "\n[" << mensagem.usuario_ << "]: " << mensagem.conteudo_ << std::endl;
            std::cout << "> " << std::flush;
            
            TSLog::info("Mensagem recebida de " + mensagem.usuario_ + ": " + mensagem.conteudo_);
            
        } catch (const std::exception& e) {
            TSLog::error("Erro ao receber mensagem: " + std::string(e.what()));
            break;
        }
    }
    
    conectado_.store(false);
}

ChatMessage ClienteTCP::receber_mensagem() {
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytes_recebidos = recv(socket_cliente_, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_recebidos <= 0) {
        return ChatMessage(); // Retorna mensagem vazia em caso de erro
    }
    
    std::string dados(buffer, bytes_recebidos);
    return ChatMessage::deserialize(dados);
}

bool ClienteTCP::enviar_mensagem(const ChatMessage& mensagem) {
    if (!conectado_.load()) {
        TSLog::warning("Cliente não está conectado");
        return false;
    }
    
    std::string dados = mensagem.serialize();
    
    ssize_t bytes_enviados = send(socket_cliente_, dados.c_str(), dados.length(), 0);
    
    if (bytes_enviados <= 0) {
        TSLog::error("Erro ao enviar mensagem");
        return false;
    }
    
    return true;
}

bool ClienteTCP::enviar_mensagem_chat(const std::string& mensagem) {
    ChatMessage msg(nome_usuario_, mensagem, 0);
    return enviar_mensagem(msg);
}

bool ClienteTCP::solicitar_lista_usuarios() {
    // Implementação futura para Etapa 3
    TSLog::info("Solicitação de lista de usuários (não implementado nesta etapa)");
    return true;
}

void ClienteTCP::exibir_menu() const {
    std::cout << "\n========== Chat CLI ==========" << std::endl;
    std::cout << "Comandos disponíveis:" << std::endl;
    std::cout << "  /quit ou /sair - Sair do chat" << std::endl;
    std::cout << "  /usuarios - Listar usuários conectados" << std::endl;
    std::cout << "  Qualquer outro texto será enviado como mensagem" << std::endl;
    std::cout << "==============================\n" << std::endl;
}

void ClienteTCP::processar_comando_usuario(const std::string& comando) {
    if (comando == "/quit" || comando == "/sair") {
        std::cout << "Encerrando chat..." << std::endl;
        desconectar();
    } else if (comando == "/usuarios") {
        solicitar_lista_usuarios();
    } else if (comando == "/ajuda" || comando == "/help") {
        exibir_menu();
    } else {
        // É uma mensagem de chat
        if (enviar_mensagem_chat(comando)) {
            TSLog::info("Mensagem enviada: " + comando);
        } else {
            std::cout << "Erro ao enviar mensagem!" << std::endl;
        }
    }
}

void ClienteTCP::executar_interface_cli() {
    exibir_menu();
    
    std::string entrada;
    while (conectado_.load()) {
        std::cout << "> ";
        std::getline(std::cin, entrada);
        
        if (entrada.empty()) {
            continue;
        }
        
        processar_comando_usuario(entrada);
    }
}