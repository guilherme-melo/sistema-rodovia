# -*- coding: utf-8
"""Implementa una simulacion del enfoque microscopico de modelado del
trafico vehicular.

Attributes:
  BRAKE_PROB (float): probabilidad que un vehiculo desacelere.
  COLISION_PROB (float): probabilidad que un vehiculo no respete su
    distancia de seguridad e impacte al vehiculo de enfrente.
  POS_MAX (int): la cantidad de casillas que contiene el carril.
  T_MAX (int): el numero de iteraciones que realizara la simulacion.
  VEL_MAX (int): la velocidad maxima que los vehiculos pueden alcanzar.
"""

import random
import sys
import time

T_MAX = 50
VEL_MAX = 9
POS_MAX = 13
BRAKE_PROB = 0.4
COLISION_PROB = 0.1

class Vehicle(object):
    """Representa a las unidades basicas del modelo microscopico.
    
    Attributes:
      vid (object): una etiqueta para identificar al vehiculo.
      p (int): la casilla del carril en donde esta el vehiculo.
      v (int): la velocidad del vehiculo.
    """
    
    def __init__(self, plate):
        """Crea un nuevo vehiculo.
        
        El coche automaticamente obtiene una casilla y una
        velocidad inicial.
        
        Args:
          vid (object): la etiqueta que identificara al vehiculo.
        """
        self.plate = plate
        self.p = random.randint(0, POS_MAX)
        self.v = random.randint(0, VEL_MAX)

    def __str__(self):
        return self.vid + ':' + str(self.v)


def distance(v1, v2):
    """Calcula la distancia entre dos vehiculos.
    
    Como la simulacion utiliza un carril continuo, la distancia
    de los dos vehiculos depende de la posicion de los mismos
    (es decir, distance(v1, v2) no es necesariamente igual a
    distance(v2, v1)).
    
    Args:
      v1 (int): la casilla en la que se encuentra el vehiculo 1.
      v2 (int): la casilla en la que se encuentra el vehiculo 2.
    
    Returns:
      int: la distancia del vehiculo 1 al vehiculo 2.
    """
    if (v1 < v2):
        return (v2 - v1) - 1
    else:
        return (VEL_MAX - v2) + v1 - 1


def print_usage():
    """Imprime una guia de como usar el programa."""
    print("USAGE: python[3] traffic.py [-b B] [-c C] [-t T]")
    print("  where:")
    print("  -b: establece la probabilidad de frenado. Debe ser")
    print("        un numero entre 0 y 1.")
    print("  -c: establece la probabilidad de colision. Debe ser")
    print("        un numero entre 0 y 1.")
    print("  -t: establece la cantidad de iteraciones que realizara")
    print("        la simulacion.")

if __name__ == '__main__':
    """Realiza la simulacion"""
    # Intentamos leer parametros
    try:
        for i, arg in enumerate(sys.argv):
            if arg == '-b':
                BRAKE_PROB = float(sys.argv[i + 1])
            elif arg == 'c':
                COLISION_PROB = float(sys.argv[i + 1])
            elif arg == '-t':
                T_MAX = int(sys.argv[i + 1])

            if arg == '-h':
                print_usage()
                sys.exit(0)
    except ValueError:
        print_usage()
        sys.exit(1)
    except IndexError:
        print_usage()
        sys.exit(1)
    
    # Imprime el encabezado
    sys.stdout.write('\t| ');
    
    for i in range(0, POS_MAX):
        if(i < 9):
            sys.stdout.write('  {0} |'.format(i + 1))
        else:
            sys.stdout.write(' {0} |'.format(i + 1))
    
    sys.stdout.write('\n')
    sys.stdout.flush()
    
    # Crea los vehiculos de la simulacion
    car1 = Vehicle('X')
    car2 = Vehicle('Y')
    
    # Estas variables se usan en caso de choque
    grua = 0
    choque = False
    
    # Imprime condiciones iniciales
    sys.stdout.write('Con. In.| ')
    for j in range(0, POS_MAX):
        if j == car1.p:
            sys.stdout.write(' X:' + str(car1.v) + '|')
        elif j == car2.p:
                sys.stdout.write(' Y:' + str(car2.v) + '|')
        else:
            sys.stdout.write('    |')
        
    sys.stdout.write('\n')
    sys.stdout.flush()
    
    # Realizamos la simulacion
    for i in range(0, T_MAX):
        sys.stdout.write('t{0}\t| '.format(i))
        
        # Verificamos si hubo colision
        if choque and grua < 5:
            # La grua tarda cinco iteraciones en terminar
            grua += 1
        elif choque and grua == 5:
            # La grua termina y separa a los coches
            grua = 0
            choque = False
            car1.p = (car1.p + 1) % POS_MAX
        else:
            # No hay choque: el trafico circula con normalidad
            # Paso 1
            if car1.v < VEL_MAX:
                car1.v += 1
            
            if car2.v < VEL_MAX:
                car2.v += 1
            
            # Paso 2:
            if  random.random() > COLISION_PROB:
                car1.v = min(car1.v, distance(car1.p, car2.p))
            
            if  random.random() > COLISION_PROB:
                car2.v = min(car2.v, distance(car2.p, car1.p))
            
            # Paso 3:
            if random.random() < BRAKE_PROB:
                car1.v -= 1
            if car1.v < 0:
                car1.v = 0
            
            if random.random() < BRAKE_PROB:
                car2.v -= 1
            if car2.v < 0:
                car2.v = 0
            
            # Paso 4:
            car1.p = (car1.p + car1.v) % POS_MAX
            car2.p = (car2.p + car2.v) % POS_MAX
            
            # Verificamos si hubo una colision
            if car1.p == car2.p:
                choque = True
        
        # Imprime el resultado de la iteracion
        for j in range(0, POS_MAX):
            if choque and j == car1.p:
                sys.stdout.write(' X-Y|')
            elif j == car1.p:
                sys.stdout.write(' X:' + str(car1.v) + '|')
            elif j == car2.p:
                sys.stdout.write(' Y:' + str(car2.v) + '|')
            else:
                sys.stdout.write('    |')
        
        sys.stdout.write('\n')
        sys.stdout.flush()
        
        time.sleep(1)