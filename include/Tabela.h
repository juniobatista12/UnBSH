#ifndef TABELA_H
#define TABELA_H

#include <fstream>
#include <iostream>
#include <vector>
#include "LinhaTabela.h"

using namespace std;

class Tabela{
public:
    vector<LinhaTabela> tabela;
    Tabela(string);
    void printTabela();
};

#endif