#include "../classes/Client.hpp"

Client::Client(int socket, std::string nickName, std::string userName, std::string hostName, std::string serverHostName, std::string realName)
: socketFd(socket), nickName(nickName), userName(userName), hostName(hostName), serverHostName(serverHostName), realName(realName){
	std::cout << Yellow << "Client Created" << Reset << std::endl;
}
Client::~Client(){
	std::cout << Red<< "CLIENT DESTRUCTED" << Reset<< std::endl;
	//close(socketFd);
}

int Client::getSocketFd()
{
	return (this->socketFd);
}

std::string Client::getNickName()
{
	return (this->nickName);
}

std::string Client::getUserName()
{
	return (this->userName);
}

std::string Client::getHostName()
{
	return (this->hostName);
}

std::string Client::getServerHostName()
{
	return (this->serverHostName);
}

std::string Client::getRealName()
{
	return (this->realName);
}

std::string Client::getUserHost()
{
	return (this->userHost);
}

std::string Client::getLocalHost()
{
	return (this->localHost);
}

void Client::setNickName(std::string newNickName){
	this->nickName = newNickName;
}

void Client::setUserName(std::string newUserName){
	this->userName = newUserName;
}

void Client::setHostName(std::string newUserName){
	this->hostName = newUserName;
}
