#include "../includes/Pipeline.hpp"
#include "../includes/Process.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iostream>

using namespace std;

Pipeline::Pipeline() {}

Instruction Pipeline::InstructionFetch(RAM& ram, int endereco) {
    if (endereco >= 0 && endereco < ram.tamanho) {
        return ram.instruction_memory[endereco];
    }
    cout << "Erro: Endereço inválido para instrução na RAM " << endereco << endl;
    return Instruction(ADD, 0, 0, 0);
}

void Pipeline::Wb(const DecodedInstruction& decoded, int& resultado, RAM& ram, Disco& disco, int& Clock) {
    ram.write(decoded.value1, resultado); 
    Clock++;
}

void Pipeline::MemoryAccess(const DecodedInstruction& decoded, int resultado, Registers& regs, int& Clock) {
    //cout << "Resultado a ser armazenado: " << resultado << endl;

    try {
        regs.set(decoded.destiny, resultado);
        Clock++;
        //cout << "Registrador R" << decoded.destiny << " atualizado para: " << regs.get(decoded.destiny) << endl;
    }
    catch (const exception& e) {
        cerr << "Erro no Memory Access: " << e.what() << endl;
    }
}

void Pipeline::Execute(const DecodedInstruction& decoded, Registers& regs, RAM& ram, int& PC, Disco& disco, int& Clock) {
    switch (decoded.opcode) {
        case ADD: {
            int resultado = ula.exec(decoded.value1, decoded.value2, ADD);
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "ADD R" << decoded.destiny << " = " << decoded.value1 << " + " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case SUB: {
            int resultado = ula.exec(decoded.value1, decoded.value2, SUB);
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "SUB R" << decoded.destiny << " = " << decoded.value1 << " - " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case AND: {
            int resultado = ula.exec(decoded.value1, decoded.value2, AND);
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "AND R" << decoded.destiny << " = " << decoded.value1 << " & " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case OR: {
            int resultado = ula.exec(decoded.value1, decoded.value2, OR);
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "OR R" << decoded.destiny << " = " << decoded.value1 << " | " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case LOAD: {
            int valor = ram.read(decoded.value1);
            Clock++;
            MemoryAccess(decoded, valor, regs, Clock);
            //cout << "LOAD R" << decoded.destiny << " = RAM[" << decoded.value1 << "] -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case STORE: {
            int valor = regs.get(decoded.destiny);
            Clock++;
            Wb(decoded, valor, ram, disco, Clock);
            //cout << "STORE RAM[" << decoded.value1 << "] = R" << decoded.destiny << " -> " << valor << endl;
            disco.write(valor);
            //cout << "STORE DISK[" << valor << "]" << endl;
            break;
        }
        case MULT: {
            int resultado = ula.exec(decoded.value1, decoded.value2, MULT);
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "MULT R" << decoded.destiny << " = " << decoded.value1 << " * " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case DIV: {
            if (decoded.value2 != 0) {
                int resultado = ula.exec(decoded.value1, decoded.value2, DIV);
                Clock++;
                MemoryAccess(decoded, resultado, regs, Clock);
                
                //cout << "DIV R" << decoded.destiny << " = " << decoded.value1 << " / " << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            } 
            else {
                //cerr << "Erro: Divisão por zero!" << endl;
            }
            break;
        }
        case IF_igual: {
            int resultado = (decoded.value1 == decoded.value2) ? 1 : 0;
            Clock++;
            MemoryAccess(decoded, resultado, regs, Clock);
            
            //cout << "IF_igual " << decoded.destiny << " = R" << decoded.value1 << " == R" << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        case ENQ: {
            int resultado = decoded.value1;
            //cout << "OPERADOR 1: " << decoded.value1 << endl;
            //cout << "OPERADOR 2: " << decoded.value2 << endl;
            
            while (resultado != decoded.value2) {
                if (resultado < decoded.value2) {
                    resultado = ula.exec(resultado, 1, ADD);
                    Clock++;
                    MemoryAccess(decoded, resultado, regs, Clock);
                } else {
                    resultado = ula.exec(resultado, 1, SUB);
                    Clock++;
                    MemoryAccess(decoded, resultado, regs, Clock);
                }
            }
            //cout << "ENQ " << decoded.destiny << " = R" << decoded.value1 << " enquanto R" << decoded.value2 << " -> " << regs.get(decoded.destiny) << endl;
            break;
        }
        default:
            cerr << "Opcode desconhecido: " << decoded.opcode << endl;
    }
}

void Pipeline::setRegistersFromFile(Registers& regs, const string& regsFilename) {
    ifstream regsFile(regsFilename);
    if (!regsFile.is_open()) {
        cerr << "Erro ao abrir o arquivo de registradores: " << regsFilename << endl;
        return;
    }

    string line;
    while (getline(regsFile, line)) {
        int regNum, regValue;
        char virgula;
        stringstream ss(line);

        ss >> regNum >> virgula >> regValue;
        regs.set(regNum, regValue); 
    }
    regsFile.close();
}

