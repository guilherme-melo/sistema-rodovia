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

// 1 ( RODOVIAS) ----------------------------------------------------------------------------------

// Funcao que retorna o nome de todas as pastas (rodovias)
vector<string> get_roads()
{

    const char *folder_path = "./data/";
    vector<string> folders;
    DIR *dirp = opendir(folder_path);

    struct dirent *entry;

    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_DIR && string(entry->d_name) != "." && string(entry->d_name) != "..")
        {
            folders.push_back(string(entry->d_name));
        }
    }

    closedir(dirp);

    return folders;
}

// 2 (CALCULA VELOCIDADE, ACELERACAO, RISCO DE COLISAO) ----------------------------------------------------------------------------------


// Funcao de calculo de velocidade em uma thread
void calc_speed_thread(Road positions, Road *old_position, Road *calculated_speed, int i, int j, mutex &mtx, int n_cicles){
    // Procura a posicao anterior do mesmo carro (pela placa)
    for (int y_old = 0; y_old < old_position->size(); y_old++)
    {
        for (int x_old = 0; x_old < old_position->at(y_old).size(); x_old++)
        {
            if (get<0>(positions[i][j]) == get<0>(old_position->at(y_old)[x_old]))
            {
                // Calcula a velocidade atual (posicao atual - posicao anterior)
                int speed = get<1>(positions[i][j]) - get<1>(old_position->at(y_old)[x_old]);
                speed = (int) abs(speed)/n_cicles;
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
Road calc_speed(Road positions, Road* old_position, int n_cicles){
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
            threads.push_back(thread(calc_speed_thread, positions, old_position, &calculated_speed, y, x, ref(mtx), n_cicles));
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


// Funcao que checa o limite de velocidade de cada pista e adiciona os limites a um vetor
void speed_limits(vector<string> folders, vector<int> *s_limits) {
    // Passa pelo vetor de pastas e pega o limite de velocidade de cada uma
    for (int i = s_limits->size(); i < folders.size(); i++) {
        size_t start = folders[i].find('_');
        cout << folders[i] << endl;
        int speed =  stoi(folders[i].substr(start+1, folders[i].size()));
        s_limits->push_back(speed);
    }
}


// Funcao que checa se cada carro esta acima do limite de velocidade pista por pista
void is_above_limit(Lane* lane_to_calc,int limit, vector<vector<tuple<string,bool>>>* answer, mutex& mtx) {
    int size = lane_to_calc->size();
    vector<tuple<string,bool>> answ;

    // Passa pelo vetor da pista checando se cada carro está acima do limite e adiciona o resultado booleano a um vetor
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

//Aa função que deleta todos arquivos
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
        }
    }
}

string getMostRecentFile(const string& folderPath,int& iter) {
    DIR* dirp = opendir(folderPath.c_str());
    struct dirent * dp;
    time_t latestTime = 0;
    std::string latestFile;
    while ((dp = readdir(dirp)) != NULL) {
        iter++;
        struct stat fileStat;
        std::string filePath = folderPath + dp->d_name;
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
