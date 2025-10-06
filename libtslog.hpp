#ifndef LIBTSLOG_HPP
#define LIBTSLOG_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class ThreadSafeLogger {
private:
    std::ofstream arquivo_log;
    std::mutex mutex_log;
    LogLevel nivel_minimo;
    bool console_habilitado;

    std::string obter_timestamp() {
        auto agora = std::chrono::system_clock::now();
        auto tempo = std::chrono::system_clock::to_time_t(agora);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&tempo), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string nivel_para_string(LogLevel nivel) {
        switch (nivel) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

public:
    ThreadSafeLogger(const std::string& nome_arquivo,
                     LogLevel nivel = LogLevel::INFO,
                     bool console = true)
        : nivel_minimo(nivel), console_habilitado(console) {
        arquivo_log.open(nome_arquivo, std::ios::app);
        if (!arquivo_log.is_open()) {
            throw std::runtime_error("Erro ao abrir arquivo de log");
        }
    }

    ~ThreadSafeLogger() {
        if (arquivo_log.is_open()) {
            arquivo_log.close();
        }
    }

    void log(LogLevel nivel, const std::string& mensagem) {
        if (nivel < nivel_minimo) return;

        std::lock_guard<std::mutex> lock(mutex_log);

        std::stringstream entrada_log;
        entrada_log << "[" << obter_timestamp() << "] "
                    << "[" << nivel_para_string(nivel) << "] "
                    << mensagem << std::endl;

        std::string log_completo = entrada_log.str();

        if (arquivo_log.is_open()) {
            arquivo_log << log_completo;
            arquivo_log.flush();
        }

        if (console_habilitado) {
            std::cout << log_completo;
        }
    }

    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg) { log(LogLevel::INFO, msg); }
    void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }
    void critical(const std::string& msg) { log(LogLevel::CRITICAL, msg); }
};

#endif // LIBTSLOG_HPP
