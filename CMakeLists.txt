cmake_minimum_required(VERSION 3.10)
project(ServidorChatConcorrente)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

add_executable(servidor
    servidor_main.cpp
)

target_link_libraries(servidor pthread)

add_executable(cliente
    cliente_chat.cpp
)

target_link_libraries(cliente pthread)

add_executable(teste_libtslog
    teste_libtslog.cpp
)

target_link_libraries(teste_libtslog pthread)
