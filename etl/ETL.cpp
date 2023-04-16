#include <vector>
#include <mutex>
#include <pthread>
#include <Legado.cpp>



//THREAD 4
int get_info(vector<string> data) {
    //Get vector with unique plates from the data
    int N_PLATES = 10;

    vector<string> plates = ["example1", "example2"];

    vector<vector<string>> return_data;

    int QUEUE_CAPACITY = 5;

    Legado* legado = new Legado(QUEUE_CAPACITY);

    typedef int semaphore;

    semaphore requests = 0;
    semaphore legado_api = 0;
    semaphore mutex = 1;

    //Create "costumer" function
    void request(int id_thread) { //id_threads is a unique number between 0 and the total numbers of threads
        vector<string> car_data;
        down(&mutex); //Fecha o acesso a classe
        if (legado->actual_capacity < QUEUE_CAPACITY) { //Confere se a fila não ta cheia (se não tiver, paciencia)

            //Check-in
            legado->actual_capacity += 1;
            up(&requests); //Adiciona que tem uma request pra ser processada
            up(&mutex) //Libera a classe para que outras requests também solicitem
            
            down(&legado_api);
            
            //Get informations about this car
            car_data[0] = legado_api->car_plate;
            car_data[1] = legado_api->car_name;
            car_data[2] = legado_api->car_model;
            car_data[3] = legado_api->car_year;

            return_data[id_thread] = car_data; //Save the informations putting the data into the global vector in the right position

        } else { //Fila ja estava cheia, saindo e continuando sem fazer nada
            up(&mutex);
        }
    }

    //Creating "barber" function
    void procces_request(string plate) {
        
        while (true) {
            down(&requests);
            down(&mutex);
            legado->actual_capacity -= 1;

            up(&legado_api);
            up(&mutex);

            //Analyse
            legado->query_vehicle(plate);
        }
    }

    pthread_t thr_request[N_PLATES], thr_procces;

    //Creating the request threads
    for(int i=0; i<N_PLATES; i++) {
        pthread_create(&thr_request[i], NULL, request, NULL);
    }

    pthread_create(&thr_barbeiro, NULL, procces_request, NULL);

    for (int i=0; i<N_PLATES; i++) {
        pthread_join(thr_barbeiro[i], NULL);
    }

    return return_data;
}



int main() {
    return 0;
}