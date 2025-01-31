#ifndef UNIDADECONTROLE_HPP
#define UNIDADECONTROLE_HPP

#include <iostream>
#include "Instruction.hpp"
#include "Registers.hpp"
#include "ULA.hpp"
#include "RAM.hpp"
#include "InstructionDecode.hpp"
#include "Disco.hpp"
#include "Pipeline.hpp"
#include "ProcessManager.hpp"
#include "Process.hpp"
#include "Core.hpp"
#include "Cache.hpp"

using namespace std;

class UnidadeControle {
private:
    ProcessManager processManager;

public:
    UnidadeControle(int numeroCores, RAM& ram, Disco& disco, Cache& cache, const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros);
    void carregarArquivo(ProcessManager& processManager, const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros);
};

#endif
