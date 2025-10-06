#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ClienteChat {
private:
    int socket_cliente;
    std::atomic<bool> conectado;
    std::thread thread_recepcao;

    void receber_mensagens() {
        char buffer[4096];

        while (conectado) {
            ssize_t bytes = recv(socket_cliente, buffer, sizeof(buffer) - 1, 0);

            if (bytes <= 0) {
                std::cout << "\nDesconectado do servidor." << std::endl;
                conectado = false;
                break;
            }

            buffer[bytes] = '\0';
            std::cout << buffer;
            std::cout.flush();
        }
    }

public:
    ClienteChat() : socket_cliente(-1), conectado(false) {}

    ~ClienteChat() {
        desconectar();
    }

    bool conectar(const std::string& ip_servidor, int porta,
                  const std::string& nome_usuario) {
        socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_cliente < 0) {
            std::cerr << "Erro ao criar socket" << std::endl;
            return false;
        }

        struct sockaddr_in endereco_servidor;
        std::memset(&endereco_servidor, 0, sizeof(endereco_servidor));
        endereco_servidor.sin_family = AF_INET;
        endereco_servidor.sin_port = htons(porta);

        if (inet_pton(AF_INET, ip_servidor.c_str(),
                      &endereco_servidor.sin_addr) <= 0) {
            std::cerr << "Endereço IP inválido" << std::endl;
            close(socket_cliente);
            return false;
        }

        if (connect(socket_cliente, (struct sockaddr*)&endereco_servidor,
                   sizeof(endereco_servidor)) < 0) {
            std::cerr << "Erro ao conectar ao servidor" << std::endl;
            close(socket_cliente);
            return false;
        }

        send(socket_cliente, nome_usuario.c_str(), nome_usuario.length(), 0);

        conectado = true;
        thread_recepcao = std::thread(&ClienteChat::receber_mensagens, this);

        std::cout << "Conectado ao servidor!" << std::endl;
        return true;
    }

    void enviar_mensagem(const std::string& mensagem) {
        if (!conectado) return;

        std::string msg_completa = mensagem + "\n";
        ssize_t enviado = send(socket_cliente, msg_completa.c_str(),
                              msg_completa.length(), 0);

        if (enviado < 0) {
            std::cerr << "Erro ao enviar mensagem" << std::endl;
            conectado = false;
        }
    }

    void desconectar() {
        if (!conectado) return;

        conectado = false;

        if (socket_cliente >= 0) {
            close(socket_cliente);
        }

        if (thread_recepcao.joinable()) {
            thread_recepcao.join();
        }
    }

    bool esta_conectado() const {
        return conectado;
    }
};

int main(int argc, char* argv[]) {
    std::string ip_servidor = "127.0.0.1";
    int porta = 8080;
    std::string nome_usuario;

    if (argc > 1) ip_servidor = argv[1];
    if (argc > 2) porta = std::atoi(argv[2]);

    std::cout << "=== Cliente de Chat ===" << std::endl;
    std::cout << "Digite seu nome de usuário: ";
    std::getline(std::cin, nome_usuario);

    ClienteChat cliente;

    if (!cliente.conectar(ip_servidor, porta, nome_usuario)) {
        std::cerr << "Falha ao conectar. Encerrando..." << std::endl;
        return 1;
    }

    std::cout << "\nDigite suas mensagens (ou 'sair' para desconectar):" << std::endl;

    std::string mensagem;
    while (cliente.esta_conectado()) {
        std::getline(std::cin, mensagem);

        if (mensagem == "sair" || mensagem == "exit") {
            break;
        }

        if (!mensagem.empty()) {
            cliente.enviar_mensagem(mensagem);
        }
    }

    cliente.desconectar();
    std::cout << "Desconectado." << std::endl;

    return 0;
}
