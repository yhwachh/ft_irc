#pragma once
#include <iostream>
#include "Client.hpp"
#include <map>
#include <vector>
#include "../includes/colors.hpp"

class Client;
class Channel{
	private :
		std::string channelName;
		std::map<std::string, Client> invitedClients;
		std::vector<int>opClientFd;
		std::vector<int>invitedClientsToChannel;
		bool iMode;
		bool tMode;
		bool kMode;
		bool oMode;
		bool lMode;
		unsigned int maxClients;
		std::string password;
		std::string topic;
	public :
		Channel(std::string name, Client op);
		~Channel();
		// Getters
		std::string getChannelName();
		std::map<std::string, Client> &getInvitedClients();
		std::vector<int>&getOpClientFd();
		std::vector<int>&getInvitedClientsToChannel();
		bool getIMode();
		bool getTMode();
		bool getKMode();
		bool getOMode();
		bool getLMode();
		unsigned int getMaxClients();
		std::string &getPassword();
		std::string getTopic();
		// Setters
		void setIMode(bool state);
		void setTMode(bool state);
		void setLMode(bool state);
		void setPassword(std::string newPassword);
		void setTopic(std::string newTopic);
		void setMaxClients(unsigned int max);
};