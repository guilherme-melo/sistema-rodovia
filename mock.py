import random
import numpy as np
import string

# class BST:
#     def insert(self, key, value):
#         if key < self.key:
#             if self.left is None:
#                 self.left = BST(key, value)
#             else:
#                 self.left.insert(key, value)
#         else:
#             if self.right is None:
#                 self.right = BST(key, value)
#             else:
#                 self.right.insert(key, value)

#     def search(self, key):
#         if key == self.key:
#             return self.value
#         elif key < self.key:
#             if self.left is None:
#                 return None
#             else:
#                 return self.left.search(key)
#         else:
#             if self.right is None:
#                 return None
#             else:
#                 return self.right.search(key)
#     def inorder():

#     def __str__(self):
#         return str(self.key) + " " + str(self.value)    


    
class vehicle:
    def __init__(self, x, y, plate):
        self.x = x
        self.y = y
        self.plate = plate
        self.speed = 0
        self.acceleration = 0
        self.collision = False

class road:
    def __init__(self, name, lanes, size, cicles_to_remove_collision, speed_limit,
                 prob_vehicle_surge,prob_lane_change,max_speed,min_speed,collision_risk,max_acceleration,max_decceleration):
        self.name = name
        self.lanes = lanes
        self.size = size
        self.speed_limit = speed_limit
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
        
        self.min_accel = 0

def write_to_file(list_cars,timestamp):
    id_file = "data/"+ str(timestamp) + ".txt"
    str_file = ""
    for car in list_cars:
        str_file += (car.plate + " " + "(" + str(car.x) + "," + str(car.y) + ")" + "\n")

    f = open(id_file, "w")
    f.write(str_file)
    f.close()
    
def calc_speed(car):
    return car.speed


def car_plate():
    # cria a placa do carro
    first_letters = ''.join(random.choices(string.ascii_uppercase, k=3))
    first_number = ''.join(random.choices(string.digits, k=1))
    second_letter = ''.join(random.choices(string.ascii_uppercase, k=1))
    second_numbers = ''.join(random.choices(string.digits, k=2))
    plate = first_letters + first_number + second_letter + second_numbers
    return plate

def main(road):
    
    # while True:
    for contador in range(30):
        # cria a matriz que representa a estrada
        matrix_cars = np.full((road.size,road.lanes), "XXXXXXX")
        
        cars = road.vehicles
        #cars = cars.sort(key = calc_speed)
        for car in cars:
            # define velocidade e aceleração do carro
            if car.speed == 0:
                car.speed = road.max_decceleration
            else:
                car.speed += car.acceleration
            
            accel_new = random.randrange(-road.max_acceleration,road.max_acceleration)

            if car.speed + accel_new < road.min_speed:
                car.acceleration = road.max_decceleration
            
            
            # checa se o carro vai trocar de pista
            if road.prob_lane_change > random.random():
                if car.speed > 0:
                    if car.y != 0 and car.y != road.lanes -1:
                        car.y = random.choice([car.y+1,car.y-1])
                    elif car.y == 0:
                        car.y += 1
                    else:
                        car.y -= 1

            trigger_collision = False
            for i in range(car.speed):
                # checa se há carros no meio do avanço do carro
                achieved_speed = i
                if matrix_cars[car.x + i][car.y] != "XXXXXXX":
                    trigger_collision = True
                    collision_pos = car.x + i
                    plate_colided = matrix_cars[car.x + i][car.y]
                    break
            
            # forçar colisão se a probabilidade de colisão for maior que o random
            if random.random() < road.collision_risk and trigger_collision:
                car.x = collision_pos   # se tiver, colidiu (y)
                car.collision = True
                print("Colided",car.plate)
                # define que o carro em que ele bateu também colidiu
                for car_2 in cars:
                    if car_2.plate == plate_colided:
                        car_2.colided = True

            # se nao houver só atualiza a posicao do carro
            if not trigger_collision:
                matrix_cars[car.x + car.speed][car.y] = car.plate
                car.x += car.speed
            
            # checa a possibilidade de o carro trocar de pista
            if trigger_collision:
                # se conseguiu trocar de pista, nao ha mais risco de colisao
                if car.y != 0: # para a esquerda
                    if matrix_cars[collision_pos][car.y - 1] != "XXXXXXX":
                        trigger_collision = False
                        matrix_cars[collision_pos][car.y - 1] = car.plate
                if car.y != road.lanes-1: # para a direita
                    if matrix_cars[collision_pos][car.y + 1] != "XXXXXXX" and trigger_collision:
                        trigger_collision = False
                        matrix_cars[collision_pos][car.y - 1] = car.plate
                        
                # se nao conseguiu trocar de pista, diminui a velocidade
                if trigger_collision:
                    if car.speed - (achieved_speed+1) < road.max_decceleration:
                        car.speed -= (achieved_speed+1)
                        matrix_cars[car.x + car.speed][car.y] = car.plate
                        trigger_collision = False


            if trigger_collision:
                car.x = collision_pos   # se tiver, colidiu (y)
                car.collision = True
                print("Colided",car.plate)
                # define que o carro em que ele bateu também colidiu
                for car_2 in cars:
                    if car_2.plate == plate_colided:
                        car_2.colided = True

        for i in range(road.lanes):
            if random.random() < road.prob_vehicle_surge:
                plate = car_plate()
                car = vehicle(0,i,plate)
                cars.append(car)
        road.vehicles = cars
        write_to_file(road.vehicles,contador) 

# to-do
# fazer veiculos entrarem com velocidade
# tirar veiculos do final
# fazer os veiculos que colidiram pararem
# tirar veiculos que colidiram 
# fazer a pista de volta


ponte = road("Ponte Rio-Niteroi", 4, 2000, 20, 100, .5, .1, 150, 0, .2, 5, 2)
main(ponte)
