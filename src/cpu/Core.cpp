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
    cache(*(new Cache())),
    disponivel(true), 
    processoAtual(nullptr) {
    pthread_mutex_init(&mutex_core, NULL);
}

Core::Core(int coreId, RAM& memory, Disco& storage, Cache& cache) : 
    id(coreId), 
    ram(memory), 
    disco(storage), 
    cache(cache),
    disponivel(true),
    processoAtual(nullptr) {
    pthread_mutex_init(&mutex_core, NULL);
}

Core::~Core() {
    pthread_mutex_destroy(&mutex_core);
}

void Core::executeProcess(Process* processo, vector<Process*>& filaProcessos) {
    //std::lock_guard<std::mutex> lock(coutMutex);
    cout << endl << "--- Iniciando execução do processo " << processo->pcb.pid + 1 << " no Core " << id + 1<< " ---" << endl;
    //pthread_mutex_lock(&mutex_core);
    //std::cout.flush();
    //std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Atraso de 100ms
    
    try {
        if (processo == nullptr) {
            cerr << "Erro: Tentativa de executar processo nulo no Core " << id << endl;
            pthread_mutex_unlock(&mutex_core);
            return;
        }

        //disponivel = false;
        processoAtual = processo;

        processo->executar(ram, disco, cache);

        switch (processo->pcb.estado) {
            case Process::FINALIZADO:
                cout << "Processo " << processo->pcb.pid + 1 << " finalizado no Core " << id + 1<< endl;
                break;
            case Process::BLOQUEADO:
                cout << "Processo " << processo->pcb.pid + 1 << " bloqueado no Core " << id + 1 << endl;
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

    //pthread_mutex_unlock(&mutex_core);
    cout << "Finalizando execução do processo " << processo->pcb.pid + 1 << " no Core " << id + 1 << endl;
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
