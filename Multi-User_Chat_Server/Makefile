CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -pthread

all: servidor cliente teste_libtslog

servidor: servidor_main.cpp servidor_chat.hpp libtslog.hpp thread_safe_queue.hpp
	$(CXX) $(CXXFLAGS) -o servidor servidor_main.cpp $(LDFLAGS)

cliente: cliente_chat.cpp
	$(CXX) $(CXXFLAGS) -o cliente cliente_chat.cpp $(LDFLAGS)

teste_libtslog: teste_libtslog.cpp libtslog.hpp
	$(CXX) $(CXXFLAGS) -o teste_libtslog teste_libtslog.cpp $(LDFLAGS)

clean:
	rm -f servidor cliente teste_libtslog *.log

.PHONY: all clean
