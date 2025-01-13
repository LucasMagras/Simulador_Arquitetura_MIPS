#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <vector>
#include <string>
#include <pthread.h>

using namespace std;

class Process;
class Core;
class RAM;
class Disco;

class ProcessManager {
private:
    vector<Process*> filaProcessos;
    vector<Process*> filaProcessosSJF;
    vector<Process*> filaProcessosAltaPrioridade;
    vector<Process*> filaProcessosMediaPrioridade;
    vector<Process*> filaProcessosBaixaPrioridade;
    vector<Core> cores;
    RAM& ramRef;
    Disco& discoRef;
    pthread_mutex_t mutex_fila;
    pthread_cond_t condicao_fila;

public:
    ProcessManager(int numeroCores, RAM& ram, Disco& disco);
    ~ProcessManager();

    void carregarProcessosFCFS(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros);
    void carregarProcessosPrioridade(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros);
    void carregarProcessoSJF(const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros);

    void escalonarProcessosFCFS();
    void escalonarProcessosPrioridade();
    void escalonarSJF();
    int contarLinhas(const string& filename);
};

#endif