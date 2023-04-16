#include <queue>
#include <string>

class Legado {
    public: 
        string car_plate;
        string car_name;
        string car_model;
        string car_year;
        int actual_capacity; 
        int queue_capacity; //WAITING do codigo do barbeiro
        queue<int> q;

        Legado(int capacity) {
            this->queue_capacity=capacity;
            this->actual_capacity = 0;
        };

        int query_vehicle(string plate) {

            if (this->q.size() == queue_capacity) {
                throw std::invalid_argument('Fila cheia!');
            }    
            string name;
            string model;
            string year;

            // codigo para abrir o arquivo e procurar a placa
            vector[4] data;
            string line;
            fstream file('legado_data.csv', ios::in);
            if(file.is_open()) {
                while(getline(file, line)) {
                    if (line.substr(0, 7)==plate) {
                        stringstream s(line);
                        while (getline(s, word, ', ')) {
                            data.push_back(word); // adiciona placa, proprietario, modelo e ano (nesta ordem)
                        }
                    }    
                }
            }
            this.car_plate = data[0];
            this.car_name = data[1];
            this.car_model = data[2];
            this.car_year = data[3];
            return 0;
        }

        string get_name() {
            return this->car_name;
        }

        string get_model() {
            return this->car_model;
        }

        string get_year() {
            return this->car_year;
        }

};