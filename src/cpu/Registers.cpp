#include "../includes/Registers.hpp"

Registers::Registers() : registradores(NUM_REGISTRADORES, make_pair(0, false)) {}

int Registers::get(int index) const {
    
    if (index < 0 || index >= NUM_REGISTRADORES) {
        std::cerr << "Erro: Índice de registrador inválido: " << index << std::endl;
        return 0;
    }
    
    return registradores[index].first;
}

void Registers::set(int index, int value) {
    if (index >= 0 && index < registradores.size()) {        
        registradores[index] = {value, true}; 
    } else {
        cerr << "Erro: Registrador inválido " << index << endl;
    }
}


void Registers::display() const {
    cout << endl << "Registradores: " << endl;
    for (size_t i = 0; i < registradores.size(); ++i) {
        cout << "R[" << i << "] = " << registradores[i].first << ", Flag = " << (registradores[i].second ? "sujo" : "limpo") << endl;
    }
}
