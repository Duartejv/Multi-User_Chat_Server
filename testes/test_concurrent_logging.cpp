#include "../codigo/libtslog/tslog.h"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

void funcao_worker_log(int id_worker, int num_mensagens) {
    for (int i = 0; i < num_mensagens; ++i) {
        std::string mensagem = "Worker " + std::to_string(id_worker) + 
                              " - Mensagem " + std::to_string(i);
        
        // Testa diferentes níveis de log
        switch (i % 4) {
            case 0:
                LOG_INFO(mensagem);
                break;
            case 1:
                LOG_DEBUG(mensagem);
                break;
            case 2:
                LOG_WARNING(mensagem);
                break;
            case 3:
                LOG_ERROR(mensagem);
                break;
        }
        
        // Pequeno delay para simular trabalho real
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    try {
        std::cout << "=== TESTE DE LOGGING CONCORRENTE ===" << std::endl;
        
        // Inicializa logger
        auto& logger = TSLog::LoggerManager::obter_instancia("teste_concorrente.log", 
                                                            TSLog::LogLevel::DEBUG);
        
        LOG_INFO("Iniciando teste de logging concorrente");
        
        const int NUM_THREADS = 5;
        const int MENSAGENS_POR_THREAD = 20;
        
        std::vector<std::thread> threads;
        
        // Cria múltiplas threads que fazem logging simultaneamente
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back(funcao_worker_log, i, MENSAGENS_POR_THREAD);
        }
        
        // Aguarda todas as threads terminarem
        for (auto& t : threads) {
            t.join();
        }
        
        LOG_INFO("Teste de logging concorrente finalizado com sucesso");
        std::cout << "Teste concluído! Verifique o arquivo teste_concorrente.log" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro durante o teste: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}