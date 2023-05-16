from pymongo import MongoClient

#TODO: RECEIVE MESSAGE (DATA) FROM MOCK PYTHON IN JSON FORMAT
SERVER_URL = 'mongodb://localhost:27017'
DB_NAME = 'etl'
COLLECTION_NAME = 'car-data'

data = {
    "time": "tempo que vinha no nome do arquivo",
    "avenue": "avenue_name",
    "cars": {
        "car1": "position",
        "car2": "position"
    }
}


def connect_database(server_url, db_name, collection_name):
    client = MongoClient(server_url)
    db = client[db_name]
    return db[collection_name]

connection = connect_database(SERVER_URL, DB_NAME, COLLECTION_NAME)
    
cursor = connection.insert_one(data)

print(cursor.inserted_id)
