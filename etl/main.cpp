#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <thread>

using namespace std;

typedef vector<tuple<string,int>> Lane;
typedef vector<Lane> Road;


//PREPARE DATA----------------------------------------------------------------------------------
Road splitData(string fileName) {
    vector<string> plates;
    vector<int> x;
    vector<int> y;
    Road por_pista;
    string line;
    Lane *pista_anterior_ptr = nullptr;

    // Separa os dados dos arquivos em um vetor que contém uma pista em cada espaço 
    ifstream file(fileName);
    while(getline(file, line)) {
        
        // Le as strings e 
        char delimiter = '(';
        size_t pos_inicio = line.find(delimiter);
        size_t pos_fim = line.find(',', pos_inicio);

        x.push_back(stoi(line.substr(pos_inicio+1, pos_fim-pos_inicio-1)));
        y.push_back(stoi(line.substr(pos_fim+1, 1)));

        plates.push_back(line.substr(0,7));
        line.clear();
    }

    file.close();

    auto N_PISTAS = max_element(y.begin(), y.end());
    int n_pistas = *N_PISTAS+1;

    int plates_size = plates.size();
    for (int pista_atual = 0; pista_atual < n_pistas; pista_atual++) {
        Lane pista;

        // insert plates and x values into pista
        for (int carro_atual = 0; carro_atual < plates_size; carro_atual++) {
            if (y[carro_atual] == pista_atual) {
                pista.push_back(make_tuple(plates[carro_atual], x[carro_atual]));
            }
        }
        por_pista.push_back(pista);
    }
    return por_pista;
}

//THREAD 1 ( RODOVIAS) ----------------------------------------------------------------------------------
vector<string> get_roads() {
    const char* folder_path = "./data/";
    vector<string> folders;
    DIR* dirp = opendir(folder_path);

    struct dirent* entry;

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_DIR) {
            folders.push_back(entry->d_name);
        }
    }

    closedir(dirp);

    return folders;
}    

//THREAD 2 (CALCULA VELOCIDADE, ACELERACAO, RISCO DE COLISAO) ----------------------------------------------------------------------------------
//
using namespace std;

// vetor de tuplas (placa, posicao/velocidade/aceleracao) para cada pista
typedef vector<tuple<string,int>> Lane; 

// calcula a velocidade de cada carro dada a posicao atual e a do ciclo anterior
// as posicoes anteriores sao passadas por referencia para que possam ser atualizadas
Road calc_speed(Road positions, Road* old_position) {
    Lane calculated_speed;

    int positions_size = positions[0].size();
    // para cada posicao atual
    for (int j = 0; j < positions.size(); j++){
        for(int i =0; i < positions_size; i++) {
            // procura a posicao anterior do mesmo carro (pela placa)
            for (int k = 0; k < old_position->size(); k++) {
                for (int l = 0; l < old_position->at(0).size(); l++){
                    if (get<0>(positions[i][j]) == get<0>(old_position->at(k)[l])) {
                        // calcula a velocidade (velocidade atual - antiga)
                        int speed = get<1>(positions[i][j]) - get<1>(old_position->at(k)[l]);
                        calculated_speed.push_back(make_tuple(get<0>(positions[i][j]), speed));
                    }
                }
            }
        }
    }

    // atualiza as posicoes anteriores
    old_position = &positions;
    return calculated_speed;
}

// calcula a aceleracao de cada carro dada a velocidade atual e a do ciclo anterior
Lane calc_accel(Lane speed, Lane* old_speeds) {
    Lane calculated_accel;

    int speed_size = speed.size();
    // para cada velocidade atual
    for(int i =0; i< speed.size(); i++) {
        // procura a velocidade anterior do mesmo carro (pela placa)
        for (int j = 0; j < old_speeds->size(); j++) {
            if (get<0>(speed[i]) == get<0>(old_speeds->at(j))) {
                // calcula a aceleracao (aceleracao atual - antiga)
                int accel = get<1>(speed[i]) - get<1>(old_speeds->at(i));
                calculated_accel.push_back(make_tuple(get<0>(speed[i]), accel));
            }
        }
    }

    // atualiza as velocidades anteriores
    old_speeds = &speed;
    return calculated_accel;
}

// calcula o risco de colisão de cada carro dada a posicao, velocidade e aceleracao calculadas
Lane calc_collision_risk(Lane positions, Lane speed, Lane accel) {
    Lane collision_risk;

    // ordena os carros por posicao
    sort(positions.begin(), positions.end(), [](const tuple<string,int>& a, const tuple<string,int>& b) {
        return get<1>(a) < get<1>(b);
    });

    // encontra a velocidade e aceleracao de cada carro
    for (int i = 0; i < positions.size(); i++) {
        for (int j = 0; j < speed.size(); j++) {
            for (int k = 0; k < accel.size(); k++) {
                if (get<0>(positions[i]) == get<0>(speed[j]) && get<0>(positions[i]) == get<0>(accel[k])) {
                    // calcula a nova posicao do carro, mas escreve mantendo a ordem original (da posicao antiga) 
                    int new_pos = get<1>(positions[i]) + get<1>(speed[j]) + get<1>(accel[k]);
                    positions[i] = make_tuple(get<0>(positions[i]), new_pos);
                }
            }
        }
    }

    // checa se algum carro vai colidir com o carro a frente, e marca o risco de colisao
    // utilizamos 1 caso haja risco de colisao, e 0 caso contrario
    bool previous_collision = false; // booleano para marcar se o carro anterior ja colidiu
    for (int i = 0; i < positions.size(); i++) {
        if (i == positions.size() - 1) {
            break;
        }

        // se houver colisao
        if (get<1>(positions[i]) >= get<1>(positions[i+1])) { 

            // se o carro anterior nao colidiu, marca o risco de colisao para ambos
            if (previous_collision == false){
                collision_risk.push_back(make_tuple(get<0>(positions[i]), 1));
                collision_risk.push_back(make_tuple(get<0>(positions[i+1]), 1));
            } else { // se o carro anterior colidiu, marca o risco de colisao apenas para o proximo
                collision_risk.push_back(make_tuple(get<0>(positions[i+1]), 1));
            }
            previous_collision = true; // atualiza o booleano para marcar que houve colisao

        } else { // se nao houver colisao

            // se o carro anterior nao colidiu, marca o risco de colisao para o atual
            if (previous_collision == false) {
                collision_risk.push_back(make_tuple(get<0>(positions[i]), 0));    
            }
            // caso contrario, nao marca o risco de colisao para nenhum
            
            // atualiza o booleano para marcar que nao houve colisao
            previous_collision = false;
        }
    }


    return collision_risk;
}

vector<int> speed_limits(vector<string> folders) {
    vector<int> speed_limits;
    for (int i = 0; i < folders.size(); i++) {
        size_t pos = folders[i].find("_");
        string s = folders[i].substr(pos+1);
        int speed = stoi(s);
        speed_limits.push_back(speed);
    }
    return speed_limits;
}

vector<vector<bool>> cars_above_limit(vector<int> limits, Road matrix_speeds) {
    vector<thread> t_vec;
    vector<vector<bool>> answer;
    int size = matrix_speeds.size();
    for (int i = 0; i < size; i++) {
        matrix_speeds[i];
        thread t = thread(is_above_limit, matrix_speeds[i], limits[i]);
        t_vec.push_back(t);
    }
    for (int i = 0; i < size; i++) {
        t_vec[i].join();
    }

    return answer;
}

vector<bool> is_above_limit(Lane* lane_to_calc,int limit, vector<bool>* answer) {
    int size = lane_to_calc->size();
    vector<bool> answ;
    for (int i = 0; i < size; i++) {
        bool is_above = (get<1>(lane_to_calc->at(i)) > limit); 
        answ.push_back(is_above);
    }
    answer = &answ;
}





int main() {
    vector<Road> historyPositionsData;
    vector<Road> historySpeedsData;

    //PRIMEIRA LEVA DE DADOS
    
    vector<int> cicles_per_road;
    vector<string> folders;
    vector<Road> Simulation;
    while (true) {
        string fileName = "./data"; //PEGAR O ARQUIVO MAIS RECENTE
        int roadId = 0; //(0:BR101; 1:RS101, 2:RS101);
        Road old_positions = historyPositionsData[roadId];
        Road old_speeds = historySpeedsData[roadId];
        
        get_roads();
        Simulation.clear();
        for (int i = 0; i < folders.size(); i++) {
            Road road_i = splitData(fileName);
            Simulation.push_back(road_i);
            

        }
        

        for (int lane=0; lane < matrix.size(); lane++) {
            Lane vector_speed = calc_speed(matrix[lane], &old_positions[lane]);
            Lane vector_accel = calc_accel(vector_speed, &(old_speeds[lane]));
            Lane vector_collision_risk = calc_collision_risk(matrix[lane], vector_speed, vector_accel);

        }
        
        
        //Lane vector_speed = calc_speed(positions, old_positions);
        //Lane vector_accel = calc_accel(vector_speed, old_speeds);
        //Lane vector_collision_risk = calc_collision_risk(positions, vector_speed, vector_accel);

        //Função que pega quantidade de rodovias: getRoads.size()
        //Função que calcula velocidade, aceleração e risco de colisão pra cada veículo: calc_all()
        //join()
        //Função que verifica número de veículo: .size()
        //Função que verifica quais veículos estão acima da velocidade
        //Função que pega informações externas: getInfo()
        

    }

}