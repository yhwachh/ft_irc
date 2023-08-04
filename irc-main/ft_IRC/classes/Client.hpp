#pragma once
#include <iostream>
#include <unistd.h>
#include "../includes/colors.hpp"
class Client{
	public :
		Client(int socket, std::string nickName, std::string userName, std::string hostName, std::string serverHostName, std::string realName);
		~Client();
		// Getters
		int getSocketFd();
		std::string getNickName();
		std::string getUserName();
		std::string getHostName();
		std::string getServerHostName();
		std::string getRealName();
		std::string getUserHost();
		std::string getLocalHost();
		//setter
		void setNickName(std::string newNickName);
		void setUserName(std::string newUserName);
		void setHostName(std::string newUserName);
	private :
        std::string userHost;
        std::string localHost;
		int socketFd;
		std::string nickName;
		std::string userName;
		std::string hostName;
		std::string serverHostName;
		std::string realName;
};