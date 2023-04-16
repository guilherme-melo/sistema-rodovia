import random
import numpy as np

class vehicle:
    def __init__(self, x, y, plate):
        self.x = x
        self.y = y
        self.plate = plate
        self.speed = 0
        self.colision = False
        self.prob_lane_change = 0
        self.max_speed = 0
        self.max_acceleration = 0
        self.max_deceleration = 0
        self.colision_risk = 0

class road:
    def __init__(self, name, lanes, size, cicles_to_remove_colision, speed_limit):
        self.name = name
        self.lanes = lanes
        self.size = size
        self.cicles_to_remove_colision = cicles_to_remove_colision
        self.speed_limit = speed_limit
        self.vehicles = []
        self.vehicles_to_remove = False

def write_to_file(list_cars,timestamp):
    id_file = str(timestamp) + ".txt"
    f = open(id_file, "w")
    for car in list_cars:
        f.write(car.plate + " " + "(" + str(car.x) + "," + str(car.y) + ")" + "\n")
    f.close()

def main():
    while True:
        return 0

