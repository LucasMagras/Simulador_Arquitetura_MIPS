#include "../includes/Cache.hpp"

Cache::Cache() : capacidade(5) {}

Cache::Cache(int capacidade) : capacidade(capacidade) {}

void Cache::set(const std::string& instruction, int result) {
        if (cacheMap.size() >= capacidade) {
            cout << "Cache cheia, removendo a instrução mais antiga." << endl;
            std::string oldest = order.front(); 
            order.pop_front(); 
            cacheMap.erase(oldest); 
        }
        cacheMap[instruction] = result; 
        order.push_back(instruction); 
}

bool Cache::get(const std::string& instruction, int& result) {
        auto it = cacheMap.find(instruction);
        if (it != cacheMap.end()) {
            result = it->second; 
            return true; 
        }
        return false; 
}

void Cache::printCache() {
        cout << "Estado atual da Cache:" << endl;
        for (const auto& instruction : order) {
            cout << "Instrução: " << instruction 
                << ", Resultado: " << cacheMap[instruction] << endl;
        }
}

