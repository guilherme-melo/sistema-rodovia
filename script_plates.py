import names 
import random
import string
import csv
import os

car_models = ["Jetta", "350Z", "TL", "E-Class", "CTS", "Equinox", "745", "TT", "Grand Cherokee", "Tundra", "88", "Corvair 500","F-Series","Yukon",
 "D350 Club","911","Prius","Grand Marquis","Marquis","Monaro","Golf","Pajero","E-Class","Vandura 1500","Patriot","Corvair 500","Tahoe",
 "Navigator","F430","Mark LT","525","Firefly","Camry","X5","Riviera","LS","xD","Savana 3500","LX","Spirit","Echo","Tredia","xB",
 "Bronco II","Galant","SC","2500 Club Coupe","C-Class","G","Caravan","XJ Series","MX-6","CTS-V","T100","B2600","300ZX","Precis",
 "Blazer","Diablo","Yaris","New Beetle","HHR","Continental GT","Stratus","GTO","Range Rover","Optima","Ram 1500","Sable","CL-Class",
 "Escape","Expo","Econoline E150","Sorento","Taurus","323","Yukon XL 2500","Phaeton","Tundra","Eldorado","Daewoo Magnus","Montero Sport",
 "Galant","Capri","LeSabre","Savana 1500","Altima","200","GS","Mustang","Tiburon","Mirage","XT","RAV4","Trans Sport","CL","Jetta",
 "SLS-Class","Passat","Tahoe","SLK-Class","Montana SV6","Gran Sport","Coupe Quattro","X-Type","Voyager","90","LS","Esprit","Murano",
 "Villager","Boxster","MX-5","Firebird Formula","Impreza","Tracker","TT","Yukon XL 2500","Viper","Legacy","Samurai","XJ Series","Lynx",
 "Grand Voyager","RL","Esprit","Bonneville","Corvette","Sonoma","Intrepid","Seville","Leone","E-Series","Bronco","7 Series",
 "Yukon XL 1500","Concorde","MX-5","Capri","Mustang","X3","ZX2","Silverado 2500","ES","Skylark","Crown Victoria","V70","CLS-Class",
 "Taurus","Sentra","EXP","MX-6","Range Rover","Riviera","Sebring","Esprit Turbo","Ascender","3 Series","M6","Firefly","S-Class","V70",
 "Arnage","GTI","Caravan","LX","Tempest","Elantra","Expedition","Tredia","Grand Cherokee","xA","Charger","Town & Country","Impala","NX",
 "Cayman","SX4","Dakota","Range Rover","Fiero","MX-5","DTS","Durango","A4","Prius","CL-Class","Continental Mark VII","tC","Traverse",
 "R-Class","Challenger","350Z","928","Lancer","Mark VIII","Pathfinder","Cordia","Yaris","Xterra","Town Car","H3","Lumina","Taurus",
 "Range Rover","Veyron","LR2","Scoupe","Century","911","CC","Thunderbird", "W201","Tracer","1500 Club Coupe","940","GX","GT",
 "Dakota Club","Grand Prix","Navigator L","Century","Entourage","Cirrus","xB","Neon","F350","Eldorado","612 Scaglietti","Mustang",
 "Tucson","Thunderbird","1500","Crown Victoria","Yukon","Suburban 2500","530","CL","Yaris","XJ","Diamante","GLC","Mighty Max Macro",
 "Bonneville","X5","B-Series Plus","EX","GLC","Tempo","Caravan","Eclipse","Explorer","Quattroporte","Wrangler","9-5","Century",
 "LTD Crown Victoria","Gran Sport","Ram 1500","Passat","Outlander","Dakota","Corvair","Taurus","Jetta","3000GT","Cutlass","QX",
 "Corrado","Sienna","Grand Am","F430 Spider","Esprit Turbo","E-Class","Firebird","Sierra 2500","H2","Exige","Ramcharger","Ram",
 "Sonata","Element","Quest","JX","SL-Class","GranTurismo","R-Class","Tempo","Metro","Rogue","Tucson","2500 Club Coupe","Savana 1500",
 "Ram 1500","Fox","Mazda3","Swift","Grand Am","Town & Country","Caprice"]

current_dir = os.getcwd()
csv_file_path = os.path.join(current_dir, 'legado_data.csv')

letters = string.ascii_uppercase
plates = []
for letter1 in letters:
    for letter2 in letters:
        for number1 in range(10):
            for number2 in range(10):
                for number3 in range(10):
                        plate = f"{letter1}{letter2}{number1}{number2}{number3}"
                        plates.append(plate)
print("Plates generated successfully")
#Gerar nome, modelo e ano aleatórios para cada uma das placas

with open(csv_file_path, mode='w', newline='') as file:
    for plate in plates:
        writer = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        writer.writerow([plate, names.get_full_name(), random.choice(car_models), random.randint(1960, 2023)])
file.close()
print("CSV file generated successfully")
