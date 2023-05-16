import grpc
import rpc_pb2
import rpc_pb2_grpc

from concurrent import futures

class RoadSimServicer(rpc_pb2_grpc.RoadSimServicer):
    def Simulate(self, request, context):
        response = rpc_pb2.Response()
        response.message = f"{request.name}"
        print(response.message, '\n')
        return response

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    rpc_pb2_grpc.add_RoadSimServicer_to_server(RoadSimServicer(), server)
    server.add_insecure_port("[::]:50051")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
