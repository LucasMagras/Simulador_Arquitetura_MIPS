#include "../includes/UnidadeControle.hpp"

UnidadeControle::UnidadeControle(int numeroCores, RAM& ram, Disco& disco, Cache& cache, const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros) : processManager(numeroCores, ram, disco, cache) { 
    carregarArquivo(processManager, arquivosInstrucoes, arquivosRegistros);
}

void UnidadeControle::carregarArquivo(ProcessManager& processManager, const vector<string>& arquivosInstrucoes, const vector<string>& arquivosRegistros)
{
    //processManager.carregarProcessosPrioridade(arquivosInstrucoes, arquivosRegistros);
    //processManager.escalonarProcessosPrioridade();
    //processManager.carregarProcessoSJF(arquivosInstrucoes, arquivosRegistros);
    //processManager.escalonarSJF();
    //processManager.carregarProcessosFCFS(arquivosInstrucoes, arquivosRegistros);
    //processManager.carregarProcessosPorSimilaridade(arquivosInstrucoes, arquivosRegistros);
    //processManager.escalonarProcessosFCFS();
    processManager.carregarProcessoMMU(arquivosInstrucoes, arquivosRegistros);
    processManager.escalonarMMU();
}