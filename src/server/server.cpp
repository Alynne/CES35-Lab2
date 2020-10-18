
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <forward_list>
#include <sstream>
#include <thread>
#include "server.h"
#include "http.h"

#include <fstream>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string>


http_connection::http_connection(int connSocket, struct sockaddr_in clientAddr, const fs::path& servingRoot) 
:   connSocket(connSocket), 
    clientAddr(clientAddr), 
    servingRoot(servingRoot)
{}

void
http_connection::serve() {
    // Call recv to receive request
    http::response response(http::status::Ok);
    response.initialize(connSocket);
    http::bytes data;
    try {
        auto request = recvRequest();
        fs::path resourcePath;
        //
        // Check path, respond 400 or 404 if there are any errors.
        //
        auto resourcePathStr = request.getUrl().getPath();
        std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Client requests resource: /" << resourcePathStr << std::endl;
        resourcePath = servingRoot / fs::path(resourcePathStr);
        std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Requested resource location: " << resourcePath.string() << std::endl;
        if (!fs::exists(resourcePath)) {
            // Respond 404, no such path
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Resource not found! Reply 404" << std::endl;
            response.setStatusCode(http::status::NotFound);
            data = "ERROR 404: Not found";
            response.setContentLength(data.size());
        } else if (fs::is_directory(resourcePath)) {
            // Look for an "index.html" inside the directory
            resourcePath = resourcePath / fs::path("index.html");
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Looking for " << resourcePath.string() << std::endl;
            if (!fs::exists(resourcePath)) {
                // Respond 404, no such index.html
                std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Resource not found! Reply 404" << std::endl;
                response.setStatusCode(http::status::NotFound);
                data = "ERROR 404: Path has no index.html";
                response.setContentLength(data.size());
            }
        }
        if (response.getStatusCode().getCode() != http::status::Ok) {
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Sending header..." << std::endl;
            response.sendHead();
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Sent!" << std::endl;
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Sending body..." << std::endl;
            response.sendBodyPart(data);
            std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Sent!..." << std::endl;
        } else {
            // resource is set, open it, set content length, send header and send body
            send(response, resourcePath);
        }
    } catch (std::runtime_error& err) {
        std::cerr << "[THREAD " << std::this_thread::get_id() << "]: " << err.what() << std::endl;
        std::cerr << "[THREAD " << std::this_thread::get_id() << "]: " << "Send response 400." << std::endl;
        response.setStatusCode(http::status::BadRequest);
        data = "ERROR 400: Bad Request";
        response.setContentLength(data.size());
        try {
            response.sendHead();
            response.sendBodyPart(data);
        } catch(std::runtime_error& err) {
            std::cerr << "[THREAD " << std::this_thread::get_id() << "]: " << "Fatal error when responding HTTP 400" << std::endl;
            std::cerr << "[THREAD " << std::this_thread::get_id() << "]: " << "Aborting connection..." << std::endl;
        }
    }
    std::cout << "[THREAD " << std::this_thread::get_id() << "]: " << "Finishing worker..." << std::endl;
    close(connSocket);
}

http::request
http_connection::recvRequest() {
    auto requestParseResult = http::request::parse(connSocket);
    if (!requestParseResult.has_value()) {
        // Some problem occured when receiving the request.
        std::stringstream ss;
        ss  << "Failed to receive and parse incoming request from " 
            << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port;
        throw std::runtime_error(ss.str());
    }
    http::request recvRequest = requestParseResult.value().first;
    http::bytes body = requestParseResult.value().second;
    for (auto&item : recvRequest.getHeaders()) {
        std::cout << "Header \"" << item.first << "\": " << item.second << std::endl;
    }
    // std::cout << "Header \"" << "content-length" << "\": " << recvRequest.getContentLength() << std::endl;
    // std::cout << "leftover body bytes:" << body.size() << std::endl;
    //
    // Call recv to receive remaining body.
    //
    const size_t BODY_PART_SIZE = 4096;
    http::bytes remainingBody(BODY_PART_SIZE, '\0');
    size_t totalBytesReceived = 0;
    while (true) {
        size_t bytesReceived = 0;
        try {
            bytesReceived = recvRequest.recvBody(remainingBody);
        } catch (std::logic_error& err) {
            std::stringstream ss(err.what());
            ss << std::endl << "HTTP Client socket abruptly disconnected.";
            throw std::runtime_error(ss.str());
        }
        if (bytesReceived == 0) break;
        totalBytesReceived += bytesReceived;
        if (totalBytesReceived == remainingBody.length()) {
            remainingBody.resize(remainingBody.length() + BODY_PART_SIZE);
        }
    }
    body.append(remainingBody);
    //
    // We just ignore this body stuff k k k k k k
    // Not useful for this activity
    //
    return {recvRequest};
}

void 
http_connection::send(http::response& response, fs::path resourcePath) {
    std::ifstream uploadStream;
    uploadStream.open(resourcePath.string(), std::ios_base::in | std::ios_base::binary);
    // Get file size in bytes
    uploadStream.seekg(0, std::ios::end);
    size_t totalBytes = uploadStream.tellg();
    //
    // Send Header
    //
    response.setContentLength(totalBytes);
    response.sendHead();
    //
    // Send Body
    //
    uploadStream.close();
    uploadStream.open(resourcePath.string(), std::ios_base::in | std::ios_base::binary);
    const size_t UPLOAD_BUFFER_SIZE = 4096;
    http::bytes buffer;
    while (!uploadStream.eof() && !uploadStream.fail() && !uploadStream.bad()) {
        buffer.resize(UPLOAD_BUFFER_SIZE);
        uploadStream.read(buffer.data(), UPLOAD_BUFFER_SIZE);
        size_t bytesRead = uploadStream.gcount();
        buffer.resize(bytesRead);
        response.sendBodyPart(buffer);
        totalBytes -= bytesRead;
    }
    uploadStream.close();
}

void 
http_server::run() noexcept {
    std::atomic<int> openedConnections = 0;
    int totalThreadsSpawned = 0;
    while (true) {
        while (openedConnections >  maxConnections) {
            // Couldn't remove any finished worker. Sleep a bit.
            std::this_thread::sleep_for(std::chrono::milliseconds(workerWaitMSec));
        }
        // We're on budget and may accept a new connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        std::cout << "[MAIN_THREAD]: Waiting for connections... (" << openedConnections << "/" << maxConnections << ")\n";
        int clientSockfd = accept(socket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSockfd == -1) {
            handleAcceptError();
            continue;
        }
        // Spawn worker to handle connection
        http_connection connection(clientSockfd, clientAddr, serverRoot);
        char aux_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), aux_str, INET_ADDRSTRLEN);
        std::cout << "[MAIN_THREAD]: Spawn worker for client " << aux_str << ":" << clientAddr.sin_port << std::endl;
        std::thread worker(connection, std::reference_wrapper(openedConnections)); // Spawn worker in new thread
        worker.detach(); // Detach since we don't retrieve anything from it.
        openedConnections++;
        totalThreadsSpawned++;
        std::cout << "[MAIN_THREAD]: Opened connections: " << openedConnections << std::endl;
        std::cout << "[MAIN_THREAD]: Total threads spawned: " << totalThreadsSpawned << std::endl;
    }
    exit(0);
}

http_server::http_server(std::string host, unsigned short port, std::string dir){
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
    }
    //Resolving address
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    // hints - modo de configurar o socket para o tipo  de transporte
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    int resolve_status = 0;
    std::string portStr = std::to_string(port);
    if ((resolve_status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res))== 0) {
        if (res != NULL){
            addr = *((struct sockaddr_in*) res->ai_addr);
            //Printing ip address for debugging
            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(res->ai_family, &(addr.sin_addr), ipstr, sizeof(ipstr));
            freeaddrinfo(res);
            // Binding the port, so the OS registers it for socket use
            if (bind(socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                perror("bind");
            } else {
                std::cout << "Binded server to: " << ipstr << ":" << portStr.c_str() << std::endl;
            }
            // colocar o socket em modo de escuta, ouvindo a porta 
            const int MAX_CONN_Q = 2*DEFAULT_MAX_CONNECTIONS;
            if (listen(socket, MAX_CONN_Q) == -1) {
                perror("listen");
            } else {
                std::cout << "Socket listening! Max queued connection allowed: " << MAX_CONN_Q << std::endl;
            }
            maxConnections = DEFAULT_MAX_CONNECTIONS;
            workerWaitMSec = DEFAULT_WORKER_WAIT_MSEC;
            if(dir == ""){
                perror("no directory provided.");
            }
            fs::path dir_path  = fs::path(dir);
            serverRoot = fs::absolute(dir_path);
            if(!fs::exists(serverRoot)){
                std::cerr << "Path does not exist."<< std::endl;
            }
            if(!fs::is_directory(serverRoot)){
                std::cerr << "Path is not a directory."<< std::endl;
            }
        }
        else {
            std::cout << "Not able to resolve address."<< std::endl;
        }

    }
    std::cout << "Server ready to accept connections!" << std::endl;
}

void 
http_server::handleAcceptError() noexcept {
    // Error handling
    switch (errno)
    {
    case EAGAIN:
        std::cerr << "http_server::run(): EAGAIN, accept(2) would block." << std::endl;
        std::cerr << "Wrong socket option? Terminating..." << std::endl;
        exit(1);
    case EHOSTDOWN:
    case EHOSTUNREACH:
    case ENETDOWN:
    case ENETUNREACH:
    case ENOPROTOOPT:
    case EPROTO:
        // This errors are not generated by the accept(2) call and should have been
        // already pending. Just ignore and retry.
        break;
    case EBADF:
        std::cerr << "http_server::run(): EBADF, socket is not opened." << std::endl;
        std::cerr << "Socket initialization failed? Terminating..." << std::endl;
        exit(1);
    case ECONNABORTED:
        std::cerr << "http_server::run(): connection aborted before accepting." << std::endl;
        break;
    case EFAULT:
        std::cerr << "http_server::run(): EFAULT, corrupted addr variable." << std::endl;
        std::cerr << "Passing invalid address variable? Terminating..." << std::endl;
        exit(1);
    case EINTR:
        std::cerr << "http_server::run(): EINTR, accept(2) call interrupted." << std::endl;
        break;
    case EINVAL:
        std::cerr << "http_server::run(): EINVAL, socket not listening for connections." << std::endl;
        std::cerr << "Missing a listen(2) call at initialization? Terminating..." << std::endl;
        exit(1);
    case EOPNOTSUPP:
        std::cerr << "http_server::run(): EOPNOTSUPP, socket is not of type SOCK_STREAM." << std::endl;
        std::cerr << "Wrong socket type? Terminating..." << std::endl;
        exit(1);
    case EMFILE:
        std::cerr << "http_server::run(): ENFILE, Reached per-process limit of opened file descriptors." << std::endl;
        exit(2);
    case ENFILE:
        std::cerr << "http_server::run(): ENFILE, Reached per-process limit of opened file descriptors." << std::endl;
        exit(2);
    case ENOBUFS:
    case ENOMEM:
        std::cerr << "http_server::run(): ENOBUFS/ENOMEM, Not enough memory for new connections." << std::endl;
        break;
    case ENOTSOCK:
        std::cerr << "http_server::run(): ENOTSOCK, invalid socket descriptor." << std::endl;
        std::cerr << "Initialized invalid socket? Terminating..." << std::endl;
        exit(1);
    case EPERM:
        std::cerr << "http_server::run(): EPERM, connection blocked by firewall." << std::endl;
        break;
    default:
        std::cerr << "http_server::run(): Unknown error." << std::endl;
        exit(-1);
    }
}