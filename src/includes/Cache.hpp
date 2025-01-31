#ifndef CACHE_HPP
#define CACHE_HPP

#include <vector>
#include <iostream>
#include <string>

using namespace std;

class Cache {
private:
    vector<pair<string, int>> cache; 
    int capacidade; 

public:
    Cache(); 
    Cache(int capacidade); 

    void set(const string& instruction, int result) {
        if (cache.size() >= capacidade) {
            cout << "Cache cheia, removendo a instrução mais antiga." << endl;
            cache.erase(cache.begin()); // FIFO
            return;
        }
        cache.push_back({instruction, result});
    }

    bool get(const string& instruction, int& result) {
        for (const auto& entry : cache) {
            if (entry.first == instruction) { // verifica se a instrucao esta na cache
                result = entry.second; 
                return true;
            }
        }
        return false;
    }

    void printCache() {
        cout << "Estado atual da Cache:" << endl;
        for (const auto& entry : cache) {
            cout << "Instrução: " << entry.first 
                 << ", Resultado: " << entry.second << endl;
        }
    }
};

#endif