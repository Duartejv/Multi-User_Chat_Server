#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex_;
    std::queue<T> fila_;
    std::condition_variable condicao_;

public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue& other);
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Adiciona elemento à fila (thread-safe)
    void push(T item);
    
    // Remove e retorna elemento (bloqueia se vazio)
    void wait_and_pop(T& item);
    std::shared_ptr<T> wait_and_pop();
    
    // Tenta remover elemento (não bloqueia)
    bool try_pop(T& item);
    std::shared_ptr<T> try_pop();
    
    // Verifica se está vazia
    bool empty() const;
    
    // Retorna tamanho atual
    size_t size() const;
};

#endif // THREAD_SAFE_QUEUE_H
