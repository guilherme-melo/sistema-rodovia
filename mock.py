import random
import numpy as np
import string
import os
#sleep import
import time
    
class vehicle:
    def __init__(self, x, y, plate, speed):
        self.x = x
        self.y = y
        self.plate = plate
        self.speed = speed
        self.acceleration = 0

        self.collision = False
        self.cicles_to_remove_collision = 0

class road:
    def __init__(self, name, lanes, size, cicles_to_remove_collision,
                 prob_vehicle_surge, prob_lane_change, max_speed, min_speed, collision_risk,
                 max_acceleration, max_decceleration):
        self.name = name
        self.lanes = lanes
        self.size = size
        self.speed_limit = 200 # limite físico para qualquer carro
        self.prob_vehicle_surge = prob_vehicle_surge
        self.prob_lane_change = prob_lane_change
        self.max_speed = max_speed
        self.min_speed = min_speed
        self.cicles_to_remove_collision = cicles_to_remove_collision
        self.collision_risk = collision_risk
        self.max_acceleration = max_acceleration
        self.max_decceleration = max_decceleration # positivo 

        self.vehicles = []
        self.vehicles_to_remove = False

# cria arquivo de texto com os dados
def write_to_file(name, max_speed, list_cars, timestamp, mode, lanes, size):
    
    id_file = "data/" + name  + "_" + str(max_speed) + "/" + timestamp + "_mockdata.txt"
    str_file = ""

    id_folder = os.path.dirname(id_file)
    if not os.path.exists(id_folder):
        os.makedirs(id_folder)

    for car in list_cars:
        if mode == "forward":
            str_file += (car.plate + " " + "(" + str(car.x) + "," + str(car.y) + ")" + "\n")
        elif mode == "backward":
            str_file += (car.plate + " " + "(" + str(size - car.x) + "," + str(lanes + car.y) + ")" + "\n")

    if mode == "forward": # apaga o arquivo e sobrescreve
        f = open(id_file, "w")
    elif mode == "backward": # adiciona ao final do arquivo
        f = open(id_file, "a")
    f.write(str_file)
    f.close()


def calc_speed(car):
    return car.speed

def car_plate():
    # cria a placa do carro
    letters = ''.join(random.choices(string.ascii_uppercase, k=2))
    numbers = ''.join(random.choices(string.digits, k=3))
    plate = letters + numbers
    return plate

def sub(road, mode):
    # cria a matriz que representa a estrada
    matrix_cars = np.full((road.size,road.lanes), "XXXXXX")

    cars = road.vehicles
    cars.sort(key = calc_speed) # ordena os carros por velocidade

    for car in cars:
        print(car.x)
        # atualiza o contador de ciclos para remover a colisão
        if car.collision:
            car.cicles_to_remove_collision += 1
            if car.cicles_to_remove_collision == road.cicles_to_remove_collision:
                print("Removed", car.plate)
                cars.remove(car)
                continue

        trigger_collision = False

        for i in range(car.speed):
            # checa se há carros no meio do avanço do carro
            achieved_speed = i
            if matrix_cars[car.x + i][car.y] != "XXXXXX":
                trigger_collision = True
                collision_pos = car.x + i
                plate_colided = matrix_cars[car.x + i][car.y]
                break

        # forçar colisão se a probabilidade de colisão for maior que o random
        if random.random() < road.collision_risk and trigger_collision:
            car.x = collision_pos
            car.collision = True
            # define que o carro em que ele bateu também colidiu
            for car_2 in cars:
                if car_2.plate == plate_colided:
                    car_2.collision = True

        # se nao houver, só atualiza a posicao do carro
        if not trigger_collision:
            matrix_cars[car.x + car.speed][car.y] = car.plate
            car.x += car.speed

        # checa a possibilidade de o carro trocar de pista
        # adicionamos car.collision == False para evitar que o carro troque de pista se ja colidiu
        if trigger_collision and car.collision == False:
            # se conseguiu trocar de pista, nao ha mais risco de colisao
            if car.y != 0: # para a esquerda
                if matrix_cars[collision_pos][car.y - 1] != "XXXXXX":
                    trigger_collision = False
                    matrix_cars[collision_pos][car.y - 1] = car.plate

            if car.y != road.lanes-1: # para a direita
                if matrix_cars[collision_pos][car.y + 1] != "XXXXXX": #and trigger_collision:
                    trigger_collision = False
                    matrix_cars[collision_pos][car.y + 1] = car.plate

            # se nao conseguiu trocar de pista, diminui a velocidade
            if trigger_collision:
                if car.speed - (achieved_speed+1) < road.max_decceleration:
                    car.speed -= (achieved_speed+1)
                    matrix_cars[car.x + car.speed][car.y] = car.plate
                    trigger_collision = False

        # se nao conseguiu trocar de pista ou diminuir a velocidade, colidiu
        if trigger_collision:
            car.x = collision_pos
            car.collision = True
            print("Colided", car.plate)
            # define que o carro em que ele bateu também colidiu
            for car_2 in cars:
                if car_2.plate == plate_colided:
                    car_2.collision = True

        if car.collision == True:
            car.speed = 0
        else:
            # define velocidade e aceleração do carro
            if car.speed == 0:
                car.speed = road.max_decceleration
            else:
                car.speed += car.acceleration

            accel_new = random.randrange(-road.max_acceleration,road.max_acceleration)

            # mantém a velocidade dos carros em movimento acima do limite minimo
            if car.speed + accel_new < road.min_speed:
                car.acceleration = road.max_decceleration - 1
            # ou evita que a velocidade do carro ultrapasse o limite físico do carro
            elif car.speed + accel_new > road.speed_limit:
                car.speed = road.speed_limit
            # ou atualiza a aceleração
            else:
                car.acceleration = accel_new

            # tira o carro da pista caso sua posição seja maior que o tamanho da pista
            if car.x + car.speed > road.size-1:
                cars.remove(car)

        # checa se o carro vai trocar de pista
        if road.prob_lane_change > random.random():
            if car.speed > 0:
                if car.y != 0 and car.y != road.lanes -1:
                    car.y = random.choice([car.y+1,car.y-1])
                elif car.y == 0:
                    car.y += 1
                else:
                    car.y -= 1

    for i in range(road.lanes):
        if random.random() < road.prob_vehicle_surge:
            plate = car_plate()
            # carros entram com uma velocidade entre o mínimo e o máximo da pista
            car = vehicle(0, i, plate, random.randint(road.min_speed, road.max_speed))
            cars.append(car)
    road.vehicles = cars

    # escreve dependendo do sentido da rodovia em questão



    time.sleep(0.05)

def main(road1,road2):
    # while True:
    road1.vehicles = []
    road2.vehicles = []
    contador = -1
    while True:
        tempo = int(1000*time.time())
        tempo = str(tempo)[-9:]
        sub(road1, "forward")
        write_to_file(road1.name, road1.max_speed, road1.vehicles, tempo, "forward", road1.lanes, road1.size)
        sub(road2, "backward")
        write_to_file(road2.name, road2.max_speed, road2.vehicles, tempo, "backward", road2.lanes, road2.size)

var = input("Digite o nome do arquivo: ")

rod_ida = road(var, 3, 150000, 5, .5, .1, 120, 60, .2, 5, 2)
rod_volta = road(var, 3, 150000, 5, .5, .1, 120, 60, .2, 5, 2)
main(rod_ida, rod_volta)
