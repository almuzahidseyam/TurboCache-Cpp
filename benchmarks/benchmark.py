import socket
import time
import threading
import argparse

def worker_task(host, port, num_requests, thread_id):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        
        # SET phase
        for i in range(num_requests):
            key = f"key_{thread_id}_{i}"
            val = f"val_{thread_id}_{i}"
            cmd = f"SET {key} {val}\r\n"
            s.sendall(cmd.encode('utf-8'))
            s.recv(1024)
        
        # GET phase
        for i in range(num_requests):
            key = f"key_{thread_id}_{i}"
            cmd = f"GET {key}\r\n"
            s.sendall(cmd.encode('utf-8'))
            s.recv(1024)

        s.close()
    except Exception as e:
        print(f"Error in thread {thread_id}: {e}")

def run_benchmark(host, port, threads, requests_per_thread):
    print(f"Starting TurboCache Benchmark...")
    print(f"Target: {host}:{port}")
    print(f"Firing {threads * requests_per_thread * 2} total requests ({threads} threads)...\n")
    
    start_time = time.time()
    
    thread_list = []
    for i in range(threads):
        t = threading.Thread(target=worker_task, args=(host, port, requests_per_thread, i))
        thread_list.append(t)
        t.start()
        
    for t in thread_list:
        t.join()
        
    end_time = time.time()
    total_time = end_time - start_time
    total_requests = threads * requests_per_thread * 2 # SET and GET
    throughput = total_requests / total_time if total_time > 0 else 0
    
    print(f"=====================================")
    print(f"         BENCHMARK RESULTS           ")
    print(f"=====================================")
    print(f"Total Threads       : {threads}")
    print(f"Requests per Thread : {requests_per_thread * 2} (50% SET, 50% GET)")
    print(f"Total Requests      : {total_requests}")
    print(f"Time Taken          : {total_time:.4f} seconds")
    print(f"Throughput          : {throughput:.2f} req/sec")
    print(f"=====================================\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="TurboCache Performance Benchmarking Tool")
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Server host")
    parser.add_argument("--port", type=int, default=6379, help="Server port")
    parser.add_argument("--threads", type=int, default=10, help="Number of concurrent client threads")
    parser.add_argument("--requests", type=int, default=1000, help="Number of SET+GET pairs per thread")
    args = parser.parse_args()
    
    run_benchmark(args.host, args.port, args.threads, args.requests)
