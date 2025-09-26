#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <chrono>

struct ChatMessage {
    std::string usuario_;           // Nome do usuário
    std::string conteudo_;         // Conteúdo da mensagem
    std::chrono::system_clock::time_point timestamp_;
    int id_cliente_;               // ID único do cliente
    
    // Serialização para transmissão via socket
    std::string serialize() const;
    static ChatMessage deserialize(const std::string& dados);
    
    ChatMessage() = default;
    ChatMessage(const std::string& usuario, const std::string& conteudo, int id_cliente);
};

enum class TipoMensagem {
    CHAT = 1,
    CONECTAR = 2,
    DESCONECTAR = 3,
    LISTAR_USUARIOS = 4,
    ERRO = 5
};

#endif // MESSAGE_H