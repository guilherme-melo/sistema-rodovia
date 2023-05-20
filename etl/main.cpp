#include "functions.cpp"

int main() //thread calculations
{
    // Inicializando historyVectors antes de começar o while
    vector<Road> historyPositionsData;
    vector<Road> historySpeedsData;
    vector<string> roads = get_roads();
    int old_roads_size = roads.size();
    for (int roadId = 0; roadId < roads.size(); roadId++) {
        Road temp;
        historyPositionsData.push_back(temp);
        historySpeedsData.push_back(temp);

    }

    int capacity = 40; // Tamanho da fila do barbeiro
    Legado legado(capacity);
    while (true)
    {
    // for (int d = 0; d < 30; d++)
    // {
        //system("clear");
        vector<string> roads_new = get_roads();
        vector<int> speed_limit_list;
        speed_limits(roads_new, &speed_limit_list);
        if (roads_new.size() > old_roads_size){
            for (int i = 0; i < roads_new.size() - roads.size(); i++){
                Road temp;
                historyPositionsData.push_back(temp);
                historySpeedsData.push_back(temp);
            }
            old_roads_size = roads_new.size();
        }
        cout << "-----------HEADER-----------" << endl;
        vector<Road> positions_list;
        vector<Road> speeds_list;
        vector<Road> accelerations_list;
        vector<Road> collision_risk_list;

        // Pegando os dados mais recentes
        int iter = 0;
        int posicaoInicial;
        int time;
        string file;
        for (int roadId = 0; roadId < roads_new.size(); roadId++) {
            file = getMostRecentFile(roads_new[roadId],ref(iter));
            if (file == "") {
                cout << "No file found" << endl;
                continue;
            }

            string cars = extractCarsValue(file);
            if (cars == "") {
                cout << "No car data found" << endl;
                continue;
            }

            time = stoi(extractTime(file));

            Road positions = splitData(cars);

            deleteAllDocuments(roads_new[roadId]);
            positions_list.push_back(positions);


            Road* old_positions = &historyPositionsData[roadId];
            Road* old_speeds = &historySpeedsData[roadId];


            // Iniciando análises -------------------------
            // Aqui teremos velocidade, aceleração, risco de colisão pra cada veiculo
            // Número de veículos
            // Inicializa velocidades

            // Tratamento para os dois primeiros casos onde não temos dados históricos o suficiente
            if (old_positions->size() == 0 ) {
                cout << "Without history data" << endl;

            }
            else if (old_speeds->size() == 0) {

                Road speeds = calc_speed(positions, old_positions,iter); // thread pra calcular pra cada lane -> join
                speeds_list.push_back(speeds);
                cout << "Without history data" << endl;
                historySpeedsData[roadId] = speeds;
            }

            else {
                Road speeds = calc_speed(positions, old_positions,iter);
                speeds_list.push_back(speeds);
                Road accel = calc_accel(speeds, old_speeds); // threads pra calcular pra cada lane -> join
                accelerations_list.push_back(accel);
                Road collision_risk;
                vector<thread> thread_vec;
                mutex mtx_print;
                mutex mtx_risk;
                for (int i = 0; i < positions.size(); i++)
                {
                    Lane c_risk;
                    thread_vec.push_back(thread(calc_collision_risk, &positions[i], &speeds[i], &accel[i], &collision_risk , ref(mtx_print), ref(mtx_risk)));
                    // Pela prioridade, essas informacoes devem ser printadas no console;
                    //collision_risk.push_back(c_risk);
                }

                for (int i = 0; i < thread_vec.size(); i++)
                {
                    thread_vec[i].join();
                }

                collision_risk_list.push_back(collision_risk);
                historySpeedsData[roadId] = speeds;
            }

            //Atualizar historyData ao fim do loop
            historyPositionsData[roadId] = positions;
        }

        //Inicialização do vetor que vai receber o resultado do barbeiro
        vector<vector<string>> cars_data;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                for (int k = 0; k < positions_list[i][j].size(); k++) {
                    vector<string> car_data;
                    // Iniciar 3 strings que possivelmente serão usadas
                    cars_data.push_back(car_data);
                }
            }
        }


        // Análise do barbeiro (em threads)
        vector<thread> t_vec;
        int recount = 0;
        mutex m;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                for (int k = 0; k < positions_list[i][j].size(); k++) {
                    string plate = get<0>(positions_list[i][j][k]);
                    t_vec.push_back(thread(&Legado::request, ref(legado), plate, ref(cars_data), recount, ref(m)));
                    recount++;
                }
            }
        }

        //Análise do número de carros
        int n_carros_simulacao = 0;
        for (int i = 0; i < positions_list.size(); i++) {
            for (int j = 0; j < positions_list[i].size(); j++) {
                n_carros_simulacao += positions_list[i][j].size();
            }

            if (historyPositionsData[i].size() != 0) {
                if (historySpeedsData[i].size() != 0) {
                    //Análise de carros acima da velocidade
                    vector<vector<tuple<string,bool>>> numbers_of_car = cars_above_limit(speed_limit_list[i], speeds_list[i]);


                    cout << "------------------" << endl;
                    for (int i = 0; i < numbers_of_car.size(); i++) {
                        for (int j = 0; j < numbers_of_car[i].size(); j++) {
                            if (get<1>(numbers_of_car[i][j]) == 1) {
                                cout << "Placa: " << get<0>(numbers_of_car[i][j]) << endl;
                                cout << "Velocidade acima do limite." << endl;
                                cout << "---------------" << endl;
                            }
                        }
                    }
                }
            }
        }

        cout <<"Número de carros na simulação: " << n_carros_simulacao << endl;
        int n_road = roads_new.size();
        cout << "Número de rodovias presentes na simulação: " << n_road << endl;

        //Joins das threads do barbeiro
        for (int i = 0; i < t_vec.size() ; i++) {
            t_vec[i].join();
        }



        // Barbeiro
        //vector<vector<string>> cars_data;
        //prepare_to_barber(&positions_list, &legado, &cars_data);

        //DASHBOARD
        cout << "------------------" << endl;
        int count_cars = 0;

        for (int road = 0; road < positions_list.size(); road++) {
            for (int lane = 0; lane < positions_list[road].size(); lane++) {
                for (int car = 0; car < positions_list[road][lane].size(); car++) {

                    cout << "---------------" << endl;
                    cout << "Placa: " << get<0>(positions_list[road][lane][car]) << endl;
                    cout << "Posição: " << get<1>(positions_list[road][lane][car]) << " , " << lane << endl;

                    bool speed_found = false;
                    for (int road_speed = 0; road_speed < speeds_list.size(); road_speed++) {
                        for (int lane_speed = 0; lane_speed < speeds_list[road_speed].size(); lane_speed++) {
                            for (int car_speed = 0; car_speed < speeds_list[road_speed][lane_speed].size(); car_speed++) {
                                if (get<0>(speeds_list[road_speed][lane_speed][car_speed]) == get<0>(positions_list[road][lane][car])) {
                                    cout << "Velocidade: " << get<1>(speeds_list[road_speed][lane_speed][car_speed]) << endl;
                                    speed_found = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!speed_found) {
                        cout << "Velocidade: Sem dados" << endl;
                    }

                    bool acceleration_found = false;
                    for (int road_acceleration = 0; road_acceleration < accelerations_list.size(); road_acceleration++) {
                        for (int lane_acceleration = 0; lane_acceleration < accelerations_list[road_acceleration].size(); lane_acceleration++) {
                            for (int car_acceleration = 0; car_acceleration < accelerations_list[road_acceleration][lane_acceleration].size(); car_acceleration++) {
                                if (get<0>(accelerations_list[road_acceleration][lane_acceleration][car_acceleration]) == get<0>(positions_list[road][lane][car])) {
                                    cout << "Aceleração: " << get<1>(accelerations_list[road_acceleration][lane_acceleration][car_acceleration]) << endl;
                                    acceleration_found = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!acceleration_found) {
                        cout << "Aceleração: Sem dados" << endl;
                    }

                    bool collision_risk_found = false;
                    for (int road_collision = 0; road_collision < collision_risk_list.size(); road_collision++) {
                        for (int lane_collision = 0; lane_collision < collision_risk_list[road_collision].size(); lane_collision++) {
                            for (int car_collision = 0; car_collision < collision_risk_list[road_collision][lane_collision].size(); car_collision++) {
                                if (get<0>(collision_risk_list[road_collision][lane_collision][car_collision]) == get<0>(positions_list[road][lane][car])) {
                                    cout << "Risco de colisão: " << get<1>(collision_risk_list[road_collision][lane_collision][car_collision]) << endl;
                                    collision_risk_found = true;
                                    break;
                                }
                            }
                        }
                }
                if (!collision_risk_found) {
                    cout << "Risco de colisão: Sem dados" << endl;
                }


                    cout << "Modelo: " << cars_data[count_cars][0] << endl;
                    cout << "Ano: " << cars_data[count_cars][1] << endl;
                    cout << "Proprietário: " << cars_data[count_cars][2] << endl;
                    count_cars++;
                    cout << "---------------" << endl;
                }
            }

            chrono::milliseconds ms = chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch() );
            //removes the 5 first digits of ms
            long long mil_sec = ms.count();
            //cout << "Tempo de análise: " << mil_sec << endl;
            mil_sec = mil_sec % 1000000000;
            cout << "Tempo de análise: " << mil_sec - time << "ms" << endl;
            //cout << "Tempo de análise: " << mil_sec << endl;
            // if (fileName == "") {
            //     cout << "Tempo de análise: não há arquivo de entrada" << endl;
            // }
            // else {
            //     int latestTime = stoi(fileName.substr(posicaoInicial +1, posicaoInicial + 8));
            //     cout << "Tempo de análise: " << mil_sec - latestTime << "ms" << endl;
            // }
        }
    }
    return 0;
}