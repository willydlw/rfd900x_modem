cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

# project name and language, C++ is defaut
project(p2p900 LANGUAGES CXX)

add_library( rfd900
  SHARED
    rfd900_modem.h
    rfd900_modem.cpp
    message900.h
    message900.cpp
)

add_library( messagesim
  SHARED
    simulation_constants.h
    simulation_constants.cpp
    sim_artifact_message.h
    sim_artifact_message.cpp
)

add_executable(txsimple simple_tx.cpp)
add_executable(rxsimple simple_rx.cpp)

target_link_libraries(txsimple rfd900)
target_link_libraries(rxsimple rfd900)
