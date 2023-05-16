import grpc
import rpc_pb2
import rpc_pb2_grpc
import sys
import os
import pickle
import time

def run():
    channel = grpc.insecure_channel("localhost:50051")
    stub = rpc_pb2_grpc.RoadSimStub(channel)
    
    last_request_name = None 
    
    while True:
        # tries to read the message from the file
        try:
            # change to ..\json_string.p if your o.s. is Windows
            # ../json_string.p if your o.s. is Linux or Mac
            request = rpc_pb2.Request(name=pickle.load(open("../json_string.p", "rb")))
            request_name = request.name

            # checks if the last message is different from the current one
            if request_name != last_request_name:
                # sends the message to the server if it is different
                response = stub.Simulate(request)
                last_request_name = request_name
        except:
            continue

if __name__ == "__main__":
    run()