#include <vector>
#include <mutex>
#include <Legado.cpp>

int get_info(vector<string> data) {
    //Get vector with unique plates from the data

    vector<string> plates = ['example1', 'example2'];

    int QUEUE_CAPACITY = 5;

    Legado* legado = new Legado(QUEUE_CAPACITY);

    for (int i=0; i<plates.size(); i++) {
        typedef int semaphore;

        semaphore requests = 0;
        semaphore legado_api = 0;
        semaphore mutex = 1;

        down(&mutex);
        if (legado->actual_capacity < QUEUE_CAPACITY){
                
                //Check-in
                legado->actual_capacity += 1;
                
                //Query vehicle
                legado->query_vehicle(plates[i]);
    
                //Check-out
                legado->actual_capacity -= 1;
    
                //Get vehicle info
                string name = legado->get_name();
                string model = legado->get_model();
                string year = legado->get_year();
    
                //Save vehicle info
                //TODO: save in a vector

        }

    }

}



int main() {
    return 0;
}