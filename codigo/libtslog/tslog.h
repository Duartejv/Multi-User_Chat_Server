#ifndef TSLOG_H
#define TSLOG_H

#include <string>
#include <fstream>
#include <mutex>              
#include <memory>
#include <chrono>
#include <thread>           
#include <condition_variable> 
#include <iomanip>          
#include <sstream>           

namespace TSLog {
    
    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        FATAL = 4
    };
    
    class ThreadSafeLogger {
    private:
        std::ofstream arquivo_log_;               
        std::mutex mutex_escrita_;                
        LogLevel nivel_minimo_;                   
        bool incluir_timestamp_;                  
        bool incluir_thread_id_;                  
        
        std::string obter_timestamp_atual();
        std::string nivel_para_string(LogLevel nivel);
        std::string obter_thread_id();
        
    public:
        ThreadSafeLogger(const std::string& nome_arquivo, 
                        LogLevel nivel_min = LogLevel::INFO,
                        bool timestamp = true, 
                        bool thread_id = true);
        
        ~ThreadSafeLogger();
        
        void debug(const std::string& mensagem);
        void info(const std::string& mensagem);
        void warning(const std::string& mensagem);
        void error(const std::string& mensagem);
        void fatal(const std::string& mensagem);
        
        void log(LogLevel nivel, const std::string& mensagem);
        
        void definir_nivel_minimo(LogLevel nivel);
        void ativar_timestamp(bool ativar);
        void ativar_thread_id(bool ativar);
        
        void flush();
        
        bool esta_funcional() const;
    };
    
    class LoggerManager {
    private:
        static std::unique_ptr<ThreadSafeLogger> instancia_;
        static std::mutex mutex_instancia_;
        
    public:
        static ThreadSafeLogger& obter_instancia(
            const std::string& nome_arquivo = "aplicacao.log",
            LogLevel nivel_min = LogLevel::INFO
        );
        
        static void finalizar();
    };
    
    #define LOG_DEBUG(msg) TSLog::LoggerManager::obter_instancia().debug(msg)
    #define LOG_INFO(msg) TSLog::LoggerManager::obter_instancia().info(msg)
    #define LOG_WARNING(msg) TSLog::LoggerManager::obter_instancia().warning(msg)
    #define LOG_ERROR(msg) TSLog::LoggerManager::obter_instancia().error(msg)
    #define LOG_FATAL(msg) TSLog::LoggerManager::obter_instancia().fatal(msg)
}

#endif // TSLOG_H
