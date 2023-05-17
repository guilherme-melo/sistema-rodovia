import grpc
import rpc_pb2
import rpc_pb2_grpc
from pymongo import MongoClient
from concurrent import futures
import json

SERVER_URL = 'mongodb://localhost:27017'
DB_NAME = 'etl'

client = MongoClient(SERVER_URL)
db = client[DB_NAME]

class RoadSimServicer(rpc_pb2_grpc.RoadSimServicer):
    def Simulate(self, request, context):
        response = rpc_pb2.Response()
        response.message = f"{request.name}"
        json_message = json.loads(response.message)
        
        # get the avenue name
        avenue_name = json_message['avenue']
        COLLECTION_NAME = avenue_name
        
        collection_names = db.list_collection_names()

        if COLLECTION_NAME not in collection_names:
            print("Creating collection: ", COLLECTION_NAME)
            db.create_collection(COLLECTION_NAME)
        
        connection = db[COLLECTION_NAME]
        cursor = connection.insert_one(json_message)
        print(cursor.inserted_id)
        return response

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    rpc_pb2_grpc.add_RoadSimServicer_to_server(RoadSimServicer(), server)
    server.add_insecure_port("[::]:50051")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
