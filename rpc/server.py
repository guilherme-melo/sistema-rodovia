import grpc
import rpc_pb2
import rpc_pb2_grpc

from concurrent import futures

class GreeterServicer(rpc_pb2_grpc.GreeterServicer):
    def SayHello(self, request, context):
        response = rpc_pb2.HelloReply()
        response.message = f"Hello, {request.name}!"
        return response

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    rpc_pb2_grpc.add_GreeterServicer_to_server(GreeterServicer(), server)
    server.add_insecure_port("[::]:50051")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
