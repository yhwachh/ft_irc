#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/poll.h>
#include <string>
#include <sstream>
#include <fstream>
#include <string.h>
#include "Channel.hpp"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../includes/colors.hpp"
#include "../classes/CommandHandler.hpp"
class Server{
	public :
		// Constructors
		Server(unsigned int port, std::string password);
		~Server();
		// Methodes
		void run();
		//getters
		std::map<int, Client*> &getClients();
		std::vector<pollfd> &getPollFds();
		unsigned int getPort();
		std::string getPassword();
		int getSocketFd();
		std::string getHostName();
		std::vector<Channel> &getChannels();
		void printData();
	private :
 	 static void signalHandler(int signal);
		static bool running;
		// Properties
		unsigned int port;
		std::string password;
		int socketFd;
		std::string hostName;
		std::vector<pollfd> pollFds;
		std::map<int, Client*> clients;
		std::vector<Channel> channels;
		struct sockaddr_in socketAddr;
		// Methodes
		void createSocket();
		void bindSocket();
		void listening();
		std::string readFile(const std::string& filePath);
		void handleCommunication(std::vector<pollfd>& pollFds);
		void handleClientMessage(std::string message, int& clientFd);
		bool completedMessage(std::string &message);
};
