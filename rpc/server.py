import grpc
import rpc_pb2
import rpc_pb2_grpc
from pymongo import MongoClient
from concurrent import futures
import json

SERVER_URL = 'mongodb://localhost:27017'
DB_NAME = 'etl'
COLLECTION_NAME = 'car-data'

def connect_database(server_url, db_name, collection_name):
    client = MongoClient(server_url)
    db = client[db_name]
    return db[collection_name]

connection = connect_database(SERVER_URL, DB_NAME, COLLECTION_NAME)

class RoadSimServicer(rpc_pb2_grpc.RoadSimServicer):
    def Simulate(self, request, context):
        response = rpc_pb2.Response()
        response.message = f"{request.name}"
        json_message = json.loads(response.message)
        cursor = connection.insert_one(json_message)
        print(cursor.inserted_id)
        
        #print(response.message, '\n')
        return response

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    rpc_pb2_grpc.add_RoadSimServicer_to_server(RoadSimServicer(), server)
    server.add_insecure_port("[::]:50051")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
