#include "cliente.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string endereco_servidor = "127.0.0.1";
    int porta = 8080;
    std::string nome_usuario;
    
    // Parse argumentos de linha de comando
    if (argc > 1) {
        endereco_servidor = argv[1];
    }
    if (argc > 2) {
        porta = std::atoi(argv[2]);
    }
    
    std::cout << "========== Cliente de Chat ==========" << std::endl;
    std::cout << "Servidor: " << endereco_servidor << ":" << porta << std::endl;
    
    // Solicitar nome do usuário
    std::cout << "Digite seu nome de usuário: ";
    std::getline(std::cin, nome_usuario);
    
    if (nome_usuario.empty()) {
        nome_usuario = "Usuario_Anonimo";
    }
    
    // Criar cliente
    ClienteTCP cliente(endereco_servidor, porta);
    
    // Conectar ao servidor
    if (!cliente.conectar(nome_usuario)) {
        std::cerr << "Erro ao conectar ao servidor!" << std::endl;
        return 1;
    }
    
    std::cout << "Conectado com sucesso!" << std::endl;
    
    // Executar interface CLI
    cliente.executar_interface_cli();
    
    return 0;
}