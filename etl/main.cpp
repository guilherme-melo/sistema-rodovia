#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <thread>
#include <mutex>
#include "Legado.cpp"

using namespace std;

typedef vector<tuple<string, int>> Lane;
typedef vector<Lane> Road;

// PREPARE DATA----------------------------------------------------------------------------------
Road splitData(string fileName)
{
    vector<string> plates;
    vector<int> x;
    vector<int> y;
    Road por_pista;
    string line;
    Lane *pista_anterior_ptr = nullptr;

    // Separa os dados dos arquivos em um vetor que contém uma pista em cada espaço
    ifstream file(fileName);
    while (getline(file, line))
    {

        // Le as strings e
        char delimiter = '(';
        size_t pos_inicio = line.find(delimiter);
        size_t pos_fim = line.find(',', pos_inicio);

        x.push_back(stoi(line.substr(pos_inicio + 1, pos_fim - pos_inicio - 1)));
        y.push_back(stoi(line.substr(pos_fim + 1, 1)));

        plates.push_back(line.substr(0, 7));
        line.clear();
    }

    file.close();

    auto N_PISTAS = max_element(y.begin(), y.end());
    int n_pistas = *N_PISTAS + 1;

    int plates_size = plates.size();
    for (int pista_atual = 0; pista_atual < n_pistas; pista_atual++)
    {
        Lane pista;

        // insert plates and x values into pista
        for (int carro_atual = 0; carro_atual < plates_size; carro_atual++)
        {
            if (y[carro_atual] == pista_atual)
            {
                pista.push_back(make_tuple(plates[carro_atual], x[carro_atual]));
            }
        }
        por_pista.push_back(pista);
    }
    return por_pista;
}

// THREAD 1 ( RODOVIAS) ----------------------------------------------------------------------------------
vector<string> get_roads()
{
    const char *folder_path = "./data/";
    vector<string> folders;
    DIR *dirp = opendir(folder_path);

    struct dirent *entry;

    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            folders.push_back(entry->d_name);
        }
    }

    closedir(dirp);

    return folders;
}

// THREAD 2 (CALCULA VELOCIDADE, ACELERACAO, RISCO DE COLISAO) ----------------------------------------------------------------------------------
//
using namespace std;

// vetor de tuplas (placa, posicao/velocidade/aceleracao) para cada pista
typedef vector<tuple<string, int>> Lane;

void calc_speed_thread(Road positions, Road *old_position, Road *calculated_speed, int i, int j, mutex &mtx){
    // procura a posicao anterior do mesmo carro (pela placa)
    for (int y_old = 0; y_old < old_position->size(); y_old++)
    {
        for (int x_old = 0; x_old < old_position->at(y_old).size(); x_old++)
        {
            if (get<0>(positions[i][j]) == get<0>(old_position->at(y_old)[x_old]))
            {
                int speed = get<1>(positions[i][j]) - get<1>(old_position->at(y_old)[x_old]);
                tuple<string,int> plate_speed = make_tuple(get<0>(positions[i][j]), speed);
                mtx.lock();
                calculated_speed->at(i).push_back(plate_speed);
                mtx.unlock();
            }
        }
    }
}

// calcula a velocidade de cada carro dada a posicao atual e a do ciclo anterior
// as posicoes anteriores sao passadas por referencia para que possam ser atualizadas
Road calc_speed(Road positions, Road* old_position){
    Road calculated_speed;
    vector<thread> threads;
    mutex mtx;
    for (int i = 0; i < positions.size(); i++)
    {
        Lane lane;
        calculated_speed.push_back(lane);
    }
    int count = 0;
    int positions_size = positions[0].size();
    // para cada posicao atual
    for (int y = 0; y < positions.size(); y++)
    {
        for (int x = 0; x < positions[y].size(); x++)
        {
            count++;
            threads.push_back(thread(calc_speed_thread, positions, old_position, &calculated_speed, y, x, ref(mtx)));
            // procura a posicao anterior do mesmo carro (pela placa)

        }
    }

    for (int y = 0; y < count; y++)
    {
        threads[y].join();
    }

    // atualiza as posicoes anteriores
    Road old_position_var = positions;
    old_position = &old_position_var;
    return calculated_speed;
}


void calc_accel_thread(Road speed, Road *old_speeds, Road *calculated_accel, int i, int j, mutex &mtx){
    // procura a velocidade anterior do mesmo carro (pela placa)
    for (int y_old = 0; y_old < old_speeds->size(); y_old++)
    {
        for (int x_old = 0; x_old < old_speeds->at(y_old).size(); x_old++)
        {
            if (get<0>(speed[i][j]) == get<0>(old_speeds->at(y_old)[x_old]))
            {
                // calcula a aceleracao (aceleracao atual - antiga)
                int accel = get<1>(speed[i][j]) - get<1>(old_speeds->at(y_old)[x_old]);
                tuple<string,int> plate_accel = make_tuple(get<0>(speed[i][j]), accel);
                mtx.lock();
                calculated_accel->at(i).push_back(plate_accel);
                mtx.unlock();
            }
        }
    }
}
// calcula a aceleracao de cada carro dada a velocidade atual e a do ciclo anterior
Road calc_accel(Road speed, Road *old_speeds)
{
    vector<thread> threads;
    int count = 0;
    mutex mtx;
    Road calculated_accel;
    for (int i = 0; i < speed.size(); i++)
    {
        Lane lane;
        calculated_accel.push_back(lane);
    }

    // para cada velocidade atual
    for (int y = 0; y < speed.size(); y++)
    {
        for (int x = 0; x < speed[y].size(); x++)
        {
            // procura a velocidade anterior do mesmo carro (pela placa)
            threads.push_back(thread(calc_accel_thread, speed, old_speeds, &calculated_accel, y, x, ref(mtx)));
            count++;
        }
    }
    for (int y = 0; y < count; y++)
    {
        threads[y].join();
    }

    return calculated_accel;
}

// calcula o risco de colisão de cada carro dada a posicao, velocidade e aceleracao calculadas
Lane calc_collision_risk(Lane positions, Lane speed, Lane accel) {
    Lane collision_risk;

    // ordena os carros por posicao
    sort(positions.begin(), positions.end(), [](const tuple<string, int> &a, const tuple<string, int> &b)
         { return get<1>(a) < get<1>(b); });

    // encontra a velocidade e aceleracao de cada carro
    for (int i = 0; i < positions.size(); i++)
    {
        for (int j = 0; j < speed.size(); j++)
        {
            for (int k = 0; k < accel.size(); k++)
            {
                if (get<0>(positions[i]) == get<0>(speed[j]) && get<0>(positions[i]) == get<0>(accel[k]))
                {
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
    for (int i = 0; i < positions.size(); i++)
    {
        if (i == positions.size() - 1)
        {
            break;
        }

        // se houver colisao
        if (get<1>(positions[i]) >= get<1>(positions[i + 1]))
        {

            // se o carro anterior nao colidiu, marca o risco de colisao para ambos
            if (previous_collision == false)
            {
                collision_risk.push_back(make_tuple(get<0>(positions[i]), 1));
                collision_risk.push_back(make_tuple(get<0>(positions[i + 1]), 1));
            }
            else
            { // se o carro anterior colidiu, marca o risco de colisao apenas para o proximo
                collision_risk.push_back(make_tuple(get<0>(positions[i + 1]), 1));
            }
            previous_collision = true; // atualiza o booleano para marcar que houve colisao
        }
        else
        { // se nao houver colisao

            // se o carro anterior nao colidiu, marca o risco de colisao para o atual
            if (previous_collision == false)
            {
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


void is_above_limit(Lane* lane_to_calc,int limit, vector<vector<tuple<string,bool>>>* answer, mutex& mtx) {
    int size = lane_to_calc->size();
    vector<tuple<string,bool>> answ;
    for (int i = 0; i < size; i++) {
        bool is_above = (get<1>(lane_to_calc->at(i)) > limit);
        tuple<string,bool> car = make_tuple(get<0>(lane_to_calc->at(i)), is_above);
        answ.push_back(car);
    }
    mtx.lock();
    answer->push_back(answ);
    mtx.unlock();

}

vector<vector<tuple<string,bool>>> cars_above_limit(int limit, Road matrix_speeds) {
    vector<thread> t_vec;
    mutex mtx;
    int count = 0;
    vector<vector<tuple<string,bool>>> answer;
    int size = matrix_speeds.size();
    for (int i = 0; i < size; i++) {
        t_vec.push_back(thread(is_above_limit, &matrix_speeds[i], limit, &answer, ref(mtx)));
        count++;
    }
    for (int i = 0; i < count; i++) {
        t_vec[i].join();
    }
    return answer;
}

int main() //thread calculations
{
    //Initializing historyVectors before starting of while
    vector<Road> historyPositionsData;
    vector<Road> historySpeedsData;
    vector<string> roads = get_roads();
    for (int roadId = 0; roadId < roads.size()-2; roadId++) {
        Road temp;
        historyPositionsData.push_back(temp);
        historySpeedsData.push_back(temp);

    }
    int capacity = 10;
    Legado legado(capacity);
    // while (true)
    // {
    for (int d = 0; d < 4; d++)
    {
        //system("clear");
        cout << "ATUALIZAÇÃO DASHBOARD" << endl;
        vector<Road> positions_list;
        vector<Road> speeds_list;
        vector<Road> accelerations_list;
        vector<Road> collision_risk_list;
        for (int roadId = 0; roadId < roads.size()-2; roadId++) {
            int file_number = 15 + d;
            string fileName = "./data/" + roads[roadId+2] + "/" + to_string(file_number) + ".txt";// PEGAR O ARQUIVO MAIS RECENTE

            Road positions = splitData(fileName);
            positions_list.push_back(positions);

            cout << roads[roadId+2] << endl;
            cout << positions.size() << endl;
            Road* old_positions = &historyPositionsData[roadId];
            Road* old_speeds = &historySpeedsData[roadId];

            // Iniciando análises -------------------------
            // Aqui teremos velocidade, aceleração, risco de colisão pra cada veiculo
            // Número de veículos
            //initializes speeds

            //Treatment for the two first cases where we don't have enough historyData
            if (old_positions->size() == 0 ) {
                cout << "Without history data" << endl;

            }
            else if (old_speeds->size() == 0) {
                cout << "Without history adsada" << endl;
                Road speeds = calc_speed(positions, old_positions); //1 thread pra calcular pra cada lane -> join
                speeds_list.push_back(speeds);
                cout << "Without history data" << endl;
                historySpeedsData[roadId] = speeds;
            }

            else {
                Road speeds = calc_speed(positions, old_positions);
                speeds_list.push_back(speeds);
                Road accel = calc_accel(speeds, old_speeds); //1 threads pra calcular pra cada lane -> join
                accelerations_list.push_back(accel);
                Road collision_risk;

                for (int i = 0; i < positions.size(); i++)
                {
                    Lane c_risk = calc_collision_risk(positions[i], speeds[i], accel[i]);
                    //AQUI ESSAS INFORMAÇÕES JÁ DEVEM SER PRINTADAS NO CONSOLE;
                    collision_risk.push_back(c_risk);
                }
                collision_risk_list.push_back(collision_risk);

                cout << "Veículos com risco de colisão" << endl;
                for (Lane lane : collision_risk) {
                    for (tuple<string, int> tupla : lane) {
                        if (get<1>(tupla) == 1) {
                            cout << "---------------" << endl;
                            cout << "Placa: " << get<0>(tupla) << endl;
                            cout << "Risco de colisão" << endl;
                            cout << "---------------" << endl;
                        }
                    }
                }
                historySpeedsData[roadId] = speeds;
            }

            //Atualizar historyData on the final of the loop
            historyPositionsData[roadId] = positions;
        }

        int n_carros_simulacao = 0;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                n_carros_simulacao += positions_list[i][j].size();
            }
            vector<vector<tuple<string,bool>>> numbers_of_car = cars_above_limit(100, positions_list[i]);
            cout << "------------------" << endl;
            for (int i = 0; i < numbers_of_car.size(); i++) {
                for (int j = 0; j < numbers_of_car[i].size(); j++) {
                    cout << "Placa: " << get<0>(numbers_of_car[i][j]) << endl;
                    cout << "Velocidade acima do limite: " << get<1>(numbers_of_car[i][j]) << endl;
                    cout << "---------------" << endl;
                }
            }
        }

        cout <<"Números de carros na simulação: " << n_carros_simulacao << endl;
        int n_road = roads.size() - 2;
        cout << "Número de rodovias presentes na simulação: " << n_road << endl;

        vector<vector<string>> cars_data;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                for (int k = 0; k < positions_list[i][j].size(); k++) {
                    vector<string> car_data;
                    //iniciar 3 strings possivelmente
                    cars_data.push_back(car_data);
                }
            }
        }

        vector<thread> t_vec;
        int recount = 0;
        mutex m;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                for (int k = 0; k < positions_list[i][j].size(); k++) {
                    string plate = get<0>(positions_list[i][j][k]);
                    t_vec.push_back(thread(&Legado::request, &legado, plate, ref(cars_data), recount, ref(m)));
                    recount++;
                }
            }
        }
        cout << recount << endl;
        for (int i = 0; i < t_vec.size() ; i++) {
            cout << "Thread " << i  ;
            t_vec[i].join();
            cout << "joined" << endl;
        }
        cout << "------------------" << endl;
        int count_cars = 0;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                for (int k = 0; k < positions_list[i][j].size(); k++) {
                    cout << "---------------" << endl;
                    cout << "Placa: " << get<0>(positions_list[i][j][k]) << endl;
                    cout << "Posição: " << get<1>(positions_list[i][j][k]) << endl;
                    cout << "Velocidade: " << get<1>(speeds_list[i][j][k]) << endl;
                    cout << "Aceleração: " << get<1>(accelerations_list[i][j][k]) << endl;
                    cout << "Risco de colisão: " << get<1>(collision_risk_list[i][j][k]) << endl;
                    cout << "Modelo: " << cars_data[count_cars][0] << endl;
                    cout << "Ano: " << cars_data[count_cars][1] << endl;
                    cout << "Proprietário: " << cars_data[count_cars][2] << endl;
                    count_cars++;
                    cout << "---------------" << endl;
                }
            }
        }

    }
    return 0;
}