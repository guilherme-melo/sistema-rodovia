# Sistema de monitoramento de rodovias
This is an application that produces analysis from mocked traffic data, it's implmented in C++ and uses threads to achieve paralelism.

Essa é uma aplicação que produz análises a partir de dados de trágeco simulados. O simulador dos dados (*mock*) foi implementado em Python e o sistema de monitoramento em C++.

# Execução o ETL

To compile the ETL, go to the etl folder and run 
Para compilar o ETL, entre no diretório `etl` e execute no terminal:

Linux:
```g++ -pthread -o main ETL.cpp ```

E em seguida:

Linux:
```./main```

# Personalização do *mock*

Como cada instância do mock é uma rodovia, de modo que o usuário pode personalizar os parâmetros que orientam e geração dos dados. A ordem dos parâmetros é:

`name`: nome da rodovia

`lanes`: número de pistas 

`size`: tamanho da rodovia

`circles_to_remove_collision`: número de ciclos até que um carro envolvido em um acidente seja removido da pista

`prob_vehicle_surge`: probabilidade de um veículo surgir na rodovia

`prob_lane_change`: probabilidade de um veículo mudar de pista

`max_speed`: velocidade máxima legal da rodovia

`min_speed`: velocidade mínima legal da rodovia

`collision_risk`: probabilidade de um veículo colidir

`max_acceleration`: aceleração máxima de um veículo na rodovia

`max_decceleration`: desaceleração máxima de um veículo

Para inicializar a pista nos dois sentidos, é necessário chamar a função `main( )` duas vezes, definindo o objeto `road` criado e o parâmetro `forward` e `backward`.

Por exemplo:

```
ponte = road(name = "Ponte Rio-Niteroi", 
             lanes = 4, 
             size = 2000, 
             circles_to_remove_collision = 5, 
             prob_vehicle_surge = .5, 
             prob_lane_change = .1, 
             max_speed = 120, 
             min_speed = 60, 
             collision_risk = .2, 
             max_acceleration = 5, 
             max_decceleration = 2)
            
main(ponte, "forward")
main(ponte, "backward")
```

