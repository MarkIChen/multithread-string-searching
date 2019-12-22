# OS Mutithread Server and Client

<p>
This is a server and client communication tool implemented by C. The server's job is to search the strings in all files of a specific directory and reply searching results to the client.
</p>

### How to use it?
<p>Please run the makefile in terminal in linux or Mac OS.</p>

### Architecture

#### Server Side
- one main thread to handle all the requests from clients.
- one thread pool to limit the number of worker thread, which is to answear the request from client.

#### Client Side
- Support muti-thread to handle the situation that the searching process takes a long time and the users cannot get response.
- In each query, the client will start a new thread and wait the server to response.
- During waiting, the users can still request another query.
- Before send to server, the client will precheck the formate of the query to make sure that it is a valid request.

### Formate
-  ./server -r ROOT -p PORT -n THREAD_NUMBER
    - ROOT: The path of the files
    - PORT: The port which the server is listening to
    - THREAD_NUMBER: number of (worker) threads in the thread pool
-  ./client -h LOCALHOST -p PORT
    -  LOCALHOST: Server would be running on localhost (127.0.0.1)
    -  PORT: The port which the server is listening to
