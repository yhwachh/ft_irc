#pragma once
#include <iostream>
#include "../classes/Server.hpp"
#include "../classes/Client.hpp"
#define NOTICE_MSG(hostName, target, message) (":" + hostName + " NOTICE " + target + " :" + message + "\r\n")
#define ERROR_BADPASSWORD() ("ERROR :Connection refused: Password Does Not Match\r\n")
#define ERROR_NICKTAKEN() ("ERROR :Connection refused: NickName already taken\r\n");
#define WHOIS_MSG(hostName, nickName) (":" + hostName + " 311 " + nickName + " " + hostName + " " + nickName + " *\r\n")
#define CAP_END() (":localhost CAP * ACK :END\r\n")
#define NICKCHANGE_MSG(serverHostName, nickName, newNickName) (":" + nickName + "!" + nickName + " NICK :" + newNickName + "\r\n")
#define WELCOME_SERVER(hostName, nickName, message, message2) (":" + hostName + " 001 " + nickName + " :" + message  + " " + nickName + " " + message2 + "\r\n")
#define CANTJOIN(servername,client,channelname) (":" + servername + " 437 " + client)
#define PONG_MSG(serverHostName) ("PONG " + serverHostName + "\r\n")
#define MODE_MEG(serverHostName, channelName, mode) (":" + serverHostName + " MODE " + channelName + " " + mode + "\r\n")
#define AS(nick, name, symbol, chan, prefix) (std::string("353 ") + nick + " " + symbol + " " + chan + " " + prefix + name + "\r\n")

#define NOTOPERATOR(SERVERNAME, CHANNELNAME) (":" + SERVERNAME + " 482 " + SERVERNAME + " " + CHANNELNAME + " :You're not a channel operator\r\n")
class Server;

class CommandHandler{
    public:
        static void handleCommand(Server& server, int &clientFd, std::string message, std::string &response);
		static bool isInChannel(int clientFd, Channel &channel);
        static void sendExtraMessage(int fd, std::string message);
    private:
        static int getMessageType(std::string message);
        static std::string handleCapabilityNegotiation(const std::string& message);
        static std::string readFile(const std::string& filePath);
        static bool parseChannelName(const std::string& message, std::string& channelName);
        static bool parseMessage(std::string message, std::string &channelName, std::string &messageContent);
        static bool parseNickNameMessage(const std::string& message, std::string& nickName, std::string& username, std::string& hostName, std::string&serverHostName, std::string& realName, std::string &password);
        static bool doesChannelExist(std::vector<Channel> &channels, std::string channelName);
        static bool parseKickMessage(std::string message, std::string &channelName, std::string &nickName);
		static bool isOperator(std::vector<int> operators, int clientFd);
        static std::string extractMode(std::string &message, std::string &channelName);
        static Channel& getChannelByName(std::vector<Channel> &channels, std::string channelName);
        static Client& getClientByName(std::map<int, Client*> &clients, std::string clientName);
        static std::string extractChannelPassword(std:: string &message);
        static std::string extractInvitedClient(std:: string &message);
        static std::string extractTopic(std::string message);
        static void messageAllChannelClients(Channel channel, int client, std::string message);
        static void extractNewOp(std::string message, std::string &nickName, std::string &channelName);
        static void removeElementByFd(std::vector<int>& vec, int id);
        static void deleteClient(std::map<int, Client*>& clients, int clientFd);
       static void deleteClientFromAllChannels(std::vector<Channel>& channels, int clientFd, std::string nickName);
       static void updateClient(std::vector<Channel>& channels, int clientFd, std::string nickName, std::string newName);
       static bool isInvitedToChannel(std::vector<int> &invitedVector, int clientFd);
        static bool isPasswordGood(std::string message, std::string password);
        static std::string getNameOfClients(std::map<std::string, Client> clients);
        static bool isRequest(std::string message);
        static bool instanceExist(std::map<std::string, Client> clientsOnChannel, std::string nickName);
        static std::string getAllChannelsNames(std::vector<Channel> channels);
        static unsigned int extractMaxClients(std::string message);
        static bool isValidMode(std::string mode);
};

