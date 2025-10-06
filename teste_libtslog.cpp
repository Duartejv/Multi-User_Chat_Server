#include "libtslog.hpp"
#include <thread>
#include <vector>

void thread_teste(std::shared_ptr<ThreadSafeLogger> logger, int id) {
    for (int i = 0; i < 10; ++i) {
        logger->info("Thread " + std::to_string(id) + " - Mensagem " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    auto logger = std::make_shared<ThreadSafeLogger>("teste.log", LogLevel::DEBUG, true);

    logger->info("Iniciando teste de logging concorrente");

    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(thread_teste, logger, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    logger->info("Teste concluído com sucesso");

    return 0;
}
