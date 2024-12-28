#include "includes/ProcessManager.hpp"
#include "includes/Process.hpp"
#include "includes/Core.hpp"
#include "includes/RAM.hpp"
#include "includes/Disco.hpp"

#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <mutex>

using namespace std;
namespace fs = std::filesystem;

ProcessManager::ProcessManager(int numeroCores, RAM& ram, Disco& disco) : ramRef(ram), discoRef(disco) {
    //inicializa cores
    for (int i = 0; i < numeroCores; ++i) {
        cores.emplace_back(i, ram, disco);
    }

    //mutex e condição
    pthread_mutex_init(&mutex_fila, NULL);
    pthread_cond_init(&condicao_fila, NULL);
}

void ProcessManager::carregarProcessos(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
    pthread_mutex_lock(&mutex_fila);
    
    try {
        for (size_t i = 0; i < arquivosInstrucoes.size(); ++i) {
            if (!fs::exists(arquivosInstrucoes[i])) {
                cerr << "Arquivo de instruções não encontrado: " << arquivosInstrucoes[i] << endl;
                continue;
            }

            string arquivoRegistro = i < arquivosRegistros.size() ? arquivosRegistros[i] : "data/setRegisters.txt";

            Process* novoProcesso = new Process(filaProcessos.size(), arquivosInstrucoes[i]);
            novoProcesso->carregarRegistros(arquivoRegistro);
            
            filaProcessos.push_back(novoProcesso);
        }
    }
    catch (const exception& e) {
        cerr << "Erro ao carregar processos: " << e.what() << endl;
    }

    pthread_mutex_unlock(&mutex_fila);
}

void ProcessManager::escalonarProcessos() {
    cout << endl << "--- INICIANDO A EXECUCAO DE " << filaProcessos.size() << " PROCESSOS ---" << endl;

    std::vector<std::thread> threads; 

    while (!filaProcessos.empty()) {
        //distribui os processos para cores livres
        for (auto& core : cores) {
            //cout << endl << "Verificando o estado do Core " << core.getId() + 1 << ": " 
                 //<< (core.isDisponivel() ? "Disponível" : "Ocupado") << endl;
            if (core.isDisponivel() && !filaProcessos.empty()) {
                Process* processo = filaProcessos.front();
                filaProcessos.erase(filaProcessos.begin());

                cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 << " ao Core " << core.getId() + 1 << " ---" <<endl;
                core.disponivel = false;

                //cria uma thread para executar o processo
                threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessos));
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    cout << endl << "--- FIM DA EXECUCAO, TODOS OS PROCESSOS FORAM CONCLUIDOS ---" << endl;
}

ProcessManager::~ProcessManager() {
    //libera recursos e memoria
    pthread_mutex_destroy(&mutex_fila);
    pthread_cond_destroy(&condicao_fila);
    for (auto processo : filaProcessos) {
        delete processo;
    }
}