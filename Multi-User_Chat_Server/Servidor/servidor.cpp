#include "servidor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

// ========== ClienteConectado ==========

ClienteConectado::ClienteConectado(int socket, const std::string& nome, int id)
    : socket_cliente_(socket), nome_usuario_(nome), id_cliente_(id), ativo_(true) {
}

ClienteConectado::~ClienteConectado() {
    desconectar();
}

void ClienteConectado::desconectar() {
    if (ativo_.exchange(false)) {
        if (socket_cliente_ >= 0) {
            close(socket_cliente_);
            socket_cliente_ = -1;
        }
        if (thread_cliente_.joinable()) {
            thread_cliente_.join();
        }
    }
}

void ClienteConectado::definir_thread(std::thread&& t) {
    thread_cliente_ = std::move(t);
}

// ========== ServidorTCP ==========

ServidorTCP::ServidorTCP(int porta)
    : socket_servidor_(-1), porta_(porta), executando_(false), proximo_id_cliente_(1) {
}

ServidorTCP::~ServidorTCP() {
    parar();
}

bool ServidorTCP::configurar_socket() {
    // Criar socket TCP
    socket_servidor_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor_ < 0) {
        TSLog::error("Erro ao criar socket do servidor");
        return false;
    }
    
    // Permitir reutilização de endereço
    int opt = 1;
    if (setsockopt(socket_servidor_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        TSLog::warning("Erro ao configurar SO_REUSEADDR");
    }
    
    // Configurar endereço
    struct sockaddr_in endereco_servidor;
    std::memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_addr.s_addr = INADDR_ANY;
    endereco_servidor.sin_port = htons(porta_);
    
    // Bind
    if (bind(socket_servidor_, (struct sockaddr*)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
        TSLog::error("Erro ao fazer bind na porta " + std::to_string(porta_));
        close(socket_servidor_);
        return false;
    }
    
    // Listen
    if (listen(socket_servidor_, 10) < 0) {
        TSLog::error("Erro ao colocar socket em modo listen");
        close(socket_servidor_);
        return false;
    }
    
    return true;
}

bool ServidorTCP::iniciar() {
    if (executando_.load()) {
        TSLog::warning("Servidor já está em execução");
        return false;
    }
    
    if (!configurar_socket()) {
        return false;
    }
    
    executando_.store(true);
    
    // Iniciar thread de broadcast
    thread_broadcast_ = std::thread(&ServidorTCP::processar_broadcast, this);
    
    // Iniciar thread de aceitação de conexões
    thread_aceitacao_ = std::thread(&ServidorTCP::processar_conexoes, this);
    
    TSLog::info("Servidor iniciado na porta " + std::to_string(porta_));
    
    return true;
}

void ServidorTCP::parar() {
    if (!executando_.exchange(false)) {
        return;
    }
    
    TSLog::info("Parando servidor...");
    
    // Fechar socket principal
    if (socket_servidor_ >= 0) {
        shutdown(socket_servidor_, SHUT_RDWR);
        close(socket_servidor_);
        socket_servidor_ = -1;
    }
    
    // Desconectar todos os clientes
    {
        std::lock_guard<std::mutex> lock(mutex_clientes_);
        for (auto& par : clientes_conectados_) {
            par.second->desconectar();
        }
        clientes_conectados_.clear();
    }
    
    // Aguardar threads
    if (thread_aceitacao_.joinable()) {
        thread_aceitacao_.join();
    }
    
    if (thread_broadcast_.joinable()) {
        thread_broadcast_.join();
    }
    
    TSLog::info("Servidor parado");
}

void ServidorTCP::processar_conexoes() {
    while (executando_.load()) {
        struct sockaddr_in endereco_cliente;
        socklen_t tamanho_endereco = sizeof(endereco_cliente);
        
        int socket_cliente = accept(socket_servidor_, (struct sockaddr*)&endereco_cliente, &tamanho_endereco);
        
        if (socket_cliente < 0) {
            if (executando_.load()) {
                TSLog::error("Erro ao aceitar conexão");
            }
            continue;
        }
        
        // Gerar ID único para o cliente
        int id_cliente = proximo_id_cliente_++;
        
        std::string ip_cliente = inet_ntoa(endereco_cliente.sin_addr);
        TSLog::info("Nova conexão aceita de " + ip_cliente + " (ID: " + std::to_string(id_cliente) + ")");
        
        // Criar objeto cliente
        auto cliente = std::make_shared<ClienteConectado>(socket_cliente, "Usuario_" + std::to_string(id_cliente), id_cliente);
        
        // Adicionar à lista de clientes
        {
            std::lock_guard<std::mutex> lock(mutex_clientes_);
            clientes_conectados_[id_cliente] = cliente;
        }
        
        // Criar thread para processar mensagens deste cliente
        std::thread t(&ServidorTCP::processar_cliente, this, cliente);
        cliente->definir_thread(std::move(t));
    }
}

void ServidorTCP::processar_cliente(std::shared_ptr<ClienteConectado> cliente) {
    TSLog::info("Thread iniciada para cliente ID " + std::to_string(cliente->obter_id()));
    
    while (executando_.load() && cliente->esta_ativo()) {
        try {
            ChatMessage mensagem = receber_mensagem(cliente->obter_socket());
            
            if (mensagem.conteudo_.empty()) {
                // Conexão fechada ou erro
                break;
            }
            
            TSLog::info("Mensagem recebida do cliente " + std::to_string(cliente->obter_id()) + ": " + mensagem.conteudo_);
            
            // Adicionar à fila de broadcast
            fila_broadcast_.push(mensagem);
            
        } catch (const std::exception& e) {
            TSLog::error("Erro ao processar mensagem do cliente " + std::to_string(cliente->obter_id()) + ": " + e.what());
            break;
        }
    }
    
    TSLog::info("Cliente " + std::to_string(cliente->obter_id()) + " desconectado");
    remover_cliente(cliente->obter_id());
}

void ServidorTCP::processar_broadcast() {
    while (executando_.load()) {
        ChatMessage mensagem;
        if (fila_broadcast_.wait_and_pop(mensagem, std::chrono::milliseconds(100))) {
            fazer_broadcast(mensagem, mensagem.id_cliente_);
        }
    }
}

void ServidorTCP::fazer_broadcast(const ChatMessage& mensagem, int id_origem) {
    std::lock_guard<std::mutex> lock(mutex_clientes_);
    
    for (auto& par : clientes_conectados_) {
        // Não enviar de volta para o remetente
        if (par.first == id_origem) {
            continue;
        }
        
        if (par.second->esta_ativo()) {
            enviar_mensagem(par.second->obter_socket(), mensagem);
        }
    }
    
    TSLog::info("Broadcast realizado para " + std::to_string(clientes_conectados_.size() - 1) + " clientes");
}

void ServidorTCP::remover_cliente(int id_cliente) {
    std::lock_guard<std::mutex> lock(mutex_clientes_);
    auto it = clientes_conectados_.find(id_cliente);
    if (it != clientes_conectados_.end()) {
        it->second->desconectar();
        clientes_conectados_.erase(it);
        TSLog::info("Cliente " + std::to_string(id_cliente) + " removido da lista");
    }
}

ChatMessage ServidorTCP::receber_mensagem(int socket_cliente) {
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytes_recebidos = recv(socket_cliente, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_recebidos <= 0) {
        return ChatMessage(); // Retorna mensagem vazia em caso de erro
    }
    
    std::string dados(buffer, bytes_recebidos);
    return ChatMessage::deserialize(dados);
}

bool ServidorTCP::enviar_mensagem(int socket_cliente, const ChatMessage& mensagem) {
    std::string dados = mensagem.serialize();
    
    ssize_t bytes_enviados = send(socket_cliente, dados.c_str(), dados.length(), 0);
    
    return bytes_enviados > 0;
}

size_t ServidorTCP::obter_num_clientes_conectados() const {
    std::lock_guard<std::mutex> lock(mutex_clientes_);
    return clientes_conectados_.size();
}

std::vector<std::string> ServidorTCP::listar_clientes_conectados() const {
    std::lock_guard<std::mutex> lock(mutex_clientes_);
    std::vector<std::string> lista;
    
    for (const auto& par : clientes_conectados_) {
        lista.push_back(par.second->obter_nome() + " (ID: " + std::to_string(par.second->obter_id()) + ")");
    }
    
    return lista;
}
