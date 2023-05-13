import grpc
import rpc_pb2
import rpc_pb2_grpc

def run():
    channel = grpc.insecure_channel("localhost:50051")
    stub = rpc_pb2_grpc.GreeterStub(channel)
    response = stub.SayHello(rpc_pb2.HelloRequest(name="World"))
    print(response.message)

if __name__ == "__main__":
    run()
