#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

struct Pos { int r, c; };
struct Found {
    string word;
    Pos start{};
    string dir;
    vector<Pos> path;
    bool found = false;
};

// 8 direções (inclui diagonais)
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

static inline bool inBounds(int r,int c,int R,int C){
    return r>=0 && r<R && c>=0 && c<C;
}

static bool matchFrom(const vector<string>& g, const string& w,
                      int r,int c,int dr,int dc, vector<Pos>& path){
    const int R = (int)g.size();
    const int C = (int)g[0].size();
    path.clear();

    // Otimização: verifica se a última letra ainda cabe no grid
    int end_r = r + (int(w.size()) - 1) * dr;
    int end_c = c + (int(w.size()) - 1) * dc;
    if (!inBounds(end_r, end_c, R, C)) return false;

    for(int i=0;i<(int)w.size();++i){
        int rr=r+i*dr, cc=c+i*dc;
        if(g[rr][cc]!=w[i]) return false;
        path.push_back({rr,cc});
    }
    return true;
}

static string dirName(int dr,int dc){
    for (auto& d : DIRS)
        if (d.first.first==dr && d.first.second==dc) return d.second;
    return "?";
}

static Found findWord(const vector<string>& gridLower, const string& wLower){
    const int R=(int)gridLower.size();
    const int C=(int)gridLower[0].size();

    Found ans; ans.word = wLower;
    string wRev = wLower; reverse(wRev.begin(), wRev.end());

    for(int r=0;r<R;++r){
        for(int c=0;c<C;++c){
            char ch = gridLower[r][c];
            if(ch!=wLower[0] && ch!=wRev[0]) continue;

            for (auto& d : DIRS){
                vector<Pos> path;
                int dr=d.first.first, dc=d.first.second;
                if (matchFrom(gridLower, wLower, r, c, dr, dc, path)){
                    ans.found=true; ans.start={r,c}; ans.dir=d.second; ans.path=path; return ans;
                }
            }
            for (auto& d : DIRS){
                vector<Pos> path;
                int dr=d.first.first, dc=d.first.second;
                if (matchFrom(gridLower, wRev, r, c, dr, dc, path)){
                    reverse(path.begin(), path.end());
                    ans.found=true; ans.start=path.front();
                    ans.dir = dirName(-dr, -dc);
                    ans.path=path; return ans;
                }
            }
        }
    }
    return ans;
}

// Lê: R C, depois R linhas, depois palavras (uma por linha ou separadas por espaço)
static bool readSingleFile(istream& in, vector<string>& grid, vector<string>& words){
    int R,C; 
    if(!(in>>R>>C)) return false;
    string line; getline(in,line); // consome resto
    grid.clear(); words.clear();

    for(int i=0;i<R;++i){
        if(!getline(in,line)) return false;
        // remove todos os espaços em branco
        line.erase(remove_if(line.begin(), line.end(), [](unsigned char ch){return std::isspace(ch)!=0;}), line.end());
        if((int)line.size()!=C) return false;
        grid.push_back(line);
    }
    while(getline(in,line)){
        string cur;
        for(char ch: line){
            if(isspace((unsigned char)ch)){
                if(!cur.empty()){ words.push_back(cur); cur.clear(); }
            }else cur.push_back(ch);
        }
        if(!cur.empty()) words.push_back(cur);
    }
    return true;
}

// Lê dois arquivos: grade (R C + R linhas) e lista (uma por linha)
static bool readTwoFiles(const string& gridFile,const string& wordsFile,
                         vector<string>& grid, vector<string>& words){
    ifstream ig(gridFile);
    if(!ig) return false;
    // Lê apenas a grade do primeiro arquivo
    if(!readSingleFile(ig, grid, words)) return false;

    // Lê palavras do segundo arquivo (se existir)
    ifstream iw(wordsFile);
    if(iw){
        words.clear();
        string w, tok;
        while(getline(iw,w)){
            tok.clear();
            for(char ch:w){
                if(isspace((unsigned char)ch)){
                    if(!tok.empty()){ words.push_back(tok); tok.clear(); }
                }else tok.push_back(ch);
            }
            if(!tok.empty()) words.push_back(tok);
        }
    }
    return true;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc>=2 && std::string(argv[1])=="--help") {
        cerr << "Uso:\n"
             << "  " << argv[0] << "                (le de stdin: R C, R linhas da grade e depois palavras)\n"
             << "  " << argv[0] << " arquivo.txt     (R C + grade + palavras no mesmo arquivo)\n"
             << "  " << argv[0] << " grade.txt palavras.txt\n";
        return 0;
    }

    vector<string> grid, words;
    if(argc==1){
        if(!readSingleFile(cin, grid, words)){
            cerr<<"Erro ao ler entrada do stdin.\n"; 
            return 1;
        }
    }else if(argc==2){
        ifstream in(argv[1]);
        if(!in || !readSingleFile(in, grid, words)){
            cerr<<"Erro ao ler arquivo: "<<argv[1]<<"\n"; 
            return 1;
        }
    }else{
        if(!readTwoFiles(argv[1], argv[2], grid, words)){
            cerr<<"Erro ao ler arquivos: "<<argv[1]<<" e/ou "<<argv[2]<<"\n"; 
            return 1;
        }
    }

    // Robustez: checa grid
    if (grid.empty() || grid[0].empty()){
        cerr << "Erro: grade vazia ou dimensoes invalidas.\n";
        return 1;
    }

    const int R=(int)grid.size();
    const int C=(int)grid[0].size();

    // grade minúscula
    vector<string> gridLower = grid;
    for(auto& row: gridLower) 
        for(char& ch: row) ch=(char)tolower((unsigned char)ch);

    // preserva originais e normaliza minúsculas
    vector<string> wordsOrig = words;
    for(auto& w: words){
        string t; 
        for(char ch:w) if(!isspace((unsigned char)ch)) t.push_back((char)tolower((unsigned char)ch));
        w = t;
    }

    // --- filtros seguros ---
    // 1) remove palavras vazias
    {
        vector<string> w2, wo2;
        w2.reserve(words.size()); wo2.reserve(wordsOrig.size());
        for(size_t i=0;i<words.size();++i){
            if(!words[i].empty()){ w2.push_back(words[i]); wo2.push_back(wordsOrig[i]); }
        }
        words.swap(w2); wordsOrig.swap(wo2);
    }
    // (REMOVIDO) Nao descartar palavras iguais a linhas da grade

    // busca paralela (1 thread por palavra)
    vector<Found> results(words.size());
    vector<vector<bool>> upperMask(R, vector<bool>(C,false));
    vector<thread> ts;
    ts.reserve(words.size());

    for(size_t i=0;i<words.size();++i){
        ts.emplace_back([&, i](){
            Found f = findWord(gridLower, words[i]);
            // Sem mutex: cada thread grava apenas em results[i]
            results[i] = f;
            results[i].word = wordsOrig[i];
        });
    }
    for(auto& t: ts) t.join();

    // aplica maiúsculas nas letras encontradas
    for(const auto& f: results){
        if(!f.found) continue;
        for(const auto& p: f.path) upperMask[p.r][p.c] = true;
    }
    vector<string> out = grid;
    for(int r=0;r<R;++r)
        for(int c=0;c<C;++c)
            if(upperMask[r][c]) out[r][c]=(char)toupper((unsigned char)out[r][c]);

    // imprime matriz destacada
    for(const auto& row: out) cout<<row<<"\n";
    cout << "\n";

    // imprime lista (1-based)
    for(size_t i=0;i<results.size();++i){
        const auto& f = results[i];
        if(f.found){
            cout << results[i].word << " - (" << (f.start.r+1) << "," << (f.start.c+1) << "):" << f.dir << "\n";
        }else{
            if(!results[i].word.empty())
                cout << results[i].word << ": nao encontrada\n";
        }
    }
    return 0;
}