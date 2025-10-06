#include "servidor.h"
#include <iostream>
#include <csignal>
#include <memory>

// Ponteiro global para o servidor (necessário para signal handler)
std::unique_ptr<ServidorTCP> servidor_global;

void signal_handler(int signum) {
    std::cout << "\nSinal recebido (" << signum << "), encerrando servidor..." << std::endl;
    if (servidor_global) {
        servidor_global->parar();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int porta = 8080;
    
    // Parse argumentos de linha de comando
    if (argc > 1) {
        porta = std::atoi(argv[1]);
    }
    
    // Configurar tratamento de sinais
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "========== Servidor de Chat Multiusuário ==========" << std::endl;
    std::cout << "Iniciando servidor na porta " << porta << "..." << std::endl;
    
    // Criar e iniciar servidor
    servidor_global = std::make_unique<ServidorTCP>(porta);
    
    if (!servidor_global->iniciar()) {
        std::cerr << "Erro ao iniciar servidor!" << std::endl;
        return 1;
    }
    
    std::cout << "Servidor iniciado com sucesso!" << std::endl;
    std::cout << "Pressione Ctrl+C para encerrar." << std::endl;
    
    // Manter o programa em execução
    while (servidor_global->esta_executando()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Exibir estatísticas
        size_t num_clientes = servidor_global->obter_num_clientes_conectados();
        std::cout << "\rClientes conectados: " << num_clientes << "   " << std::flush;
    }
    
    return 0;
}