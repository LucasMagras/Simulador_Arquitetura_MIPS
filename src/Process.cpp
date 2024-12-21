#include "includes/Process.hpp"
#include "includes/Instruction.hpp"
#include "includes/Registers.hpp"
#include "includes/RAM.hpp"
#include "includes/Disco.hpp"
#include "includes/Pipeline.hpp"
#include "includes/Opcode.hpp"
#include "includes/UnidadeControle.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <pthread.h>

using namespace std;

Process::Process(int id, const string& file) : filename(file) {
    pcb.pid = id;
    pcb.estado = PRONTO;
    pcb.quantum = 8;  
    pcb.instrucaoAtual = 0;

    pthread_mutex_init(&mutex_processo, NULL);
    pthread_cond_init(&cond_processo, NULL);

    carregarInstrucoes();
}

void Process::carregarInstrucoes() {
    ifstream arquivo(filename);
    if (!arquivo.is_open()) {
        throw runtime_error("Não foi possível abrir o arquivo de instruções: " + filename);
    }

    instrucoes.clear();
    string linha;

     //pula para a linha correta se o processo não for novo
    if (pcb.instrucaoAtual > 0) {
        cout<<"pulando linha"<< endl;
        for (int i = 0; i < pcb.instrucaoAtual; ++i) {
            getline(arquivo, linha); //pula as linhas já executadas
        }
    }

    while (getline(arquivo, linha)) {
        istringstream ss(linha);
        string opcodeStr;
        int reg1, reg2, reg3;
        char virgula;

        getline(ss, opcodeStr, ',');
        ss >> reg1 >> virgula >> reg2 >> virgula >> reg3;

        Opcode opcode;
        if (opcodeStr == "ADD") opcode = ADD;
        else if (opcodeStr == "SUB") opcode = SUB;
        else if (opcodeStr == "AND") opcode = AND;
        else if (opcodeStr == "OR") opcode = OR;
        else if (opcodeStr == "STORE") opcode = STORE;
        else if (opcodeStr == "LOAD") opcode = LOAD;
        else if (opcodeStr == "ENQ") opcode = ENQ;
        else if (opcodeStr == "IF_igual") opcode = IF_igual;
        else {
            cerr << "Instrução inválida ignorada: " << opcodeStr << endl;
            continue;
        }

        Instruction instrucao(opcode, reg1, reg2, reg3);
        instrucoes.push_back(instrucao);
    }

    arquivo.close();
}

void Process::executar(RAM& ram, Disco& disco) {
    //bloqueia o mutex para evitar acesso concorrente
    pthread_mutex_lock(&mutex_processo);
    pcb.estado = EXECUTANDO;

    Pipeline pipeline;
    int PC = pcb.instrucaoAtual * 4;
    int Clock = 0;

    try {
        for (size_t i = 0; i < instrucoes.size(); ++i) {
            ram.writeInstruction(i, instrucoes[i]);
        }

        //executa enquanto houver instrucoes
        while (PC < instrucoes.size() * 4) {

            //verifica se o quantum foi excedido
            if (Clock >= pcb.quantum) {
                cout << endl << "--- PROCESSO BLOQUEADO, QUANTUM EXCEDIDO ---"<< endl <<endl;
                pcb.estado = BLOQUEADO;
                pcb.instrucaoAtual = PC / 4;
                salvarEstado();
                break;
            }

            Instruction instr = pipeline.InstructionFetch(ram, PC / 4);
            Clock++;

            DecodedInstruction decodedInstr = InstructionDecode(instr, pcb.regs);
            Clock++;

            cout << endl << "[Processo " << pcb.pid << "] Executando instrução:" 
                      << " PC=" << PC 
                      << " Opcode=" << decodedInstr.opcode 
                      << " Destino=R" << decodedInstr.destiny 
                      << " Valor1=" << decodedInstr.value1 
                      << " Valor2=" << decodedInstr.value2 << endl;

            pipeline.Execute(decodedInstr, pcb.regs, ram, PC, disco, Clock);
            PC += 4;

            cout << "PC = " << PC << endl;
            cout << "Clock = " << Clock << endl;

        }
        
        if (PC >= instrucoes.size() * 4) {
            pcb.estado = FINALIZADO;
            cout <<  endl << "--- Processo " << pcb.pid << " concluído ---" << endl << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Erro na execução do processo " << pcb.pid << ": " << e.what() << endl;
        pcb.estado = BLOQUEADO;
    }

    pthread_mutex_unlock(&mutex_processo);
}

void Process::carregarRegistros(const string& arquivoRegistros){
    Pipeline pipeline;
    pipeline.setRegistersFromFile(pcb.regs, arquivoRegistros);
}

void Process::bloquear() {
    pthread_mutex_lock(&mutex_processo);
    pcb.estado = BLOQUEADO;
    pthread_mutex_unlock(&mutex_processo);
}

void Process::desbloquear() {
    pthread_mutex_lock(&mutex_processo);
    pcb.estado = PRONTO;
    pthread_mutex_unlock(&mutex_processo);
}

void Process::salvarEstado() {
    
    estadoSalvo.instrucaoAtual = pcb.instrucaoAtual;
    estadoSalvo.registradores = pcb.regs;
}

void Process::restaurarEstado() {
    pthread_mutex_lock(&mutex_processo);
    
    pcb.instrucaoAtual = estadoSalvo.instrucaoAtual;
    pcb.regs = estadoSalvo.registradores;
    
    pthread_mutex_unlock(&mutex_processo);
}

