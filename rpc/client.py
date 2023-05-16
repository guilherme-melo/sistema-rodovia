import grpc
import rpc_pb2
import rpc_pb2_grpc
import sys

sys.path.append("../") 
from mock import *

def run():
    channel = grpc.insecure_channel("localhost:50051")
    stub = rpc_pb2_grpc.RoadSimStub(channel)
    response = stub.Simulate(rpc_pb2.Request(name=json_string))
    print(response.message)

if __name__ == "__main__":
    run()
