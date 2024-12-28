#include "includes/Opcode.hpp"
#include "includes/Instruction.hpp"
#include "includes/Registers.hpp"
#include "includes/ULA.hpp"
#include "includes/UnidadeControle.hpp"
#include "includes/RAM.hpp"
#include "includes/InstructionDecode.hpp"
#include "includes/Pipeline.hpp"
#include "includes/Core.hpp"
#include "includes/perifericos.hpp"
#include "includes/ProcessManager.hpp"

#include <iostream>

using namespace std;


int main() {
    try {
        RAM ram;
        Disco disco;
        Perifericos perifericos;
        Core core;

        //perifericos.estadoPeriferico("teclado", true);
        //perifericos.estadoPeriferico("mouse", true);

        int numeroCores = 2;  

        vector<string> arquivosProcessos = {
            "data/instructions.txt",
            "data/instructions2.txt",
            //"data/instructions3.txt",
            //"data/instructions4.txt",
            "data/instructions5.txt",
            "data/instructions6.txt"
        };

        vector<string> arquivosRegs = {
            "data/setRegisters.txt",
        };

        //carrega os processos
        UnidadeControle uc(numeroCores, ram, disco, arquivosProcessos, arquivosRegs);

        cout << "\n--- Estado Final da RAM ---" << endl;
        ram.display();

        cout << "\n--- Estado Final do Disco ---" << endl;
        disco.display();

    } catch (const exception& e) {
        cout << "Erro durante a execução: " << e.what() << endl;
        return 1;
    }

    return 0;
}