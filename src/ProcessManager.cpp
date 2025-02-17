#include "includes/ProcessManager.hpp"
#include "includes/Process.hpp"
#include "includes/Core.hpp"
#include "includes/RAM.hpp"
#include "includes/Disco.hpp"
#include "includes/Cache.hpp"

#include <thread>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <mutex>

using namespace std;
namespace fs = std::filesystem;

ProcessManager::ProcessManager(int numeroCores, RAM& ram, Disco& disco, Cache& cache) : ramRef(ram), discoRef(disco), cacheRef(cache) {
    //inicializa cores
    for (int i = 0; i < numeroCores; ++i) {
        cores.emplace_back(i, ram, disco, cache);
    }

    //mutex e condição
    pthread_mutex_init(&mutex_fila, NULL);
    pthread_cond_init(&condicao_fila, NULL);
}

void ProcessManager::carregarProcessosFCFS(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
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

void ProcessManager::carregarProcessosPrioridade(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
    pthread_mutex_lock(&mutex_fila);
    
    try {
        for (size_t i = 0; i < arquivosInstrucoes.size(); ++i) {
            if (!fs::exists(arquivosInstrucoes[i])) {
                cerr << "Arquivo de instruções não encontrado: " << arquivosInstrucoes[i] << endl;
                continue;
            }

            string arquivoRegistro = i < arquivosRegistros.size() ? arquivosRegistros[i] : "data/setRegisters.txt";

            Process* novoProcesso = new Process(i, arquivosInstrucoes[i]);
            novoProcesso->carregarRegistros(arquivoRegistro);

            // le a prioridade do processo
            ifstream arquivo(arquivosInstrucoes[i]);
            int prioridade;
            arquivo >> prioridade;

            // adiciona o processo na fila correspondente
            if (prioridade == 1) {
                filaProcessosBaixaPrioridade.push_back(novoProcesso);
            } else if (prioridade == 2) {
                filaProcessosMediaPrioridade.push_back(novoProcesso);
            } else if (prioridade == 3) {
                filaProcessosAltaPrioridade.push_back(novoProcesso);
            }
        }

    }
    catch (const exception& e) {
        cerr << "Erro ao carregar processos: " << e.what() << endl;
    }

    pthread_mutex_unlock(&mutex_fila);
}

void ProcessManager::carregarProcessoSJF(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
    pthread_mutex_lock(&mutex_fila);
    
    try {
        for (size_t i = 0; i < arquivosInstrucoes.size(); ++i) {
            if (!fs::exists(arquivosInstrucoes[i])) {
                cerr << "Arquivo de instruções não encontrado: " << arquivosInstrucoes[i] << endl;
                continue;
            }

            string arquivoRegistro = i < arquivosRegistros.size() ? arquivosRegistros[i] : "data/setRegisters.txt";

            Process* novoProcesso = new Process(i, arquivosInstrucoes[i]);
            novoProcesso->carregarRegistros(arquivoRegistro);

            // le a prioridade do processo
            ifstream arquivo(arquivosInstrucoes[i]);
            int prioridade;
            arquivo >> prioridade;

            // adiciona o processo na fila de SJF
            filaProcessosSJF.push_back(novoProcesso);
        }

        // ordena a fila de SJF pelo número de linhas (menor número de linhas primeiro)
        sort(filaProcessosSJF.begin(), filaProcessosSJF.end(), [this](Process* a, Process* b) {
            return contarLinhas(a->filename) < contarLinhas(b->filename);
        });
    }
    catch (const exception& e) {
        cerr << "Erro ao carregar processos: " << e.what() << endl;
    }

    pthread_mutex_unlock(&mutex_fila);
}

void ProcessManager::carregarProcessosPorSimilaridade(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
    vector<Process*> processosCarregados;

    for (size_t i = 0; i < arquivosInstrucoes.size(); ++i) {
        string arquivoRegistro = i < arquivosRegistros.size() ? arquivosRegistros[i] : "data/setRegisters.txt";
        Process* novoProcesso = new Process(i, arquivosInstrucoes[i]);
        novoProcesso->carregarRegistros(arquivoRegistro);
        processosCarregados.push_back(novoProcesso);
    }

    // ordena os processos com base na similaridade
    sort(processosCarregados.begin(), processosCarregados.end(), [this](Process* p1, Process* p2) {
        return calcularSimilaridade(*p1, *p2) > 0;
    });
    
    for (auto processo : processosCarregados) {
        filaProcessos.push_back(processo);
    }
}

void ProcessManager::carregarProcessoMMU(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) {
    pthread_mutex_lock(&mutex_fila);
    
    try {
        // Limpa estruturas existentes
        filaProcessos.clear();
        posicoesMRU.clear();

        // Carrega os novos processos
        for (size_t i = 0; i < arquivosInstrucoes.size(); ++i) {
            if (!fs::exists(arquivosInstrucoes[i])) {
                cerr << "Arquivo de instruções não encontrado: " << arquivosInstrucoes[i] << endl;
                continue;
            }

            // Verifica arquivo de registros
            string arquivoRegistro = i < arquivosRegistros.size() ? arquivosRegistros[i] : "data/setRegisters.txt";
            if (!fs::exists(arquivoRegistro)) {
                cerr << "Arquivo de registros não encontrado: " << arquivoRegistro << endl;
                continue;
            }

            // Cria novo processo
            Process* novoProcesso = new Process(i, arquivosInstrucoes[i]);
            
            // Carrega registros do processo
            novoProcesso->carregarRegistros(arquivoRegistro);

            // Adiciona processo na fila
            filaProcessos.push_back(novoProcesso);
        }

        // Cria e ordena vetor de posições
        posicoesMRU.resize(filaProcessos.size());
        for (size_t i = 0; i < filaProcessos.size(); i++) {
            posicoesMRU[i] = i;
        }

        // Ordena vetor com base no número de linhas
        sort(posicoesMRU.begin(), posicoesMRU.end(),
            [this](int a, int b) {
                return contarLinhas(filaProcessos[a]->filename) < 
                       contarLinhas(filaProcessos[b]->filename);
            });
    }
    catch (const exception& e) {
        cerr << "Erro ao carregar processos MRU: " << e.what() << endl;
    }

    pthread_mutex_unlock(&mutex_fila);
}

void ProcessManager::escalonarProcessosPrioridade() {
    auto start = std::chrono::high_resolution_clock::now();
    cout << endl << "--- EXISTEM " << filaProcessosAltaPrioridade.size() + filaProcessosMediaPrioridade.size() + filaProcessosBaixaPrioridade.size() << " PROCESSOS PARA SEREM EXECUTADOS ---" << endl;
    cout << endl << "--- INICIANDO A EXECUCAO DOS PROCESSOS UTILIZANDO A POLITICA DE PRIORIDADE ---" << endl;

    std::vector<std::thread> threads; 

    while (!filaProcessosAltaPrioridade.empty() || !filaProcessosMediaPrioridade.empty() || !filaProcessosBaixaPrioridade.empty()) {
        cout << endl << "--- INICIANDO A EXECUCAO DE " << filaProcessosAltaPrioridade.size() <<  " PROCESSOS NA FILA DE ALTA PRIORIDADE ---" << endl;
        while (!filaProcessosAltaPrioridade.empty()) {
            for (auto& core : cores) {
                 if (core.isDisponivel() && !filaProcessosAltaPrioridade.empty()) {
                    Process* processo = filaProcessosAltaPrioridade.front();
                    filaProcessosAltaPrioridade.erase(filaProcessosAltaPrioridade.begin());

                    cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 << " ao Core " << core.getId() + 1 << " ---";
                    core.disponivel = false;

                    // cria uma thread para executar o processo
                    threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessosAltaPrioridade));
                }
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }

        cout << endl << "--- INICIANDO A EXECUCAO DE " << filaProcessosMediaPrioridade.size() <<  " PROCESSOS NA FILA DE MEDIA PRIORIDADE ---" << endl;
        while (!filaProcessosMediaPrioridade.empty()) {
            for(auto& core : cores){
                if (core.isDisponivel() && !filaProcessosMediaPrioridade.empty()) {
                    Process* processo = filaProcessosMediaPrioridade.front();
                    filaProcessosMediaPrioridade.erase(filaProcessosMediaPrioridade.begin());

                    cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 << " ao Core " << core.getId() + 1 << " ---";
                    core.disponivel = false;

                    // cria uma thread para executar o processo
                    threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessosMediaPrioridade));
                }
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }

        cout << endl << "--- INICIANDO A EXECUCAO DE " << filaProcessosBaixaPrioridade.size() <<  " PROCESSOS NA FILA DE BAIXA PRIORIDADE ---" << endl;        
        while (!filaProcessosBaixaPrioridade.empty()) {
            for(auto& core : cores){
                if (core.isDisponivel() && !filaProcessosBaixaPrioridade.empty()) {
                    Process* processo = filaProcessosBaixaPrioridade.front();
                    filaProcessosBaixaPrioridade.erase(filaProcessosBaixaPrioridade.begin());

                    cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 << " ao Core " << core.getId() + 1 << " ---" ;
                    core.disponivel = false;

                    // cria uma thread para executar o processo
                    threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessosBaixaPrioridade));
                }
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
    }
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << endl << "--- FIM DA EXECUCAO, TODOS OS PROCESSOS FORAM CONCLUIDOS ---" << endl;
    cout <<"Tempo total de execução (Prioridade): " << duration.count() << " ms" << endl<< endl;
}

void ProcessManager::escalonarSJF() {
    auto start = std::chrono::high_resolution_clock::now();
    cout << endl << endl << "INICIANDO A EXECUCAO DOS PROCESSOS UTILIZANDO A POLITICA SJF" << endl;
    //cout << endl << "--- INICIANDO A EXECUCAO DE " << filaProcessosSJF.size() << " PROCESSOS ---" << endl;

    std::vector<std::thread> threads; 

    while (!filaProcessosSJF.empty()) {
        //distribui os processos para cores livres
        for (auto& core : cores) {
            if (core.isDisponivel() && !filaProcessosSJF.empty()) {
                Process* processo = filaProcessosSJF.front();
                filaProcessosSJF.erase(filaProcessosSJF.begin());

                cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 << " ao Core " << core.getId() + 1 << " ---";
                core.disponivel = false;

                //cria uma thread para executar o processo
                threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessosSJF));
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << endl << "--- FIM DA EXECUCAO, TODOS OS PROCESSOS FORAM CONCLUIDOS ---" << endl;
    cout << "Tempo total de execução (SJF): " << duration.count() << " ms" << endl<< endl;
}

void ProcessManager::escalonarProcessosFCFS() {
    auto start = std::chrono::high_resolution_clock::now();
    cout << endl << "--- INICIANDO A EXECUCAO DOS PROCESSOS UTILIZANDO A POLITICA FCFS ---" << endl;

    std::vector<std::thread> threads; 

    while (!filaProcessos.empty()) {
        //distribui os processos para cores livres
        for (auto& core : cores) {
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

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << endl << "--- FIM DA EXECUCAO, TODOS OS PROCESSOS FORAM CONCLUIDOS ---" << endl;
    cout << "Tempo total de execução (FCFS): " << duration.count() << " ms" << endl<< endl;
}

void ProcessManager::escalonarMMU() {
    cout << endl << "--- INICIANDO A EXECUCAO DOS PROCESSOS UTILIZANDO A POLITICA MRU ---" << endl;

    std::vector<std::thread> threads;

    // Verifica se há processos para escalonar
    if (filaProcessos.empty() || posicoesMRU.empty()) {
        cout << "Nenhum processo para escalonar." << endl;
        return;
    }

    //Log do vetor MRU e processos correspondentes
    cout << endl << "Vetor MMU e processos correspondentes:" << endl;
    for (size_t i = 0; i < posicoesMRU.size(); i++) {
        int posicaoBinaria = posicoesMRU[i];
        Process* processo = filaProcessos[posicaoBinaria];
        
        cout << "Posição no vetor: " << i 
             << " | Valor binário: " << std::bitset<8>(posicaoBinaria).to_string()
             << " | Processo PID: " << processo->pcb.pid + 1
             << " | Tamanho: " << contarLinhas(processo->filename) << " linhas" << endl;
    }
    cout << endl;

    auto start = std::chrono::high_resolution_clock::now();

    // Distribui os processos para cores livres
    while (!posicoesMRU.empty()) {
        for (auto& core : cores) {
            if (core.isDisponivel() && !posicoesMRU.empty()) {
                // Obtém a posição na fila original
                int posicaoNaFila = posicoesMRU.front();
                posicoesMRU.erase(posicoesMRU.begin()); // Remove a posição do vetor

                // Verifica se a posição é válida
                if (posicaoNaFila < 0 || posicaoNaFila >= filaProcessos.size()) {
                    cerr << "Posição inválida no vetor MMU: " << posicaoNaFila << endl;
                    continue;
                }

                // Obtém o processo correspondente
                Process* processo = filaProcessos[posicaoNaFila];

                cout << endl << "--- Atribuindo o processo " << processo->pcb.pid + 1 
                     << " ao Core " << core.getId() + 1 << " ---" << endl;
                core.disponivel = false;

                // Cria thread para executar o processo
                threads.emplace_back(&Core::executeProcess, &core, processo, std::ref(filaProcessos));
            }
            this_thread::sleep_for(chrono::milliseconds(1)); // Remover se não necessário
        }
    }

    // Aguarda todas as threads terminarem
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << endl << "--- FIM DA EXECUCAO, TODOS OS PROCESSOS FORAM CONCLUIDOS ---" << endl;
    cout << "Tempo total de execução (MRU): " << duration.count() << " ms" << endl << endl;
}

ProcessManager::~ProcessManager() {
    //libera recursos e memoria
    pthread_mutex_destroy(&mutex_fila);
    pthread_cond_destroy(&condicao_fila);
    for (auto processo : filaProcessos) {
        delete processo;
    }
}

int ProcessManager::contarLinhas(const string& filename) {
    ifstream arquivo(filename);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        return 0;
    }
    int linhas = 0;
    string linha;
    while (getline(arquivo, linha)) {
        linhas++;
    }
    arquivo.close();
    return linhas;
}

int ProcessManager::calcularSimilaridade(const Process& p1, const Process& p2) {
    int similaridade = 0;
    // compara as instruções
    for (const auto& instr1 : p1.instrucoes) {
        for (const auto& instr2 : p2.instrucoes) {
            if (instr1.op == instr2.op) { 
                similaridade++;
            }
        }
    }
    return similaridade;
}