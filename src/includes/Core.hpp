#ifndef CORE_HPP
#define CORE_HPP

#include "Opcode.hpp"
#include "Instruction.hpp"
#include "Registers.hpp"
#include "ULA.hpp"
#include "UnidadeControle.hpp"
#include "RAM.hpp"
#include "InstructionDecode.hpp"
#include "Pipeline.hpp"
#include "Disco.hpp"
#include "Pipeline.hpp"
#include "Process.hpp"

using namespace std;

class Core {
private:
    int id;
    bool disponivel;
    Process* processoAtual;
    RAM& ram;
    Disco& disco;

public:
    Core();
    Core(int coreId, RAM& memory, Disco& storage);
    ~Core();
    
    pthread_mutex_t mutex_core;
    
    void executeProcess(Process* processo, vector<Process*>& filaProcessos);
    bool isDisponivel();
    int getId() const;
    Process* getProcessoAtual();
};

#endif 