# Sistema de monitoramento de rodovias

Essa é uma aplicação que produz análises a partir de dados de tráfego simulados. O simulador dos dados (*mock*), que atua como cliente do RPC, e o servidor (que insere os dados no banco), foram implementados em Python e o sistema de monitoramento (ETL) em C++.

O Relatório para a etapa RPC do projeto é Relatorio\_Trabalho\_RPC\_Computação\_Escalável.pdf

## Execução do ETL

Para compilar o ETL, é necessário ter o driver MongoDB C++, cuja instalação é descrita em detalhes [nesse tutorial](https://www.mongodb.com/docs/drivers/cxx/).

No nosso caso, por erros na distribuição do Linux Ubuntu utilizado para rodar o ETL, foi necessária a utilização do [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) no comando de compilação. Portanto, entrando na raiz do projeto, o comando para compilar utilizado foi o seguinte:

Linux:
```c++ --std=c++11 -pthread etl/main.cpp $(pkg-config --cflags --libs libmongocxx)```

E em seguida:

Linux:
```./a.out```

No arquivo `main.cpp`, é possível personalizar o tamanho da fila do barbeiro alterando a variável `capacity` na linha 17.

## Personalização das rodovias

Cada instância do *mock* é uma rodovia, de modo que o usuário pode personalizar os parâmetros que orientam a geração dos dados.  A ordem dos parâmetros é:

`name`: nome da rodovia

`lanes`: número de pistas 

`size`: tamanho da rodovia

`circles_to_remove_collision`: número de ciclos até que um carro envolvido em um acidente seja removido da pista

`prob_vehicle_surge`: probabilidade de um veículo surgir na rodovia

`prob_lane_change`: probabilidade de um veículo mudar de pista

`max_speed`: velocidade máxima legal da rodovia

`min_speed`: velocidade mínima legal da rodovia e mínima física do veículo

`collision_risk`: probabilidade de um veículo colidir

`max_acceleration`: aceleração máxima de um veículo na rodovia

`max_decceleration`: desaceleração máxima de um veículo na rodovia

`speed_limit`: limite de velocidade física de um veículo 

Ao executar o arquivo `mock.py`, o usuário precisará definir somente o nome da rodovia e o número de pistas em cada lado da rodovia (isto é, um inteiro par maior que zero). Caso deseje alterar qualquer outro parâmetro, deverá ajustar diretamente o código. Por exemplo:

```
ponte = road(name = "Ponte Rio-Niteroi", 
             lanes = 5, 
             size = 150000, 
             circles_to_remove_collision = 5, 
             prob_vehicle_surge = .5, 
             prob_lane_change = .1, 
             max_speed = 120, 
             min_speed = 60, 
             collision_risk = .2, 
             max_acceleration = 5,
             max_decceleration = 2,
             speed_limit = 200)

main(ponte)
```
