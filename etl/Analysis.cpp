#include "Analysis.hpp"

using namespace std;

// vetor de tuplas (placa, posicao/velocidade/aceleracao) para cada pista
typedef vector<tuple<string,int>> VectorLane; 
// matriz de tuplas (placa, posicao/velocidade/aceleracao) para cada pista
typedef vector<vector<tuple<string,int>>> MatrixRoads;

// calcula a velocidade de cada carro dada a posicao atual e a do ciclo anterior
// as posicoes anteriores sao passadas por referencia para que possam ser atualizadas
VectorLane calc_speed(VectorLane positions, VectorLane* old_position) {
    VectorLane calculated_speed;

    int positions_size = positions.size();
    // para cada posicao atual
    for(int i =0; i< positions.size(); i++) {
        // procura a posicao anterior do mesmo carro (pela placa)
        for (int j = 0; j < old_position->size(); j++) {
            if (get<0>(positions[i]) == get<0>(old_position->at(j))) {
                // calcula a velocidade (velocidade atual - antiga)
                int speed = get<1>(positions[i]) - get<1>(old_position->at(j));
                calculated_speed.push_back(make_tuple(get<0>(positions[i]), speed));
            }
        }
    }

    // atualiza as posicoes anteriores
    old_position = &positions;
    return calculated_speed;
}

// calcula a aceleracao de cada carro dada a velocidade atual e a do ciclo anterior
VectorLane calc_accel(VectorLane speed, VectorLane* old_speeds) {
    VectorLane calculated_accel;

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
VectorLane calc_collision_risk(VectorLane positions, VectorLane speed, VectorLane accel) {
    VectorLane collision_risk;

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

// calcula a velocidade, aceleracao e risco de colisao de cada carro
MatrixRoads calc_all(VectorLane positions, VectorLane* old_positions, VectorLane* old_speeds) {
    MatrixRoads calc_vector;

    VectorLane vector_speed = calc_speed(positions, old_positions);
    VectorLane vector_accel = calc_accel(vector_speed, old_speeds);
    VectorLane vector_collision_risk = calc_collision_risk(positions, vector_speed, vector_accel);

    // calc_vector.push_back(vector_speed);
    // calc_vector.push_back(vector_accel);
}


// funcao para inicializar a posicao e velocidade dos carros
// (no primeiro ciclo, nao ha posicao anterior)
void init_pos_speed(VectorLane pos, VectorLane* old_pos, VectorLane* old_speed) {
    VectorLane speed = calc_speed(pos, old_pos);
    old_speed = &speed;
}


// function to calculate how many cars there are in a road
int calc_num_cars(VectorLane positions) {
    return positions.size();
}



int main()
{
    VectorLane pos;
    VectorLane old_pos;
    VectorLane very_old_pos;
    pos.push_back(make_tuple("A", 30));
    pos.push_back(make_tuple("B", 20));
    pos.push_back(make_tuple("C", 32));
    pos.push_back(make_tuple("D", 40));
    pos.push_back(make_tuple("E", 21));
    

    old_pos.push_back(make_tuple("A", 20));
    old_pos.push_back(make_tuple("B", 10));
    old_pos.push_back(make_tuple("C", 5));
    old_pos.push_back(make_tuple("D", 15));
    old_pos.push_back(make_tuple("E", 19));

    very_old_pos.push_back(make_tuple("A", 14));
    very_old_pos.push_back(make_tuple("B", 2));
    very_old_pos.push_back(make_tuple("C", 1));
    very_old_pos.push_back(make_tuple("D", 0));
    very_old_pos.push_back(make_tuple("E", 18));

    VectorLane old_speed = calc_speed(old_pos, &very_old_pos);
    VectorLane speed = calc_speed(pos, &old_pos);
    VectorLane accel = calc_accel(speed, &old_speed);
    VectorLane col = calc_collision_risk(pos, speed, accel);

    cout << "Pos: " << endl;
    for (int i = 0; i < pos.size(); i++) {
        cout << get<0>(pos[i]) << " " << get<1>(pos[i]) << endl;
    }

    cout << "Speed: " << endl;
    for (int i = 0; i < speed.size(); i++) {
        cout << get<0>(speed[i]) << " " << get<1>(speed[i]) << endl;
    }

    cout << "Accel: " << endl;
    for (int i = 0; i < accel.size(); i++) {
        cout << get<0>(accel[i]) << " " << get<1>(accel[i]) << endl;
    }

    cout << "Collision risk" << endl;
    for (int i = 0; i < col.size(); i++) {
        cout << get<0>(col[i]) << " " << get<1>(col[i]) << endl;
    }

    return 0;
}
