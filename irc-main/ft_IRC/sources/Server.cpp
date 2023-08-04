#include "../classes/Server.hpp"
#include <csignal>

bool Server::running = false;

Server::Server(unsigned int port, std::string password) :
    port(port), password(password), socketFd(0), hostName("IRCSERVER") {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGQUIT, signalHandler);
    std::cout << "Server Constructor" << std::endl;
}

Server::~Server() {
    while (!clients.empty()) {
        std::map<int, Client*>::iterator it = clients.begin();
        delete it->second;
        clients.erase(it);
    }
}

void Server::run() {
    running = true;
    std::cout << Green << "Server Running ..." << Reset<< std::endl;
    createSocket();
}

void Server::createSocket() {
    try{
        std::cout << Yellow << "Creating Server Socket" << Reset << std::endl;
        socketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFd == -1) {
            throw std::runtime_error("Failed to create socket.");
        }
        int reuseAddr = 1;
        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == -1) {
            close(socketFd);
            throw std::runtime_error("Failed to set SO_REUSEADDR option.");
        }
        //testing to see if changes
        if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
        std::cout << Red << "Error setting socket to non-blocking mode."<< Reset << std::endl;
        }
        bindSocket();
        listening();
    }
    catch(std::exception &e){
    }

}

void Server::bindSocket() {
    try {
        struct sockaddr_in actualAddr;
        socklen_t actualAddrLen = sizeof(actualAddr);
        // Initialize Struct socketAddr
        socketAddr.sin_family = AF_INET;
        socketAddr.sin_port = htons(port);
        socketAddr.sin_addr.s_addr = htons(INADDR_ANY);
        //Give server new name baseon ip
        // Bind the Socket
        int bindStatus = bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(socketAddr));
        if (bindStatus == -1) {
            throw std::runtime_error("Failed to bind socket.");
        }
        // Get Socket Name
        if (getsockname(socketFd, (struct sockaddr*)&actualAddr, &actualAddrLen) == -1) {
            throw std::runtime_error("Failed to get socket name.");
        }
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void Server::listening() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    listen(socketFd, 1000);
    std::cout << Green << "Listening Clients Connections ..." << Reset << std::endl;
    // Create a vector of pollfd structures to hold file descriptors and events
    pollfd serverPollFd;
    serverPollFd.fd = socketFd;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    pollFds.push_back(serverPollFd);
    while (running) {
        // Wait for activity on any of the monitored file descriptors
        int pollResult = poll(pollFds.data(), pollFds.size(), -1);
        if (pollResult == -1) {
            throw std::runtime_error("Error in poll system call.");
        }
        // Check if there is a new client connection   
        if (pollResult >= 0){
            if (pollFds[0].revents & (POLLIN)) {
                int clientSocket = accept(socketFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
                if (clientSocket == -1) {
                      std::cerr << "Error in poll system call. Continuing..." << std::endl;
                    continue;
                }
                // Add the new client socket to the vector of pollfd structures
                pollfd clientPollFd;
                clientPollFd.fd = clientSocket;
                clientPollFd.events = POLLIN;
                clientPollFd.revents = 0;
                pollFds.push_back(clientPollFd);
            }
        }
        try{
            handleCommunication(pollFds);
        }
        catch(...){
        }
    }
}

void Server::handleCommunication(std::vector<pollfd>& pollFds) {
    // Iterate through all the file descriptors in the vector
    char buffer[1024];

    for (int i = pollFds.size() - 1; i >= 1; --i) {
        if (pollFds[i].revents & POLLIN) {
            // Receive Data from Client
            bzero(buffer, sizeof(buffer)); // Clear buffer before receiving a new message
            std::string message;

            while (!strstr(buffer, "\r\n")) {
                int recvBytes = recv(pollFds[i].fd, buffer, sizeof(buffer), 0);
                if (recvBytes < 0) {
                    throw std::runtime_error("Failed to receive data from client.");
                } else if (recvBytes == 0) {
                    // Client closed the connection
                    close(pollFds[i].fd);
                    pollFds.erase(pollFds.begin() + i);
                    continue;
                }

                message += buffer;
            }

            if (strstr(message.c_str(), "CAP") && !strstr(message.c_str(), "USER")) {
                // Wait for the USER message
                while (!strstr(message.c_str(), "USER")) {
                    bzero(buffer, sizeof(buffer));
                    int recvBytes = recv(pollFds[i].fd, buffer, sizeof(buffer), 0);
                    if (recvBytes < 0) {
                        throw std::runtime_error("Failed to receive data from client.");
                    } else if (recvBytes == 0) {
                        // Client closed the connection
                        close(pollFds[i].fd);
                        pollFds.erase(pollFds.begin() + i);
                        continue;
                    }
                    message += buffer;
                }
            }
            handleClientMessage(message, pollFds[i].fd);
        }
    }
}



void Server::handleClientMessage(std::string message, int& clientFd){
    std::cout << Magenta << "______________________________________________________________" << Reset << std::endl;
    std::cout << Green << "=> Received Data From Client: " << Reset << message << std::endl;
    std::string response;
    CommandHandler::handleCommand(*this, clientFd, message, response);
    if (!response.empty()){
        CommandHandler::sendExtraMessage(clientFd, response);
    }
}

void Server::printData(){
    for (std::vector<Channel>::iterator it = channels.begin();it != channels.end(); ++it){
        std::cout << "Channel: " << it->getChannelName() << std::endl;
        std::cout << "i Mode = " << it->getIMode() << std::endl;
        std::cout << "t Mode = " << it->getTMode() << std::endl;
        std::cout << "k Mode = " << it->getKMode() << std::endl;
        std::cout << "o Mode = " << it->getOMode() << std::endl;
        for(std::map<std::string, Client>::iterator iter = (*it).getInvitedClients().begin(); iter != (*it).getInvitedClients().end();++iter){ 
           //isOperator(it->opClientFd, iter->second.socketFd);
            if (it->getOpClientFd()[0] == iter->second.getSocketFd()){
                std::cout << "Client op: " << iter->second.getNickName() << std::endl;
            }
            else{
                std::cout << "Client: " << iter->second.getNickName() << std::endl;
            }
        }
        if (!it->getPassword().empty()){
             std::cout << "Channel Password: " << it->getPassword() << std::endl;
        }
        std::cout << "Topic " << it->getTopic() << std::endl;
        std::cout << "_______________________________" << std::endl;
    }
}

bool Server::completedMessage(std::string &message){
    std::size_t terminationPos = message.find("\r\n");
    if (terminationPos != std::string::npos) {
        std::cout << "Completed Message" << std::endl;
        return true;
    }
    std::cout << "Imcompleted Message" << std::endl;
    return false;
}

//getters

std::vector<pollfd> &Server::getPollFds()
{
    return (this->pollFds);
}

unsigned int Server::getPort()
{
    return (this->port);
}

std::string Server::getPassword()
{
    return (this->password);
}

int Server::getSocketFd()
{
    return (this->socketFd);
}

std::string Server::getHostName()
{
    return (this->hostName);
}

std::map<int, Client*> &Server::getClients()
{
    return (this->clients);
}

std::vector<Channel> &Server::getChannels()
{
    return (this->channels);
}

void Server::signalHandler(int signal){
    if (signal == SIGINT) {
      std::cout << Green << "Thanks for using our Server!" << Reset << std::endl;
      std::cout << Red << "Server Closed" << Reset << std::endl;
      running = false;
    }
}