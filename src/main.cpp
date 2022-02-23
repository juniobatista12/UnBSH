#include "../include/Tabela.h"
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SIZE_PATH 100

vector<vector<string>> historico;
vector<string> path;
Tabela tabela = Tabela(".unbshrc");
vector<int> pids;

string adicionaBarra(char *cwd);
vector <string> split(string linha, char caractere);
void printVetor(vector<char *> tokens);
void printVetor(vector<string> tokens);
void executa(vector<string> linhaComando, string inArquivo, string outArquivo);
void trataBuiltin(vector<string> linhaComando, string inArquivo, string outArquivo);
void parse(vector<string> tokens);
bool verificaFilhos();

int main(int argc, char *argv[]){
    ifstream fin;                                                   // Stream utilizado para abertura de arquivo em lotes e arquivos de configuracao
    string tmp;                                                     // String temporária para guardar os paths ate ocorrer o split
    fin.open(".unbshrc_profile");                                   // Abro arquivo que contem os paths para o UnBSh
    fin >> tmp;                                                     // Leio a linha contendo os paths
    fin.close();                                                    // Fecho o arquivo para que seja utilizado posteriormente se necessario
    path = split(tmp, ';');                                         // Faço um split nos ; para separar cada caminho
    path[0].erase(path[0].begin(), path[0].begin() + 5);            // Apago a palavra PATH= que contem no inicio da linha
    // printVetor(path);                                               // Debug apenas para conferir os paths
    // tabela.printTabela();                                           // Debug para conferir os aliases definidos na tabela de alias
    string entrada;                                                 // String utilizada para receber o comando, seja vindo do usuario, seja do arquivo em lotes
    char cwd[100];                                                  // Array utilizado para guardar o caminho atual, podemos observar que o mesmo eh limitado em 100 caracteres
    if (argc == 1){                                                 // Nesse caso, foi executado somente o terminal, logo sera usado o modo interativo com o usuario
        while (entrada != "exit" && entrada != "sair"){             // Loop que faz com que o UnBSh seja executado ate que utilizem a palavra-chave exit ou sair
            getcwd(cwd,sizeof(cwd));                                // Pegando o caminho atual
            string login = getlogin();                              // Pegando o nome de usuario
            cout << "UnBsh-" << login << "-" << cwd<< "->";         // Imprimindo que o UnBSh esta pronto para receber comandos
            if(verificaFilhos()){                                   // Verifica se algum filho quer ser noticiado como morto e imprime o que for necessario
                cout << "UnBsh-" << login << "-" << cwd<< "->";     // Imprime novamente a linha de pronto
            }
            getline(cin, entrada);                                  // Recebe a entrada do usuario
            vector<string> tokens = split(entrada, ' ');            // Separa os tokens
            path.push_back(adicionaBarra(cwd));                     // Adiciona o caminho atual ao path, porem adicionando uma barra ao fim para que o caminho fique correto
            parse(tokens);                                          // Analisa os tokens e executa o comando
            path.pop_back();                                        // Retira o caminho atual do vetor de Paths
        }
    }
    else{                                                           // Caso esteja usando um arquivo
        fin.open(argv[1]);                                          // Abre o arquivo que foi passado pela linha de comando
        while (getline(fin, entrada)){                              // E, enquanto consegue ler, roda esse loop
            if (entrada[0] == '#'){                                 // Caso a linha inicie por #, significa que eh um comentario, portanto ignora, porem, caso a estrutura de arquivos
                continue;                                           // estivesse bem definida, poderia verificar se o path do UnBSh estava correto aqui
            }
            vector<string> tokens = split(entrada, ' ');            // Separa os tokens
            getcwd(cwd, sizeof(cwd));                               // Pega o caminho atual
            path.push_back(adicionaBarra(cwd));                     // Adiciona ao Path
            parse(tokens);                                          // Analisa e executa
            path.pop_back();                                        // Remove o caminho do path
        }
    }
}

string adicionaBarra(char *cwd){
    int i = 0;
    string tmp;
    while (cwd[i]){                                                 // Busca o fim da string copiando caracteres para a string temporaria
        tmp += cwd[i++];
    }
    tmp += "/";                                                     // Adiciona uma barra ao fim da string temporaria
    return tmp;                                                     // Retorna a string temporaria
}

vector <string> split(string linha, char caractere){
    vector<string> tokens;
    string buffer;
    for (char letra : linha){                                       // Varre caractere a caractere da string inicial
        if (letra == caractere && buffer != ""){                    // Se encontrar o caractere separador e o buffer de armazenamento nao estiver vazio
            tokens.push_back(buffer);                               // O token eh colocado no vetor de tokens
            buffer = "";                                            // e o buffer eh esvaziado
        }
        else{
            buffer += letra;                                        // Do contrario, adiciona-se o caractere ao buffer
        }
    }
    if (buffer != "")
        tokens.push_back(buffer);                                   // Verifica se o ultimo buffer nao estava vazio e adiciona o ultimo buffer
    return tokens;                                                  // Retorna o vetor de tokens
}

void printVetor(vector<char *> tokens){
    for (auto token : tokens){                                      // Imprime cada char * de um vetor separado por " " e ao fim quebra a linha
        cout << token << " ";                                       // Usado para debugs
    }
    cout << endl;
}

void printVetor(vector<string> tokens){
    for (auto token : tokens){                                      // Overload do metodo anterior para vetor de strings
        cout << token << " ";
    }
    cout << endl;
}

void printVetor(vector<int> tokens){
    for (auto token : tokens){                                      // Overload do metodo anterior para vetor de ints
        cout << token << " ";
    }
    cout << endl;
}

void executa(vector<string> linhaComando, string inArquivo, string outArquivo, bool appArquivo){
    int pid = fork();                                                       // Clono o processo e capturo o pid do filho gerado
    bool background = linhaComando[linhaComando.size() - 1] == "&";         // Verifico se o processo deve ser rodado em background
    if (background){
        linhaComando.pop_back();                                            // Removo o & caso exista da lista de argumentos
    }
    if (!pid){                                                              // No processo filho
        if (inArquivo != ""){                                               // Verifico se ha um arquivo de entrada
            int fin = open(inArquivo.c_str(), O_RDONLY);                    // Caso exista, abro o arquivo em modo de leitura apenas
            dup2(fin, 0);                                                   // E uso o dup2 para substituir o stdin como file descriptor
        }
        if (outArquivo != ""){                                              // Verifico se ha arquivo de saida
            int mask = appArquivo ? O_APPEND : 0;                           // Crio uma mascara que dira se deve ter append no arquivo de saida ou nao
            if (!mask)                                                      // Se nao tiver que fazer append, tento remover o arquivo para iniciar um novo
                remove(outArquivo.c_str());
            int fout = open(outArquivo.c_str(), O_CREAT | mask |  O_WRONLY, S_IRWXU | S_IRWXG); // Crio o arquivo em modo de escrita, usando a mascara e dando permissoes totais para o usuario e o grupo dele
            dup2(fout, 1);                                                  // Substituo o stdout pelo file descriptor do arquivo criado
        }
        for (string caminho : path){                                        // Para cada path no vetor
            vector<string> copia = linhaComando;                            // crio uma copia da linha de comando a ser executada
            copia[0] = caminho + copia[0];                                  // concateno o caminho com o programa a ser executado
            vector<char *> args;                                            // inicializo um vetor de char * para receber a linha de comando convertida de string para char *                 
            for (int i = 0; i < copia.size(); i++){                         // converto cada argumento para char *
                args.push_back(&*copia[i].begin());
            }
            args.push_back(nullptr);                                        // coloco um nullptr no fim para indicar ao execv que acabou o vetor
            execv(args[0], args.data());                                    // e tento executar com os argumentos assim, caso haja sucesso, o filho morrera
        }                                                                   // e sera substituido pelo comando desejado
        cout << "Comando nao encontrado" << endl;                           // porem, se nenhum comando executar com sucesso, significa que temos um comando invalido
    }
    else{                                                                   // Porem se estamos falando do processo pai
        if (background){                                                    // temos que verificar se o filho roda em background ou em foreground
            cout << "Processo em background [" << pid << "] foi iniciado" << endl;  // Caso seja em background, imprimo que o filho foi iniciado em determinado PID
            pids.push_back(pid);                                            // Adiciono o PID desse filho a lista de PIDs ativos que devem ser monitorados
        }
        else {                                                              // Porem, estando em foreground,
            waitpid(pid, NULL, 0);                                          // devemos esperar o fim da execucao do comando
        }
    }
}

void trataBuiltin(vector<string> linhaComando, string inArquivo, string outArquivo, bool appArquivo){ // Essa funcao verifica se o comando se trata de um builtin do UnBSh ou de um utilitario que deve ser executado
    if (linhaComando[0] == "cd"){                                           // Verifico se eh um cd
        chdir(linhaComando[1].c_str());                                     // e troco de diretorio (possivel melhoria, verificar o retorno e avisar de erros)
    }
    else if (linhaComando[0] == "rm"){                                      // Verifico se eh um rm
        if(remove(linhaComando[1].c_str()) == -1){                          // e removo o arquivo (rmdir poderia ser algo a ser implementado)
            cout << "Erro ao deletar arquivo" << endl;
        }
    }
    else if(linhaComando[0] == "historico"){                                // Verifico se eh um historico
        if (linhaComando.size() != 1){                                      // Se for um historico e tiver algo alem de historico
            parse(historico[stoi(linhaComando[1])]);                        // Peco que o comando na posicao pedida seja executado novamente
        }
        else {                                                              // Se for apenas historico
            for (int i = 0; i < historico.size(); i++){                     // Imprimo comando a comando que consta no historico
                cout << i << " ";
                printVetor(historico[i]);
            }
        }
    }
    else if (linhaComando[0] == "ver"){                                     // Verifico se nao devo dar informacoes do UnBSh desenvolvido por mim
        cout << "v2.0" << endl << "Ultima atualizacao: 18/02/2022" << endl << "Autor: Divino Junio Batista Lopes" << endl;
    }
    else if (linhaComando[0] == "exit" || linhaComando[0] == "sair"){}      // Verifico se nao eh um comando de saida
    else                                                                    // Nao se encaixando em nenhum, tento executar tal comando
        executa(linhaComando, inArquivo, outArquivo, appArquivo);
}

void executaFD(vector<string> command, int fdIn, int fdOut, string outArquivo, bool appArquivo){
    int pid = fork();                                                       // Muito dessa funcao se assemelha a executa, porem esse eh voltado para
    if(!pid){                                                               // receber descitores de arquivo ao inves de strings para arquivos em disco
        dup2(fdIn, 0);
        if(outArquivo == ""){
            dup2(fdOut, 1);        
        }
        else{
            int mask = appArquivo ? O_APPEND : 0;
            if (!mask)
                remove(outArquivo.c_str());
            int fout = open(outArquivo.c_str(), O_CREAT | mask |  O_WRONLY, S_IRWXU | S_IRWXG);
            dup2(fout, 1);
        }
        for (string caminho : path){        
            vector<string> copia = command;
            copia[0] = caminho + copia[0];
            vector<char *> args;
            for (int i = 0; i < copia.size(); i++){
                args.push_back(&*copia[i].begin());
            }
            args.push_back(nullptr);
            execv(args[0], args.data());
        }
        cout << "Comando não encontrado" << endl;
    }
    else{                                                                   // Nao foi implementado background, visto que piping requer coisas sequenciais
        int status;                                                        
        waitpid(pid, NULL, 0);
    }
}

void executaPipe(vector<vector<string>> commands, string outArquivo, bool appArquivo){
    int fd1[2], fd0[2];                                                     // Vetores de inteiros para fazer pipes
    pipe(fd1);                                                              // Crio o primeiro pipe
    for (int i = 0; i < commands.size(); i++){                              // Para cada pipe section, verifica se eh o primeiro ou ultimo e executa 
        if (i == commands.size() - 1){                                      // corrigindo os pipes a cada execucao
            executaFD(commands[i], fd0[0], 1, outArquivo, appArquivo);
        }
        else if (i){
            executaFD(commands[i], fd0[0], fd1[1], "", appArquivo);
            close(fd1[1]);
        }
        else{
            executaFD(commands[i], 0, fd1[1], "", appArquivo);
            close(fd1[1]);
        }
        pipe(fd0);
        dup2(fd1[0], fd0[0]);
        dup2(fd1[1], fd0[1]);
        pipe(fd1);
    }
}

void parse(vector<string> tokens){
    bool piped = false;                                                         // Booleano que indica que foi encontrado um pipe
    string inArquivo = "", outArquivo = "";                                     // Variavel para indicar arquivo de entrada ou saida
    bool appArquivo = false;                                                    // Booleano que indica se ha append ou nao
    vector<vector<string>> pipedCommands;                                       // Vetor para pipe sections
    historico.push_back(tokens);                                                // Insere o ultimo comando no historico
    if (historico.size() > 10){                                                 // Caso tenha estourado o limite de 10, remove o mais antigo
        historico.erase(historico.begin());
    }
    while (tokens.size()){
        vector<string> linhaComando;                                            // Vetor no qual sera montado o comando de verdade
        for (LinhaTabela linha : tabela.tabela){                                // Busca na tabela o alias e substitui pelo comando
            if(tokens[0] == linha.alias){
                tokens[0] = linha.cmd;
            }
        }
        for (int i = 0; tokens.size() && tokens[i] != "|" && tokens[i] != "<" && tokens[i] != ">" && tokens[i] != ">>";){   // Procura por algo que separe secoes
            linhaComando.push_back(tokens[i]);
            tokens.erase(tokens.begin());
        }
        if (tokens[0] == "<"){                                                  // Verifica se ha arquivo de entrada
            tokens.erase(tokens.begin());
            inArquivo = tokens[0];
            tokens.erase(tokens.begin());
        }
        else if (tokens[0] == ">"){                                             // Verifica se ha arquivo de saida
            tokens.erase(tokens.begin());
            outArquivo = tokens[0];
            tokens.erase(tokens.begin());
        }
        else if(tokens[0] == ">>"){                                             // Verifica se ha arquivo para append
            tokens.erase(tokens.begin());
            outArquivo = tokens[0];
            appArquivo = true;
            tokens.erase(tokens.begin());
        }
        else if (tokens[0] == "|"){                                             // Verifica se eh uma pipe section
            tokens.erase(tokens.begin());
            piped = true;
        }
        if (!piped)
            trataBuiltin(linhaComando, inArquivo, outArquivo, appArquivo);      // Se nao houve pipe, ele executa o comando
        else
            pipedCommands.push_back(linhaComando);                              // Do contrario, inclui no vetor de comandos do pipe
    }
    if(piped)
        executaPipe(pipedCommands, outArquivo, appArquivo);                     // Se ocorreu um pipe, executa todos os pipes
}

bool verificaFilhos(){
    int status;
    bool achou = false;
    // printVetor(pids);
    for (int i = 0; i < pids.size(); i ++){
        if(waitpid(pids[i], &status, WNOHANG | WCONTINUED)){                    // Verifica PID a PID se algum filho foi morto
            cout << "Processo [" << pids[i] << "] executado" << endl;           // Caso sim, retorna a mensagem (poderia ser criado um vetor com o nome ou a linha de comando)
            pids.erase(pids.begin() + i);                                       // O PID eh removido do vetor de PIDs ativo
            achou = true;                                                       // Notifica o loop principal que algum filho morreu
        }
    }
    return achou;
}
// char *args[] = {"ls" "-hla", NULL}
// execv(args[0], args)