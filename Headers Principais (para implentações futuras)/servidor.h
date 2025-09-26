#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>
#include <map>

#include "../comum/message.h"
#include "../comum/thread_safe_queue.h"
#include "../libtslog/tslog.h"

class ClienteConectado {
private:
    int socket_cliente_;
    std::string nome_usuario_;
    int id_cliente_;
    std::thread thread_cliente_;
    std::atomic<bool> ativo_;
    
public:
    ClienteConectado(int socket, const std::string& nome, int id);
    ~ClienteConectado();
    
    // Getters
    int obter_socket() const { return socket_cliente_; }
    const std::string& obter_nome() const { return nome_usuario_; }
    int obter_id() const { return id_cliente_; }
    bool esta_ativo() const { return ativo_.load(); }
    
    // Controle de conexão
    void desconectar();
    void definir_thread(std::thread&& t);
};

class ServidorTCP {
private:
    int socket_servidor_;                       // Socket principal do servidor
    int porta_;                                // Porta de escuta
    std::atomic<bool> executando_;             // Flag de controle do servidor
    
    // Gerenciamento de clientes
    std::map<int, std::shared_ptr<ClienteConectado>> clientes_conectados_;
    std::mutex mutex_clientes_;                // Proteção da lista de clientes
    int proximo_id_cliente_;
    
    // Thread principal de aceitação
    std::thread thread_aceitacao_;
    
    // Fila de mensagens para broadcast
    ThreadSafeQueue<ChatMessage> fila_broadcast_;
    std::thread thread_broadcast_;
    
    // Métodos privados
    void processar_conexoes();                 // Thread principal de aceitação
    void processar_cliente(std::shared_ptr<ClienteConectado> cliente);
    void processar_broadcast();                // Thread de broadcast de mensagens
    void fazer_broadcast(const ChatMessage& mensagem, int id_origem = -1);
    void remover_cliente(int id_cliente);
    
    // Utilitários de socket
    bool configurar_socket();
    ChatMessage receber_mensagem(int socket_cliente);
    bool enviar_mensagem(int socket_cliente, const ChatMessage& mensagem);
    
public:
    ServidorTCP(int porta = 8080);
    ~ServidorTCP();
    
    // Controle do servidor
    bool iniciar();
    void parar();
    bool esta_executando() const { return executando_.load(); }
    
    // Informações do servidor
    int obter_porta() const { return porta_; }
    size_t obter_num_clientes_conectados() const;
    std::vector<std::string> listar_clientes_conectados() const;
};

#endif // SERVIDOR_H
