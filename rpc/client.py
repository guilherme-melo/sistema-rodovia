import grpc
import rpc_pb2
import rpc_pb2_grpc
import sys
import os
import pickle

def run():
    channel = grpc.insecure_channel("localhost:50051")
    stub = rpc_pb2_grpc.RoadSimStub(channel)
    response = stub.Simulate(rpc_pb2.Request(name=pickle.load(open("..\json_string.p", "rb"))))

if __name__ == "__main__":
    run()