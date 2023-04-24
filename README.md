# Sistema de monitoramento de rodovias

Essa é uma aplicação que produz análises a partir de dados de tráfego simulados. O simulador dos dados (*mock*) foi implementado em Python e o sistema de monitoramento em C++.

## Execução o ETL

Para compilar o ETL, entre na raiz do projeto e execute no terminal:

Linux:
```g++ -pthread -o main etl/main.cpp ```

E em seguida:

Linux:
```./main```

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

`min_speed`: velocidade mínima legal da rodovia

`collision_risk`: probabilidade de um veículo colidir

`max_acceleration`: aceleração máxima de um veículo na rodovia

`max_decceleration`: desaceleração máxima de um veículo na rodovia

Ao executar o arquivo `mock.py`, o usuário precisará definir somente o nome da rodovia e o número de pistas em cada lado da rodovia (isto é, um inteiro par maior que zero). Caso deseje alterar qualquer outro parâmetro, deverá ajustar diretamente o código. Por exemplo:

```
ponte = road(name = "Ponte Rio-Niteroi", 
             lanes = 4, 
             size = 150000, 
             circles_to_remove_collision = 5, 
             prob_vehicle_surge = .5, 
             prob_lane_change = .1, 
             max_speed = 120, 
             min_speed = 60, 
             collision_risk = .2, 
             max_acceleration = 5,
             max_decceleration = 2)

main(ponte)
```

Caso a execução retorne uma `Segmentation fault`, altere a linha 47 para:

`string roadPath = "./data/" + roads_new[roadId] + "/";`

 Isso acontece devido às diferenças entre os sistemas operacionais:
