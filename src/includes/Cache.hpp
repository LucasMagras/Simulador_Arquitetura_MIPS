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

    void set(const std::string& instruction, int result);
    bool get(const std::string& instruction, int& result);
    void printCache();
};

#endif