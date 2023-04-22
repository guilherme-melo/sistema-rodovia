#include <fstream>
#include <vector>

using namespace std;

typedef vector<tuple<string,int>> VectorLane;
typedef vector<vector<tuple<string,int>>> MatrixRoads;

 
VectorLane* splitData(ifstream file) {
    vector<string> plates;
    vector<int> x;
    vector<int> y;
    VectorLane* p;
    string line;
    VectorLane *pista_anterior_ptr = nullptr;

    // Separa os dados dos arquivos em um vetor que contém uma pista em cada espaço 
    for (int i = 29; i <= 30; i++) {
        string file_name = "../../data/" + to_string(i) + "_mockdata.txt";
        ifstream file(file_name);
        while(getline(file, line)) {

            char delimiter = '(';
            size_t pos_inicio = line.find(delimiter);
            size_t pos_fim = line.find(',', pos_inicio);

            x.push_back(stoi(line.substr(pos_inicio+1, pos_fim-pos_inicio-1)));
            y.push_back(stoi(line.substr(pos_fim+1, 1)));

            plates.push_back(line.substr(0,6));
            
            line.clear();
        }

        file.close();

        auto N_PISTAS = max_element(y.begin(), y.end());
        int n_pistas = *N_PISTAS;

        int plates_size = plates.size();

        for (int i = 0; i < n_pistas; i++) {
            vector<tuple<string, int> > pista;

            // insert plates and x values into pista
            for (int j = 0; j < plates_size; j++) {
                if (y[j] == i) {
                    pista.push_back(make_tuple(plates[j], x[j]));
                }
            }

            por_pista.push_back(pista);
        }

        int por_pista_size = por_pista.size();
        for (int i = 0; i < por_pista_size; i++) {
            calculos_por_pista(por_pista[i]);
        }
    }

}



int main() {

    ifstream data("mais_recente.csv"); //PEGAR O ARQUIVO MAIS RECENTE
    MatrixRoads matrix = splitData(data);

}