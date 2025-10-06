#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> fila;
    mutable std::mutex mutex_fila;
    std::condition_variable cv;
    bool encerrado;

public:
    ThreadSafeQueue() : encerrado(false) {}

    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_fila);
        if (encerrado) return;
        fila.push(std::move(item));
        cv.notify_one();
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_fila);
        cv.wait(lock, [this] { return !fila.empty() || encerrado; });

        if (fila.empty()) {
            return std::nullopt;
        }

        T item = std::move(fila.front());
        fila.pop();
        return item;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_fila);
        return fila.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_fila);
        return fila.size();
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_fila);
        encerrado = true;
        cv.notify_all();
    }
};

#endif // THREAD_SAFE_QUEUE_HPP
