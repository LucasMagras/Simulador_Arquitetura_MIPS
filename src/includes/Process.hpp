#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "Instruction.hpp"
#include "Registers.hpp"
#include "Opcode.hpp"
#include "RAM.hpp"
#include "Disco.hpp"
#include "UnidadeControle.hpp"
#include "Cache.hpp"

using namespace std;

class Process {
public:
    enum Estado {
        PRONTO,
        EXECUTANDO,
        BLOQUEADO,
        FINALIZADO
    };

    struct PCB {
        int pid;                      
        Estado estado;               
        int quantum;                  
        int instrucaoAtual;           
        Registers regs;          
        vector<string> arquivosAbertos; 
    };

    Process(int id, const std::string& file);

    void carregarInstrucoes();
    void executar(RAM& ram, Disco& disco, Cache& cache);
    string convertInstructionToString(const Instruction& instr);
    //void bloquear();
    //void desbloquear();
    void salvarEstado();
    //void restaurarEstado();

    void carregarRegistros(const string& arquivoRegistros);

    const vector<Instruction>& getInstrucoes() const {
        return instrucoes;
    }
    
    PCB pcb;
    vector<Instruction> instrucoes;
    string filename;

private:
    //pthread_mutex_t mutex_processo;
    //pthread_cond_t cond_processo;

    struct EstadoSalvo {
        int instrucaoAtual;
        Registers registradores;
    } estadoSalvo;
};

#endif 