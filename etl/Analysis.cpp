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

vector<VectorLane> calc_all(VectorLane positions, VectorLane* old_positions, VectorLane* old_speeds) {
    vector<VectorLane> calc_vector;

    VectorLane vector_speed = calc_speed(positions, old_position);
    VectorLane vector_accel = calc_accel(vector_speed, old_speeds);

    // calc_vector.push_back(vector_speed);
    // calc_vector.push_back(vector_accel);
}


void init_pos_speed(VectorLane pos, VectorLane* old_pos, VectorLane* old_speed) {
    VectorLane speed = calc_speed(pos, &old_pos);
    old_speed = &speed;
}

int main()
{
    VectorLane pos;
    VectorLane old_pos;
    VectorLane very_old_pos;
    pos.push_back(make_tuple("D", 4));
    pos.push_back(make_tuple("A", 1));
    pos.push_back(make_tuple("B", 2));
    pos.push_back(make_tuple("C", 3));
    

    old_pos.push_back(make_tuple("A", 2));
    old_pos.push_back(make_tuple("C", 3));
    //old_pos.push_back(make_tuple("D", 6));
    old_pos.push_back(make_tuple("B", 1));

    very_old_pos.push_back(make_tuple("A", -1));
    very_old_pos.push_back(make_tuple("B", -2));
    very_old_pos.push_back(make_tuple("C", -3));
    very_old_pos.push_back(make_tuple("D", -4));

    VectorLane old_speed = calc_speed(old_pos, &very_old_pos);
    VectorLane speed = calc_speed(pos, &old_pos);
    VectorLane accel = calc_accel(speed, &old_speed);

    cout << "Old Speed: " << endl;
    for (int i = 0; i < old_speed.size(); i++) {
        cout << get<0>(old_speed[i]) << " " << get<1>(old_speed[i]) << endl;
    }

    cout << "Speed: " << endl;
    for (int i = 0; i < speed.size(); i++) {
        cout << get<0>(speed[i]) << " " << get<1>(speed[i]) << endl;
    }

    cout << "Accel: " << endl;
    for (int i = 0; i < accel.size(); i++) {
        cout << get<0>(accel[i]) << " " << get<1>(accel[i]) << endl;
    }


    return 0;
}
