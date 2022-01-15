#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

const std::string END_CONNECTION {"quit"};
const int buffer_size = 1024;

// waits for message receive, then prints it
void be_receiver(int socket);
// waits for message to send, then sends it
void be_sender(int socket);

int be_server(int port);
int be_client(std::string const & address, int port);


int main(int argc, char * args [])
{
    if(argc == 2)
    {
        be_server(atoi(args[1]));
    }
    if(argc == 3)
    {
        be_client(args[1], atoi(args[2]));
    }
    return 0;
}


int be_server(int port)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    int flags =1;
    int r = setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));

    std::thread t(be_receiver, new_socket);
    be_sender(new_socket);
    t.join();
    return 0;
}

int be_client(std::string const & address, int port)
{
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    int flags =1;
    int r = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));

    std::thread t(be_receiver, sock);
    be_sender(sock);
    t.join();
    return 0;
}

void be_sender(int socket)
{
    while(true)
    {
        std::string msg {};
        std::getline(std::cin, msg);
        //std::cout <<"got message to send\n";
        if(msg == END_CONNECTION)
        {
            send(socket, END_CONNECTION.c_str(), END_CONNECTION.length(), 0);
            return;
        }
        if(msg.empty()) continue;
        send(socket, msg.c_str(), msg.length(), 0);
        //std::cout << ": sent message\n";
    }
}

void be_receiver(int socket)
{
    while(true)
    {
        char msg [buffer_size];
        int stored = 0;
        stored = read(socket, msg, buffer_size-1);
        //std::cout << "====" <<stored<< "\n\n";
        msg[stored]='\0';
        if(std::string(msg) == END_CONNECTION)
        {
            return;
        }
        if(stored == 0) continue;
        std::cout << "msg received from "<< msg << std::endl;
    }
}
