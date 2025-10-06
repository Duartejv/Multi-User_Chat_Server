#include "tslog.h"
#include <iomanip>
#include <sstream>

using namespace TSLog;

// Implementação ThreadSafeLogger
ThreadSafeLogger::ThreadSafeLogger(const std::string& nome_arquivo, 
                                   LogLevel nivel_min, 
                                   bool timestamp, 
                                   bool thread_id)
    : nivel_minimo_(nivel_min), incluir_timestamp_(timestamp), incluir_thread_id_(thread_id) {
    
    arquivo_log_.open(nome_arquivo, std::ios::app);
    if (!arquivo_log_.is_open()) {
        throw std::runtime_error("Erro: não foi possível abrir arquivo de log: " + nome_arquivo);
    }
    
    log(LogLevel::INFO, "Logger inicializado com sucesso");
}

ThreadSafeLogger::~ThreadSafeLogger() {
    if (arquivo_log_.is_open()) {
        log(LogLevel::INFO, "Logger sendo finalizado");
        arquivo_log_.close();
    }
}

void ThreadSafeLogger::log(LogLevel nivel, const std::string& mensagem) {
    // Verifica se o nível da mensagem é suficiente para ser logado
    if (nivel < nivel_minimo_) {
        return;
    }
    
    // Exclusão mútua para garantir thread-safety
    std::lock_guard<std::mutex> lock(mutex_escrita_);
    
    if (!arquivo_log_.is_open()) {
        return;
    }
    
    std::stringstream linha_log;
    
    // Adiciona timestamp se habilitado
    if (incluir_timestamp_) {
        linha_log << "[" << obter_timestamp_atual() << "] ";
    }
    
    // Adiciona ID da thread se habilitado
    if (incluir_thread_id_) {
        linha_log << "[Thread:" << obter_thread_id() << "] ";
    }
    
    // Adiciona nível do log
    linha_log << "[" << nivel_para_string(nivel) << "] ";
    
    // Adiciona mensagem
    linha_log << mensagem << std::endl;
    
    // Escreve no arquivo
    arquivo_log_ << linha_log.str();
    arquivo_log_.flush(); // Garante escrita imediata
}

std::string ThreadSafeLogger::obter_timestamp_atual() {
    auto agora = std::chrono::system_clock::now();
    auto tempo_t = std::chrono::system_clock::to_time_t(agora);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        agora.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tempo_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string ThreadSafeLogger::nivel_para_string(LogLevel nivel) {
    switch (nivel) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string ThreadSafeLogger::obter_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

// Métodos de conveniência
void ThreadSafeLogger::debug(const std::string& mensagem) {
    log(LogLevel::DEBUG, mensagem);
}

void ThreadSafeLogger::info(const std::string& mensagem) {
    log(LogLevel::INFO, mensagem);
}

void ThreadSafeLogger::warning(const std::string& mensagem) {
    log(LogLevel::WARNING, mensagem);
}

void ThreadSafeLogger::error(const std::string& mensagem) {
    log(LogLevel::ERROR, mensagem);
}

void ThreadSafeLogger::fatal(const std::string& mensagem) {
    log(LogLevel::FATAL, mensagem);
}

// Configuração dinâmica
void ThreadSafeLogger::definir_nivel_minimo(LogLevel nivel) {
    std::lock_guard<std::mutex> lock(mutex_escrita_);
    nivel_minimo_ = nivel;
}

void ThreadSafeLogger::ativar_timestamp(bool ativar) {
    std::lock_guard<std::mutex> lock(mutex_escrita_);
    incluir_timestamp_ = ativar;
}

void ThreadSafeLogger::ativar_thread_id(bool ativar) {
    std::lock_guard<std::mutex> lock(mutex_escrita_);
    incluir_thread_id_ = ativar;
}

void ThreadSafeLogger::flush() {
    std::lock_guard<std::mutex> lock(mutex_escrita_);
    if (arquivo_log_.is_open()) {
        arquivo_log_.flush();
    }
}

bool ThreadSafeLogger::esta_funcional() const {
    return arquivo_log_.is_open();
}

// Implementação LoggerManager (Singleton)
std::unique_ptr<ThreadSafeLogger> LoggerManager::instancia_ = nullptr;
std::mutex LoggerManager::mutex_instancia_;

ThreadSafeLogger& LoggerManager::obter_instancia(const std::string& nome_arquivo, 
                                                  LogLevel nivel_min) {
    std::lock_guard<std::mutex> lock(mutex_instancia_);
    
    if (!instancia_) {
        instancia_ = std::make_unique<ThreadSafeLogger>(nome_arquivo, nivel_min);
    }
    
    return *instancia_;
}

void LoggerManager::finalizar() {
    std::lock_guard<std::mutex> lock(mutex_instancia_);
    instancia_.reset();
}
