#include "Analysis.hpp"

using namespace std;
typedef vector<tuple<string,int>> VectorLane;
typedef vector<vector<tuple<string,int>>> MatrixRoads;

VectorLane calc_speed(VectorLane positions, VectorLane* old_position) {
    VectorLane calculated_speed;

    int positions_size = positions.size();
    for(int i =0; i< positions.size(); i++) {
        for (int j = 0; j < old_position->size(); j++) {
            if (get<0>(positions[i]) == get<0>(old_position->at(j))) {
                int speed = get<1>(positions[i]) - get<1>(old_position->at(j));
                calculated_speed.push_back(make_tuple(get<0>(positions[i]), speed));
            }
        }
    }

    old_position = &positions;
    return calculated_speed;
}

VectorLane calc_accel(VectorLane speed, VectorLane* old_speeds) {
    VectorLane calculated_accel;

    int speed_size = speed.size();
    for(int i =0; i< speed.size(); i++) {
        for (int j = 0; j < old_speeds->size(); j++) {
            if (get<0>(speed[i]) == get<0>(old_speeds->at(j))) {
                int accel = get<1>(speed[i]) - get<1>(old_speeds->at(i));
                calculated_accel.push_back(make_tuple(get<0>(speed[i]), accel));
            }
        }
    }

    old_speeds = &speed;
    return calculated_accel;
}

VectorLane calc_collision_risk(VectorLane positions, VectorLane speed, VectorLane accel) {
    VectorLane collision_risk;

    // sort position by tuple second value
    sort(positions.begin(), positions.end(), [](const tuple<string,int>& a, const tuple<string,int>& b) {
        return get<1>(a) < get<1>(b);
    });

    // for car in positions, look for it in speed and accel
    for (int i = 0; i < positions.size(); i++) {
        for (int j = 0; j < speed.size(); j++) {
            for (int k = 0; k < accel.size(); k++) {
                if (get<0>(positions[i]) == get<0>(speed[j]) && get<0>(positions[i]) == get<0>(accel[k])) {
                    int speed_accel = get<1>(positions[i]) + get<1>(speed[j]) + get<1>(accel[k]);
                    positions[i] = make_tuple(get<0>(positions[i]), speed_accel);
                }
            }
        }
    }

    // check if car passed the car in front of it
    // if it did, both cars are in collision risk
    int front_car_risk = 0;
    for (int i = 0; i < positions.size(); i++) {
        if (i == positions.size() - 1) {
            break;
        }
        if (get<1>(positions[i]) >= get<1>(positions[i+1])) {

            if (front_car_risk != 1){
                collision_risk.push_back(make_tuple(get<0>(positions[i]), 1));
                collision_risk.push_back(make_tuple(get<0>(positions[i+1]), 1));
            } else {
                collision_risk.push_back(make_tuple(get<0>(positions[i+1]), 1));
            }
            front_car_risk = 1;

        } else {

            if (front_car_risk != 1) {
                collision_risk.push_back(make_tuple(get<0>(positions[i]), 0));    
            }
            front_car_risk = 0;
        }
    }


    return collision_risk;
}   

vector<VectorLane> calc_all(VectorLane positions, VectorLane* old_positions, VectorLane* old_speeds) {
    vector<VectorLane> calc_vector;

    VectorLane vector_speed = calc_speed(positions, old_positions);
    VectorLane vector_accel = calc_accel(vector_speed, old_speeds);

    // calc_vector.push_back(vector_speed);
    // calc_vector.push_back(vector_accel);
}



void init_pos_speed(VectorLane pos, VectorLane* old_pos, VectorLane* old_speed) {
    VectorLane speed = calc_speed(pos, old_pos);
    old_speed = &speed;
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
