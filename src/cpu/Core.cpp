#include "../includes/Core.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;
std::mutex coutMutex;

Core::Core() : 
    id(0), 
    ram(*(new RAM())),  
    disco(*(new Disco())),
    disponivel(true), 
    processoAtual(nullptr) {
    pthread_mutex_init(&mutex_core, NULL);
}

Core::Core(int coreId, RAM& memory, Disco& storage) : 
    id(coreId), 
    ram(memory), 
    disco(storage), 
    disponivel(true),
    processoAtual(nullptr) {
    pthread_mutex_init(&mutex_core, NULL);
}

Core::~Core() {
    pthread_mutex_destroy(&mutex_core);
}

void Core::executeProcess(Process* processo, vector<Process*>& filaProcessos) {
    std::lock_guard<std::mutex> lock(coutMutex);
    cout << endl << "--- Iniciando execução do processo " << processo->pcb.pid << " no Core " << id << " ---" << endl;
    pthread_mutex_lock(&mutex_core);
    
    try {
        if (processo == nullptr) {
            cerr << "Erro: Tentativa de executar processo nulo no Core " << id << endl;
            pthread_mutex_unlock(&mutex_core);
            return;
        }

        disponivel = false;
        processoAtual = processo;

        processo->executar(ram, disco);

        switch (processo->pcb.estado) {
            case Process::FINALIZADO:
                cout << "Processo " << processo->pcb.pid << " finalizado no Core " << id << endl;
                break;
            case Process::PRONTO:
                cout << "Processo " << processo->pcb.pid  << " preemptado no Core " << id << endl;
                break;
            case Process::BLOQUEADO:
                cout << "Processo " << processo->pcb.pid << " bloqueado no Core " << id << endl;
                break;
            case Process::EXECUTANDO:
                cout << "Processo " << processo->pcb.pid << " ainda em execução no Core " << id << endl;
                break;
        }

        if (processo->pcb.estado == Process::BLOQUEADO) {
            cout << "Colocando o processo de volta na fila" << endl;
            filaProcessos.push_back(processo); 
        }
    }
    catch (const exception& e) {
        cerr << "Erro na execução do core " << id << ": " << e.what() << endl;
    }

    disponivel = true;
    processoAtual = nullptr;

    pthread_mutex_unlock(&mutex_core);
    cout << "Finalizando execução do processo " << processo->pcb.pid << " no Core " << id << endl;
}

bool Core::isDisponivel() {
    pthread_mutex_lock(&mutex_core);
    bool status = disponivel;
    pthread_mutex_unlock(&mutex_core);
    return status;
}

int Core::getId() const { 
    return id; 
}

Process* Core::getProcessoAtual() { 
    return processoAtual; 
}
