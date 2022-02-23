#include "../include/Tabela.h"

Tabela::Tabela (string filenName){
    ifstream fin;
    fin.open(filenName);
    string waste, cmd, alias;
    while(fin >> waste >> cmd >> alias){
        cmd.pop_back();
        cmd.erase(cmd.begin());
        alias.pop_back();
        alias.erase(alias.begin());
        this->tabela.push_back(LinhaTabela(cmd, alias));
    }
}

void Tabela::printTabela(){
    for(auto linha : tabela){
        cout << linha.cmd << " " << linha.alias << endl;
    }
}