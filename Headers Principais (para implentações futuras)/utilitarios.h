#ifndef UTILITARIOS_H
#define UTILITARIOS_H

#include <string>
#include <vector>
#include <chrono>

namespace Utilitarios {
    
    // Formatação de tempo
    std::string formatar_timestamp(const std::chrono::system_clock::time_point& tempo);
    std::chrono::system_clock::time_point obter_tempo_atual();
    
    // Manipulação de strings
    std::string trim(const std::string& str);
    std::vector<std::string> dividir_string(const std::string& str, char delimitador);
    bool string_vazia_ou_espacos(const std::string& str);
    
    // Validação
    bool validar_nome_usuario(const std::string& nome);
    bool validar_porta(int porta);
    bool validar_endereco_ip(const std::string& endereco);
    
    // Utilitários de socket
    std::string obter_endereco_ip_local();
    bool socket_tem_dados_disponiveis(int socket, int timeout_ms = 1000);
    
    // Geração de IDs
    int gerar_id_unico();
    
    // Tratamento de erros
    void tratar_erro_socket(const std::string& operacao);
    std::string obter_mensagem_erro_sistema();
    
    // Configuração
    struct ConfiguracaoServidor {
        int porta = 8080;
        int max_clientes = 100;
        int timeout_conexao_ms = 30000;
        std::string arquivo_log = "servidor.log";
    };
    
    struct ConfiguracaoCliente {
        std::string endereco_servidor = "127.0.0.1";
        int porta_servidor = 8080;
        int timeout_conexao_ms = 5000;
        std::string arquivo_log = "cliente.log";
    };
    
    // Carregamento de configuração
    ConfiguracaoServidor carregar_config_servidor(const std::string& arquivo_config = "");
    ConfiguracaoCliente carregar_config_cliente(const std::string& arquivo_config = "");
}

#endif // UTILITARIOS_H