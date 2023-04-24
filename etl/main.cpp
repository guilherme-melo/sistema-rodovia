#include <fstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <thread>
#include <mutex>
#include <cstdlib>
#include "Legado.cpp"
#include <cstdio>
#include <ctime>
#include <sys/stat.h>
#include <chrono>



using namespace std;

typedef vector<tuple<string, int>> Lane;
typedef vector<Lane> Road;

// PREPARE DATA----------------------------------------------------------------------------------


// Funcao que separa os dados dos arquivos em um vetor que contém uma pista em cada espaço
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

        // Separa em substrings e insere nos vetores correspondentes
        char delimiter = '(';
        size_t pos_inicio = line.find(delimiter);
        size_t pos_fim = line.find(',', pos_inicio);

        x.push_back(stoi(line.substr(pos_inicio + 1, pos_fim - pos_inicio - 1)));
        y.push_back(stoi(line.substr(pos_fim + 1, 1)));

        plates.push_back(line.substr(0,5));
        line.clear();
    }

    file.close();

    // Conta o numero de pistas
    auto N_PISTAS = max_element(y.begin(), y.end());
    int n_pistas = *N_PISTAS + 1;

    // Separa os dados por pista e insere em um vetor de pistas
    int plates_size = plates.size();
    for (int pista_atual = 0; pista_atual < n_pistas; pista_atual++)
    {
        Lane pista;

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

// Funcao que retorna o nome de todas as pastas (rodovias)
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


// Funcao de calculo de velocidade em uma thread
void calc_speed_thread(Road positions, Road *old_position, Road *calculated_speed, int i, int j, mutex &mtx){
    // Procura a posicao anterior do mesmo carro (pela placa)
    for (int y_old = 0; y_old < old_position->size(); y_old++)
    {
        for (int x_old = 0; x_old < old_position->at(y_old).size(); x_old++)
        {
            if (get<0>(positions[i][j]) == get<0>(old_position->at(y_old)[x_old]))
            {
                // Calcula a velocidade atual (posicao atual - posicao anterior)
                int speed = get<1>(positions[i][j]) - get<1>(old_position->at(y_old)[x_old]);
                speed = abs(speed);
                // Cria uma tupla (placa, velocidade) e insere no vetor de velocidades calculadas
                tuple<string,int> plate_speed = make_tuple(get<0>(positions[i][j]), speed);

                // Insere a tupla no vetor de velocidades calculadas protegendo por mutex
                mtx.lock();
                calculated_speed->at(i).push_back(plate_speed);
                mtx.unlock();
            }
        }
    }
}


// Calcula a velocidade de cada carro dada a posicao atual e a do ciclo anterior
// As posicoes anteriores sao passadas por referencia para que possam ser atualizadas
Road calc_speed(Road positions, Road* old_position){
    Road calculated_speed;
    vector<thread> threads;
    mutex mtx;
    int count = 0;

    // Inicializa o vetor de velocidades calculadas com o tamanho do vetor de posicoes
    for (int i = 0; i < positions.size(); i++)
    {
        Lane lane;
        calculated_speed.push_back(lane);
    }

    // Para cada pista
    for (int y = 0; y < positions.size(); y++)
    {
        // Para cada carro, cria uma thread para calcular a velocidade usando a funcao calc_speed_thread
        for (int x = 0; x < positions[y].size(); x++)
        {
            count++; // Contamos o numero de threads (carros)
            threads.push_back(thread(calc_speed_thread, positions, old_position, &calculated_speed, y, x, ref(mtx)));
        }
    }

    // Join das threads
    for (int y = 0; y < count; y++)
    {
        threads[y].join();
    }

    // Atualiza as posicoes anteriores
    Road old_position_var = positions;
    old_position = &old_position_var;
    return calculated_speed;
}


// Funcao de calculo de aceleração em uma thread
void calc_accel_thread(Road speed, Road *old_speeds, Road *calculated_accel, int i, int j, mutex &mtx){
    // Procura a velocidade anterior do mesmo carro (pela placa)
    for (int y_old = 0; y_old < old_speeds->size(); y_old++)
    {
        for (int x_old = 0; x_old < old_speeds->at(y_old).size(); x_old++)
        {
            if (get<0>(speed[i][j]) == get<0>(old_speeds->at(y_old)[x_old]))
            {
                // Calcula a aceleracao (aceleracao atual - antiga)
                int accel = get<1>(speed[i][j]) - get<1>(old_speeds->at(y_old)[x_old]);
                // Cria uma tupla (placa, aceleracao)
                tuple<string,int> plate_accel = make_tuple(get<0>(speed[i][j]), accel);

                // Adiciona a tupla ao vetor de aceleracoes calculadas, protegido por mutex
                mtx.lock();
                calculated_accel->at(i).push_back(plate_accel);
                mtx.unlock();
            }
        }
    }
}

// Calcula a aceleracao de cada carro dada a velocidade atual e a do ciclo anterior
// As velocidades anteriores sao passadas por referencia para que possam ser atualizadas
Road calc_accel(Road speed, Road *old_speeds) {
    Road calculated_accel;
    vector<thread> threads;
    mutex mtx;
    int count = 0;

    // Inicializa o vetor de aceleracoes calculadas com o tamanho do vetor de velocidades
    for (int i = 0; i < speed.size(); i++)
    {
        Lane lane;
        calculated_accel.push_back(lane);
    }

    // Para cada velocidade atual
    for (int y = 0; y < speed.size(); y++)
    {
        // Para cada carro, cria uma thread para calcular a aceleracao usando a funcao calc_accel_thread
        for (int x = 0; x < speed[y].size(); x++)
        {
            // Contamos o numero de threads (carros)
            count++;
            threads.push_back(thread(calc_accel_thread, speed, old_speeds, &calculated_accel, y, x, ref(mtx)));
        }
    }

    // Join das threads
    for (int y = 0; y < count; y++)
    {
        threads[y].join();
    }

    return calculated_accel;
}

// Calcula o risco de colisão de cada carro dada a posicao, velocidade e aceleracao calculadas
// para uma pista
void calc_collision_risk(Lane* positions, Lane* speed, Lane* accel, Road* c_risk, mutex &mtx_print, mutex &mtx_risk){
    Lane collision_risk;
    Lane positions_n = *positions;

    // Ordena os carros por posicao
    sort(positions->begin(), positions->end(), [](const tuple<string, int> &a, const tuple<string, int> &b)
         { return get<1>(a) < get<1>(b); });

    // Encontra a velocidade e aceleracao de cada carro
    for (int i = 0; i < positions->size(); i++)
    {
        for (int j = 0; j < speed->size(); j++)
        {
            for (int k = 0; k < accel->size(); k++)
            {
                if (get<0>(positions->at(i)) == get<0>(speed->at(j)) && get<0>(positions->at(i)) == get<0>(accel->at(k)))
                {
                    // Calcula a nova posicao do carro, mas escreve mantendo a ordem original (da posicao antiga)
                    int new_pos = get<1>(positions->at(i)) + get<1>(speed->at(j)) + get<1>(accel->at(k));
                    positions_n[i] = make_tuple(get<0>(positions->at(i)), new_pos);
                }
            }
        }
    }


    // Checa se algum carro vai colidir com o carro a frente, e marca o risco de colisao
    // Utilizamos 1 caso haja risco de colisao, 0 caso contrario
    bool previous_collision = false; // Booleano para marcar se o carro anterior ja colidiu
    for (int i = 0; i < positions->size(); i++)
    {
        if (i == positions->size() - 1)
        {
            break;
        }

        // Se houver colisao
        if (get<1>(positions_n[i]) >= get<1>(positions_n[i + 1]))
        {
            // Se o carro anterior nao colidiu, marca o risco de colisao para ambos
            if (previous_collision == false)
            {
                tuple<string,bool> risk_1 = make_tuple(get<0>(positions_n[i]), 1);
                tuple<string, bool> risk_2 = make_tuple(get<0>(positions_n[i + 1]), 1);
                collision_risk.push_back(risk_1);
                collision_risk.push_back(risk_2);

                // Mutex para proteger os outs no console
                mtx_print.lock();
                cout << get<0>(risk_1) << " --> " << "Possui Risco de Colisão" << endl;
                cout << get<0>(risk_2) << " --> " << "Possui Risco de Colisão" << endl;
                mtx_print.unlock();
            }
            else
            { // Se o carro anterior colidiu, marca o risco de colisao apenas para o proximo
                tuple<string, bool> risk_3 = make_tuple(get<0>(positions_n[i + 1]), 1);
                collision_risk.push_back(risk_3);
                mtx_print.lock();
                cout << get<0>(risk_3) << " --> " << "Possui Risco de Colisão" << endl;
                mtx_print.unlock();
            }
            previous_collision = true; // Atualiza o booleano para marcar que houve colisao
        }
        else
        { // Se nao houver colisao

            // Se o carro anterior nao colidiu, marca o risco de colisao para o atual
            if (previous_collision == false)
            {
                collision_risk.push_back(make_tuple(get<0>(positions_n[i]), 0));
            }
            // Caso contrario, nao marca o risco de colisao para nenhum

            // Atualiza o booleano para marcar que nao houve colisao
            previous_collision = false;
        }
    }

    mtx_risk.lock();
    c_risk->push_back(collision_risk);
    mtx_risk.unlock();
    //return collision_risk;
}


// Funcao que checa o limite de velocidade de cada pista
// e adiciona os limites a um vetor
void speed_limits(vector<string> folders, vector<int> *s_limits) {
    // Passa pelo vetor de pastas e pega o limite de velocidade de cada uma
    for (int i = s_limits->size() + 2 ; i < folders.size(); i++) {
        size_t start = folders[i].find('_');
        cout << folders[i] << endl;
        int speed =  stoi(folders[i].substr(start+1, folders[i].size()));
        s_limits->push_back(speed);
    }
}


// Funcao que checa se cada carro esta acima do limite de velocidade
// pista por pista
void is_above_limit(Lane* lane_to_calc,int limit, vector<vector<tuple<string,bool>>>* answer, mutex& mtx) {
    int size = lane_to_calc->size();
    vector<tuple<string,bool>> answ;

    // Passa pelo vetor da pista checando se cada carro está acima do limite
    // e adiciona o resultado booleano a um vetor
    for (int i = 0; i < size; i++) {
        bool is_above = (get<1>(lane_to_calc->at(i)) > limit);
        tuple<string,bool> car = make_tuple(get<0>(lane_to_calc->at(i)), is_above);
        answ.push_back(car);
    }

    // Insere o vetor de resultados na matriz de resultados protegendo com mutex
    mtx.lock();
    answer->push_back(answ);
    mtx.unlock();

}

// Funcao que recebe a matriz com as velocidades de cada carro na rodovia e o limite de velocidade dela
// e retorna uma matriz com booleanos indicando se cada carro esta acima do limite
vector<vector<tuple<string,bool>>> cars_above_limit(int limit, Road matrix_speeds) {
    vector<vector<tuple<string,bool>>> answer;
    vector<thread> t_vec;
    mutex mtx;
    int count = 0;
    int size = matrix_speeds.size();

    // Passa uma pista para cada thread para o calculo com a funcao is_above_limit
    for (int i = 0; i < size; i++) {
        t_vec.push_back(thread(is_above_limit, &matrix_speeds[i], limit, &answer, ref(mtx)));
        count++;
    }

    // Join das threads
    for (int i = 0; i < count; i++) {
        t_vec[i].join();
    }
    return answer;
}

// Função que busca os dados mais recentes
tuple<string,time_t> getMostRecentData(const string &dirName) {
    DIR *dir;
    struct dirent *ent;
    time_t newest_time = 0;
    std::string newest_file;

    // Abre o diretório
    dir = opendir(dirName.c_str());

    // Itera por todos os arquivos do diretório
    while ((ent = readdir(dir)) != nullptr) {
        std::string filename = ent->d_name;
        std::string filepath = dirName + filename;
        time_t newest_time = 0;
        // Checa se estamos em um .txt
        if (filename != "." && filename != "..") {
            if (filename.substr(filename.size() - 4) == ".txt") {

                // Pega o horario de modificacao do arquivo
                struct stat st;
                stat(filepath.c_str(), &st);
                time_t mod_time = st.st_mtime;

                // Atualiza newest_file e newest_time se esse arquivo for mais novo
                if (mod_time > newest_time) {

                    newest_file = filepath;
                    newest_time = mod_time;
                }
            }
        }
    }

    // Fecha o diretório
    closedir(dir);
    cout << newest_file << endl;
    tuple<string,time_t> answer = make_tuple(newest_file, newest_time);
    return answer;
}
// bool cleanDirectory(const std::string &path){
//     struct dirent *ent;
//     DIR *dir = opendir(path.c_str());
//     if (dir != NULL) {
//         /* remove all the files and directories within directory */
//         while ((ent = readdir(dir)) != NULL) {
//             std::remove((path + ent->d_name).c_str());
//         }
//         closedir (dir);
//     } else {
//         /* could not open directory */
//         return false;
//     }
//     return true;
// }

// string processName()
// {
//     FILETIME bestDate = { 0, 0 };
//     FILETIME curDate;
//     string name;
//     CFileFind finder;

//     finder.FindFile("*.png");
//     while (finder.FindNextFile())
//     {
//         finder.GetCreationTime(&curDate);

//         if (CompareFileTime(&curDate, &bestDate) > 0)
//         {
//             bestDate = curDate;
//             name = finder.GetFileName().GetString();
//         }
//     }
//     return name;
// }

void deleteAllFiles (const string &dirName) {
    DIR *dir;
    struct dirent *ent;

    // Abre o diretorio
    dir = opendir(dirName.c_str());

    // Itera por todos os arquivos no diretorio
    while ((ent = readdir(dir)) != nullptr) {
        std::string filename = ent->d_name;
        std::string filepath = dirName + filename;

        // Ignora . e .. 
        if (filename == "." || filename == "..") {
            continue;
        }

        // Delete the file
        if (remove(filepath.c_str()) != 0) {
            continue;
        } else {
            continue;
        }
    }
}
//     // Close the directory
//     closedir(dir);
// }

string getMostRecentFile(const string& folderPath,int& iter) {
    DIR* dirp = opendir(folderPath.c_str());
    struct dirent * dp;
    time_t latestTime = 0;
    std::string latestFile;
    while ((dp = readdir(dirp)) != NULL) {
        iter++;
        struct stat fileStat;
        std::string filePath = folderPath + "/" + dp->d_name;
        if (stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            if (fileStat.st_mtime > latestTime) {
                time_t l_time = fileStat.st_mtime;
                // queremos reter o último tempo fora do escopo do if para atribui-lo
                latestTime = l_time;
                //cout << "ltime:" << *latestTime   << endl;
                latestFile = filePath;
            }
        }
    }
    closedir(dirp);
    return latestFile;
}

int main() //thread calculations
{
    // Inicializando historyVectors antes de começar o while
    vector<Road> historyPositionsData;
    vector<Road> historySpeedsData;
    vector<string> roads = get_roads();
    int old_roads_size = roads.size();
    for (int roadId = 0; roadId < roads.size()-2; roadId++) {
        Road temp;
        historyPositionsData.push_back(temp);
        historySpeedsData.push_back(temp);

    }

    int capacity = 10; // Tamanho da fila do barbeiro
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
        cout << "ATUALIZAÇÃO DASHBOARD" << endl;
        vector<Road> positions_list;
        vector<Road> speeds_list;
        vector<Road> accelerations_list;
        vector<Road> collision_risk_list;

        // Pegando os dados mais recentes
        int iter = 0;
        int posicaoInicial;
        string fileName;
        for (int roadId = 0; roadId < roads_new.size()-2; roadId++) {
            //WINDOWS systems:
            string roadPath = "./data/" + roads_new[roadId+2] + "/";
            posicaoInicial = roadPath.size();
            cout << roadPath << endl;
            fileName = getMostRecentFile(roadPath,ref(iter));
            if (fileName == "") {
                cout << "No file found" << endl;
                continue;
            }

            Road positions = splitData(fileName);

            //UNIX systems:
            //string fileName = getMostRecentData("./data/" + roads[roadId]);
            thread t(deleteAllFiles, roadPath);
            t.detach();
            //string fileName = "./data/" + roads[roadId+2] + "/" + to_string(d) + "_" + roads[roadId+2] + "_120_mockdata.txt";// PEGAR O ARQUIVO MAIS RECENTE

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
                Road speeds = calc_speed(positions, old_positions); // thread pra calcular pra cada lane -> join
                speeds_list.push_back(speeds);
                cout << "Without history data" << endl;
                historySpeedsData[roadId] = speeds;
            }

            else {
                Road speeds = calc_speed(positions, old_positions);
                speeds_list.push_back(speeds);
                Road accel = calc_accel(speeds, old_speeds); // threads pra calcular pra cada lane -> join
                accelerations_list.push_back(accel);
                Road collision_risk;
                vector<thread> thread_vec;
                mutex mtx_print;
                mutex mtx_risk;
                for (int i = 0; i < positions.size(); i++)
                {
                    cout << positions[i].size() << endl;
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
        int n_road = roads_new.size() - 2;
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
                    cout << "Posição: " << get<1>(positions_list[road][lane][car]) << endl;

                    bool speed_found = false;
                    for (int road_speed = 0; road_speed < speeds_list.size(); road_speed++) {
                        for (int lane_speed = 0; lane_speed < speeds_list[road_speed].size(); lane_speed++) {
                            for (int car_speed = 0; car_speed < speeds_list[road_speed][lane_speed].size(); car_speed++) {
                                if (get<0>(speeds_list[road_speed][lane_speed][car_speed]) == get<0>(positions_list[road][lane][car])) {
                                    cout << "Velocidade: " << get<1>(speeds_list[road_speed][lane_speed][car_speed]) << get<0>(speeds_list[road_speed][lane_speed][car_speed]) << endl;
                                    cout << "Placa" << get<0>(positions_list[road][lane][car]) << endl;
                                    speed_found = true;
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
                                    cout << "Aceleração: " << get<1>(accelerations_list[road_acceleration][lane_acceleration][car_acceleration]) << "Placa" <<  get<0>(accelerations_list[road_acceleration][lane_acceleration][car_acceleration]) << endl;
                                    acceleration_found = true;
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
                                    cout << "Risco de colisão: " << get<1>(collision_risk_list[road_collision][lane_collision][car_collision]) << "Placa" <<  get<0>(collision_risk_list[road_collision][lane_collision][car_collision]) << endl;
                                    collision_risk_found = true;
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
            //cout << "Tempo de análise: " << mil_sec << endl;
            if (fileName == "") {
                cout << "Tempo de análise: não há arquivo de entrada" << endl;
            }
            else {
                int latestTime = stoi(fileName.substr(posicaoInicial +1, posicaoInicial + 8));
                cout << "Tempo de análise: " << mil_sec - latestTime << "ms" << endl;
            }
        }
    }
    return 0;
}