#include "../classes/Channel.hpp"
#include "../classes/Server.hpp"
Channel::Channel(std::string name, Client op) :
channelName(name), iMode(false), tMode(false), kMode(false), oMode(false), lMode(false), maxClients(200){
	std::cout << Yellow << "Channel " << channelName << " created!" << Reset << std::endl;
	(void)op;
}
Channel::~Channel(){
	std::cout << Red<< "Channel DESTRUCTED" << Reset<< std::endl;
}

std::string Channel::getChannelName()
{
	return this->channelName;
}

std::vector<int>& Channel::getOpClientFd()
{
	return this->opClientFd;
}

std::map<std::string, Client>&Channel::getInvitedClients()
{
	return this->invitedClients;
}

//getter

bool Channel::getIMode(){
	return this->iMode;
}
bool Channel::getTMode(){
	return this->tMode;
}
bool Channel::getKMode(){
	return this->kMode;
}
bool Channel::getOMode(){
	return this->oMode;
}

bool Channel::getLMode(){
	return this->lMode;
}

std::string &Channel::getPassword(){
	return password;
}

std::string Channel::getTopic(){
	return topic;
}

std::vector<int>&Channel::getInvitedClientsToChannel(){
	return invitedClientsToChannel;
}

unsigned int Channel::getMaxClients(){
	return this->maxClients;
}

// Setters

void Channel::setIMode(bool state){
	this->iMode = state;
}

void Channel::setTMode(bool state){
	this->tMode = state;
}

void Channel::setPassword(std::string newPassword){
	this->password = newPassword;
}

void Channel::setTopic(std::string newTopic){
	this->topic = newTopic;
}

void Channel::setLMode(bool state){
	this->lMode = state;
}

void Channel::setMaxClients(unsigned int max){
	this->maxClients = max;
}