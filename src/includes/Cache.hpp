#ifndef CACHE_HPP
#define CACHE_HPP

#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <list>

using namespace std;

class Cache {
private:
    unordered_map<string, int> cacheMap; 
    list<string> order; 
    int capacidade; 

public:
    Cache(); 
    Cache(int capacidade); 

    void set(const std::string& instruction, int result) {
        if (cacheMap.size() >= capacidade) {
            cout << "Cache cheia, removendo a instrução mais antiga." << endl;
            std::string oldest = order.front(); 
            order.pop_front(); 
            cacheMap.erase(oldest); 
        }
        cacheMap[instruction] = result; 
        order.push_back(instruction); 
    }

    bool get(const std::string& instruction, int& result) {
        auto it = cacheMap.find(instruction);
        if (it != cacheMap.end()) {
            result = it->second; 
            return true; 
        }
        return false; 
    }

    void printCache() {
        cout << "Estado atual da Cache:" << endl;
        for (const auto& instruction : order) {
            cout << "Instrução: " << instruction 
                << ", Resultado: " << cacheMap[instruction] << endl;
        }
    }
};

#endif