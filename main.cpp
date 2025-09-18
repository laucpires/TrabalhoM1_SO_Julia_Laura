#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
using namespace std;

const vector<pair<pair<int,int>, const char*>> DIRS = {
    {{ 0,  1}, "direita"},
    {{ 0, -1}, "esquerda"},
    {{ 1,  0}, "baixo"},
    {{-1,  0}, "cima"},
    {{ 1,  1}, "baixo-direita"},
    {{ 1, -1}, "baixo-esquerda"},
    {{-1,  1}, "cima-direita"},
    {{-1, -1}, "cima-esquerda"}
};

// Representa uma posição no grid (linha r, coluna c)
struct Pos { int r, c; };
// Estrutura que guarda o resultado da busca de uma palavra
struct Found {
    string word;        // palavra buscada
    Pos start{};        // posição inicial (linha,coluna)
    string dir;         // direção encontrada (ex: "direita/cima")
    vector<Pos> path;   // todas as coordenadas da palavra no grid
    bool found = false; // se foi encontrada ou não
};
// Verifica se uma posição (r,c) está dentro dos limites do grid
static inline bool inBounds(int r,int c,int R,int C){
    return r>=0 && r<R && c>=0 && c<C;
}
// Tenta casar a palavra 'w' no grid 'g' começando da posição (r,c)
// avançando na direção (dr,dc). Se casar, retorna true e preenche o path.
static bool matchFrom(const vector<string>& g, const string& w,
                      int r,int c,int dr,int dc, vector<Pos>& path) {
    int R = (int)g.size();
    int C = (int)g[0].size();
    path.clear();
    // Calcula onde a palavra terminaria
    int end_r = r + ((int)w.size() - 1) * dr;
    int end_c = c + ((int)w.size() - 1) * dc;
    if (!inBounds(end_r, end_c, R, C)) return false; // sairia do grid
    // Verifica caractere por caractere
    for(int i=0; i<(int)w.size(); ++i) {
        int rr = r + i*dr, cc = c + i*dc;
        if(g[rr][cc] != w[i]) return false; // falhou
        path.push_back({rr, cc});
    }
    return true;
}
// Função para retornar o nome da direção usando o vetor DIRS
static string getDirName(int dr, int dc) {
    for(const auto& dir : DIRS) {
        if(dir.first.first == dr && dir.first.second == dc) {
            return dir.second;
        }
    }
    return "?";
}
// Procura uma palavra (normal ou invertida) no grid
// Retorna a posição inicial, direção e caminho se encontrada
static Found findWord(const vector<string>& gridLower, const string& wLower){
    int R = (int)gridLower.size(), C = (int)gridLower[0].size();
    Found ans; ans.word = wLower;
    // Também procura a palavra ao contrário
    string wRev = wLower; reverse(wRev.begin(), wRev.end());
    // Varre todas as posições do grid
    for(int r=0; r<R; ++r){
        for(int c=0; c<C; ++c){
            char ch = gridLower[r][c];
            // Só tenta se a letra inicial bater
            if(ch!=wLower[0] && ch!=wRev[0]) continue;
            // Testa todas as 8 direções
            for(int dr=-1; dr<=1; ++dr){
                for(int dc=-1; dc<=1; ++dc){
                    if(dr==0 && dc==0) continue;
                    vector<Pos> path;
                    // Caso palavra "normal"
                    if(matchFrom(gridLower, wLower, r, c, dr, dc, path)){
                        ans.found=true;
                        ans.start={r,c};
                        ans.dir=getDirName(dr,dc);
                        ans.path=path;
                        return ans;
                    }
                    // Caso palavra invertida
                    if(matchFrom(gridLower, wRev, r, c, dr, dc, path)){
                        reverse(path.begin(), path.end());
                        ans.found=true;
                        ans.start=path.front();
                        ans.dir=getDirName(-dr,-dc); // direção invertida
                        ans.path=path;
                        return ans;
                    }
                }
            }
        }
    }
    return ans;
}
int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    // Checagem dos argumentos
    if(argc!=2){
        cerr << "Uso: " << argv[0] << " cacapalavras.txt\n";
        return 1;
    }
    ifstream in(argv[1]);
    if(!in){
        cerr<<"Erro ao abrir arquivo: "<<argv[1]<<"\n";
        return 1;
    }
    // Lê dimensões do grid
    int R, C;
    in >> R >> C;
    string line;
    getline(in, line); // consome o resto da linha
    // Lê o grid de letras
    vector<string> grid;
    for(int i=0; i<R; ++i){
        getline(in, line);
        // Remove espaços em branco
        line.erase(remove_if(line.begin(), line.end(), 
                  [](unsigned char ch){return isspace(ch)!=0;}), line.end());
        // Valida tamanho da linha
        if((int)line.size() != C){
            cerr << "Erro: linha " << (i+1) << " não tem tamanho correto\n";
            return 1;
        }
        grid.push_back(line);
    }
    // Lê lista de palavras (resto do arquivo)
    vector<string> words;
    vector<string> wordsOrig;
    while(getline(in, line)){
        if(line.empty()) continue;
        string cur;
        for(char ch : line){
            if(isspace((unsigned char)ch)){
                if(!cur.empty()){
                    wordsOrig.push_back(cur);
                    string t;
                    for(char cc : cur) t += (char)tolower((unsigned char)cc);
                    words.push_back(t);
                    cur.clear();
                }
            } else cur.push_back(ch);
        }
        if(!cur.empty()){
            wordsOrig.push_back(cur);
            string t;
            for(char cc : cur) t += (char)tolower((unsigned char)cc);
            words.push_back(t);
        }
    }
    // Validação do grid
    if(grid.empty() || grid[0].empty()){
        cerr << "Erro: grid vazio ou dimensões inválidas.\n";
        return 1;
    }
    // Cria versão minúscula do grid para busca
    vector<string> gridLower = grid;
    for(auto& row: gridLower)
        for(char& ch: row) ch = (char)tolower((unsigned char)ch);
    // Vetor de resultados e máscara para destacar letras encontradas
    vector<Found> results(words.size());
    vector<vector<bool>> upperMask(R, vector<bool>(C, false));
    // Cria uma thread para cada palavra (busca paralela)
    vector<thread> ts;
    ts.reserve(words.size());
    for(size_t i=0; i<words.size(); ++i){
        ts.emplace_back([&, i](){
            Found f = findWord(gridLower, words[i]);
            results[i] = f;
            results[i].word = wordsOrig[i]; // mantém original
        });
    }
    for(auto& t: ts) t.join();
    // Marca letras encontradas para transformar em maiúsculas
    for(const auto& f: results){
        if(!f.found) continue;
        for(const auto& p: f.path) {
            if(inBounds(p.r, p.c, R, C)) {
                upperMask[p.r][p.c] = true;
            }
        }
    }
    // Cria saída final (grid com letras encontradas em maiúsculo)
    vector<string> out = grid;
    for(int r=0; r<R; ++r)
        for(int c=0; c<C; ++c)
            if(upperMask[r][c]) out[r][c] = (char)toupper((unsigned char)out[r][c]);
    // Mostra grid processado
    for(const auto& row: out) cout << row << endl;
    // Mostra resultado da busca palavra por palavra
    for(size_t i=1; i<results.size(); ++i){
        const auto& f = results[i];
        if(f.found){
            cout << f.word << " – (" << (f.start.r+1) << "," << (f.start.c+1) << "):" << f.dir << endl;
        } else if(!f.word.empty()){
            cout << f.word << ": não encontrada" << endl;
        }
    }
}
