#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>
#include <thread>
#include <atomic>
#include <iostream>

#include "../comum/message.h"
#include "../libtslog/tslog.h"

class ClienteTCP {
private:
    int socket_cliente_;                       // Socket de conexão
    std::string endereco_servidor_;            // IP do servidor
    int porta_servidor_;                       // Porta do servidor
    std::string nome_usuario_;                 // Nome do usuário
    
    std::atomic<bool> conectado_;              // Flag de status de conexão
    std::thread thread_recebimento_;           // Thread para receber mensagens
    
    // Métodos privados
    bool conectar_ao_servidor();
    void processar_mensagens_recebidas();     // Thread de recebimento
    ChatMessage receber_mensagem();
    bool enviar_mensagem(const ChatMessage& mensagem);
    
    // Interface CLI
    void exibir_menu() const;
    void processar_comando_usuario(const std::string& comando);
    
public:
    ClienteTCP(const std::string& endereco = "127.0.0.1", int porta = 8080);
    ~ClienteTCP();
    
    // Controle de conexão
    bool conectar(const std::string& nome_usuario);
    void desconectar();
    bool esta_conectado() const { return conectado_.load(); }
    
    // Interface principal
    void executar_interface_cli();             // Loop principal da CLI
    
    // Envio de mensagens
    bool enviar_mensagem_chat(const std::string& mensagem);
    bool solicitar_lista_usuarios();
    
    // Getters
    const std::string& obter_nome_usuario() const { return nome_usuario_; }
    const std::string& obter_endereco_servidor() const { return endereco_servidor_; }
    int obter_porta_servidor() const { return porta_servidor_; }
};

#endif // CLIENTE_H