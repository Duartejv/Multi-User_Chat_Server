#include "message.h"
#include <sstream>
#include <iomanip>

// Construtor com parâmetros
ChatMessage::ChatMessage(const std::string& usuario, const std::string& conteudo, int id_cliente)
    : usuario_(usuario), conteudo_(conteudo), id_cliente_(id_cliente) {
    timestamp_ = std::chrono::system_clock::now();
}

// Serializa a mensagem para transmissão via socket
std::string ChatMessage::serialize() const {
    std::ostringstream oss;
    
    // Formato: ID|USUARIO|CONTEUDO|TIMESTAMP
    auto tempo_epoch = std::chrono::system_clock::to_time_t(timestamp_);
    
    oss << id_cliente_ << "|"
        << usuario_ << "|"
        << conteudo_ << "|"
        << tempo_epoch;
    
    return oss.str();
}

// Desserializa string recebida via socket
ChatMessage ChatMessage::deserialize(const std::string& dados) {
    ChatMessage msg;
    std::istringstream iss(dados);
    std::string temp;
    
    // Parse ID
    if (std::getline(iss, temp, '|')) {
        msg.id_cliente_ = std::stoi(temp);
    }
    
    // Parse usuario
    std::getline(iss, msg.usuario_, '|');
    
    // Parse conteúdo
    std::getline(iss, msg.conteudo_, '|');
    
    // Parse timestamp
    if (std::getline(iss, temp, '|')) {
        std::time_t tempo_epoch = std::stoll(temp);
        msg.timestamp_ = std::chrono::system_clock::from_time_t(tempo_epoch);
    }
    
    return msg;
}