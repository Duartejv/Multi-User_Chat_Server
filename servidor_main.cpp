#include "servidor_chat.hpp"
#include <csignal>
#include <memory>
#include <cstdlib>
#include <iostream>

std::shared_ptr<ServidorChat> servidor_global;

void handler_sinal(int sinal) {
    std::cout << "\nSinal recebido. Encerrando servidor..." << std::endl;
    if (servidor_global) {
        servidor_global->parar();
    }
    std::exit(0);
}

int main(int argc, char* argv[]) {
    int porta = 8080;

    if (argc > 1) {
        porta = std::atoi(argv[1]);
    }

    std::cout << "Iniciando Servidor de Chat na porta " << porta << std::endl;

    signal(SIGINT, handler_sinal);
    signal(SIGTERM, handler_sinal);

    try {
        servidor_global = std::make_shared<ServidorChat>(porta);
        servidor_global->iniciar();
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
