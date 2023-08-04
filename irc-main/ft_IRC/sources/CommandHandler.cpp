#include "../classes/CommandHandler.hpp"

void CommandHandler::handleCommand(Server& server, int &clientFd, std::string message, std::string &response){
    int messageType = getMessageType(message);
    int sendStatus;
    Client* client = NULL;
    std::string realName;
    std::string serverHostName;
    std::string hostName;
    std::string userName;
    std::string nickName;
    std::string password;
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string>words;
    std::string sendernickName;
    std::string messageContent;
    std::string channelName;
	std::string test;
    std::string mode;
    std::string topic;
    std::string response2;
    std::vector<Channel>::iterator it;
    std::map<std::string, Client>::iterator iter;
	std::vector<int> op;
    switch (messageType){
        case 0 :    //CommandCAP
            parseNickNameMessage(message, nickName, userName, hostName, serverHostName, realName, password);
            if (password.empty() || server.getPassword() != password){
                response = ERROR_BADPASSWORD();
                return;
            }
            for (std::map<int, Client*>::iterator it = server.getClients().begin(); it != server.getClients().end(); ++it){
                if (it->second->getNickName() == nickName){
                    response = ERROR_NICKTAKEN();
                    return;
                }
            }
            client = new Client(clientFd, nickName, userName, hostName, serverHostName, realName);
            server.getClients().insert(std::pair<int, Client*>(clientFd, client));
            response = WELCOME_SERVER(server.getHostName(),server.getClients()[clientFd]->getNickName(), readFile("wel.txt"), readFile("come.txt"));
            break;
        case 1 :    //CommandNICK/USER
            parseNickNameMessage(message, nickName, userName, hostName, serverHostName, realName, password);
            if (password.empty() || server.getPassword() != password){
                response = ERROR_BADPASSWORD();
                return;
            }
            for (std::map<int, Client*>::iterator it = server.getClients().begin(); it != server.getClients().end(); ++it){
                if (it->second->getNickName() == nickName){
                    response = ERROR_NICKTAKEN();
                    return;
                }
            }
            client = new Client(clientFd, nickName, userName, hostName, serverHostName, realName);
            server.getClients().insert(std::pair<int, Client*>(clientFd, client));
            response = WELCOME_SERVER(server.getHostName(),server.getClients()[clientFd]->getNickName(), readFile("wel.txt"), readFile("come.txt"));
            break;
        case 2 :    // Command NICK
            while (iss >> word) {
                words.push_back(word);
            }
            nickName = server.getClients()[clientFd]->getNickName();
            server.getClients()[clientFd]->setNickName(words[1]);
            server.getClients()[clientFd]->setUserName(words[1]);
            server.getClients()[clientFd]->setHostName(words[1]);
            updateClient(server.getChannels(), clientFd, nickName, words[1]);
            response = NICKCHANGE_MSG(server.getHostName(),nickName, server.getClients()[clientFd]->getNickName());
            break;
        case 3 : // Command WHOIS
            response = WHOIS_MSG(server.getHostName(),server.getClients()[clientFd]->getNickName());
           break;
        case 4 : // Command ENDCAP
            response = CAP_END();
            break;
        case 5 : // Command PING
            response = PONG_MSG(server.getHostName());
            server.printData();
            break;
        case 6 : // Command Mode
            try{
                mode = extractMode(message, channelName);
                if (mode == "b"){
                    response = ":" + server.getHostName() + " MODE " + channelName + " +b\r\n";
                    break;
                }
                if (!isOperator(getChannelByName(server.getChannels(), channelName).getOpClientFd(),clientFd) && !isRequest(message)){
                    response = ":" + server.getHostName() + " 482 " + server.getClients()[clientFd]->getNickName() + " " + channelName + " :You're not a channel operator\r\n";
                    break;
                }
                if (mode == "+i"){
                getChannelByName(server.getChannels(), channelName).setIMode(true);
                }
                else if (mode == "-i"){
                    getChannelByName(server.getChannels(), channelName).setIMode(false);
                }
                else if (mode == "+t"){
                    getChannelByName(server.getChannels(), channelName).setTMode(true);
                }
                else if (mode == "-t"){
                    getChannelByName(server.getChannels(), channelName).setTMode(false);
                }
                else if (mode == "+k"){
                    getChannelByName(server.getChannels(), channelName).setPassword(extractChannelPassword(message));
                }
                else if (mode == "-k"){
                    getChannelByName(server.getChannels(), channelName).getPassword().clear();
                }
                else if (mode == "+o"){
                    extractNewOp(message, nickName, channelName);
                    getChannelByName(server.getChannels(), channelName).getOpClientFd().push_back(getClientByName(server.getClients(), nickName).getSocketFd());
                    response = ":" + server.getHostName() + " MODE " + channelName + " +o " + nickName + "\n";
                    sendExtraMessage(getClientByName(server.getClients(), nickName).getSocketFd(), NOTICE_MSG(server.getHostName(), channelName, "You are now op"));
                    break;
                }
                else if (mode == "-o"){
                    extractNewOp(message, nickName, channelName);
                    removeElementByFd(getChannelByName(server.getChannels(), channelName).getOpClientFd(), getClientByName(server.getClients(), nickName).getSocketFd());
                    sendExtraMessage(getClientByName(server.getClients(), nickName).getSocketFd(), NOTICE_MSG(server.getHostName(), channelName, "You are no longer op"));
                }
                else if (mode == "-l"){
                    getChannelByName(server.getChannels(), channelName).setMaxClients(200);
                    getChannelByName(server.getChannels(), channelName).setLMode(false);
                }
                else if (mode == "+l"){
                    getChannelByName(server.getChannels(), channelName).setMaxClients(extractMaxClients(message));
                    getChannelByName(server.getChannels(), channelName).setLMode(true);
                }
                else if (mode == "MODE_REQUEST"){
                    response = ":" + server.getHostName() + " 324 " + server.getClients()[clientFd]->getNickName() + " "  + channelName + " +\r\n";
                    break;
                }
                if (isValidMode(mode)){
                    response = MODE_MEG(server.getHostName(), channelName, mode);
                }
                else{
                    response = "UNKNOWN";
                }
            }
            catch(...){
                response = ":" + server.getHostName() + " 502 " + server.getClients()[clientFd]->getNickName() + " :You are now online as " + server.getClients()[clientFd]->getNickName() + "\r\n";
            }
            break;
        case 7 :
            response = handleCapabilityNegotiation(message);
            break;
        case 8 : // Command Join
            parseChannelName(message, channelName);
            // check if channel exit and create it;
            if (!doesChannelExist(server.getChannels(), channelName)){
                std::cout << Cyan << "###" << server.getClients()[clientFd]->getNickName() << " created :" << channelName << "###" << Reset << std::endl;
                server.getChannels().push_back(Channel(channelName, *server.getClients()[clientFd]));
                // Search the channel end atribute the op to the client who created the channel
                for(std::vector<Channel>::iterator it = server.getChannels().begin(); it != server.getChannels().end(); ++it){
                    if (it->getChannelName() == channelName){
                        it->getOpClientFd().push_back(clientFd);
                    }
                }
            }
            //check if someone
            if (((getChannelByName(server.getChannels(), channelName).getIMode() == false) && (getChannelByName(server.getChannels(), channelName).getPassword().empty())) ||
            ((getChannelByName(server.getChannels(), channelName).getIMode() == true) && (getChannelByName(server.getChannels(), channelName).getPassword().empty()) && (isInvitedToChannel(getChannelByName(server.getChannels(), channelName).getInvitedClientsToChannel(), clientFd))) || 
            (((getChannelByName(server.getChannels(), channelName).getIMode() == false) && (!getChannelByName(server.getChannels(), channelName).getPassword().empty())) && (isPasswordGood(message, getChannelByName(server.getChannels(), channelName).getPassword()))) || 
            ((getChannelByName(server.getChannels(), channelName).getIMode() == true) && (!getChannelByName(server.getChannels(), channelName).getPassword().empty()) && (isPasswordGood(message, getChannelByName(server.getChannels(), channelName).getPassword())) && (isInvitedToChannel(getChannelByName(server.getChannels(), channelName).getInvitedClientsToChannel(), clientFd)))) {
                if (getChannelByName(server.getChannels(), channelName).getLMode() == true && 
                getChannelByName(server.getChannels(), channelName).getInvitedClients().size() >= getChannelByName(server.getChannels(), channelName).getMaxClients()){
                    response = ":" + server.getHostName() + " 471 " + nickName + " " + channelName + " :(+l) - max clients reached\r\n";
                    return;
                }
                for(it = server.getChannels().begin(); it != server.getChannels().end();++it){
	    	    	if ((*it).getChannelName() == channelName){
                        //does instance already exist in channel
                        if (instanceExist(getChannelByName(server.getChannels(), channelName).getInvitedClients(), server.getClients()[clientFd]->getNickName())){
                            response = ":" + server.getHostName() + " 433 " + server.getClients()[clientFd]->getNickName() + " " + channelName + " : has already this nickname in use!\r\n";
                            return;
                        }
	    	    		(*it).getInvitedClients().insert(std::make_pair<std::string, Client>(server.getClients()[clientFd]->getNickName(), *server.getClients()[clientFd]));
                        if (isInvitedToChannel(getChannelByName(server.getChannels(), channelName).getInvitedClientsToChannel(), clientFd)) {
                            std::vector<int>& invitedClients = (*it).getInvitedClientsToChannel();
                            std::vector<int>::iterator it = invitedClients.begin();
                        
                            while (it != invitedClients.end()) {
                                if (*it == clientFd) {
                                    it = invitedClients.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        }
	    	    	}
	    	    }
                response2 = ":" + server.getClients()[clientFd]->getNickName() + "!~" + server.getClients()[clientFd]->getUserName() + "@" + server.getHostName() + " JOIN " + channelName + "\r\n";
                sendStatus = send (clientFd, response2.c_str(), response2.length(), 0);
                if (sendStatus == -1){
                    std::cout << "error sending nick" << std::endl;
                }
                else if (sendStatus > 0){
	    	        std::cout << Blue << "=> Server Sended Response with: " << Reset << response2 << std::endl;
                }
                std::cout << Cyan << "###" << server.getClients()[clientFd]->getNickName() << " joined :" << channelName << "###" << Reset << std::endl;
                response2 = NOTICE_MSG(server.getHostName(), channelName, server.getClients()[clientFd]->getNickName() + " as joined the channel\r\n");
                messageAllChannelClients(getChannelByName(server.getChannels(), channelName), clientFd, response2);
                response = ":" + server.getHostName() + " 353 " + server.getClients()[clientFd]->getNickName() + " = " + channelName + " :" + getNameOfClients(getChannelByName(server.getChannels(), channelName).getInvitedClients()) + "\r\n";
                sendExtraMessage(clientFd, response);
                response = ":" + server.getHostName() + " 366 " + server.getClients()[clientFd]->getNickName() + " " + channelName + "\r\n";
                sendExtraMessage(clientFd, response);
                response = ":" + server.getHostName() + " 332 " + server.getClients()[clientFd]->getNickName() + " " + channelName + " :" + getChannelByName(server.getChannels(), channelName).getTopic() + "\r\n";
                sendExtraMessage(clientFd, response);
                response = NOTICE_MSG(server.getHostName(), channelName, "Welcome To the Channel " + channelName + "\r\n");
            }
            else{
                if (getChannelByName(server.getChannels(), channelName).getIMode()){
                    if (!getChannelByName(server.getChannels(), channelName).getPassword().empty() && isInvitedToChannel(getChannelByName(server.getChannels(), channelName).getInvitedClientsToChannel(),clientFd)){
                        response = ":" + server.getHostName() + " 475 " + nickName + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
                    }
                    else{
                        response = ":" + server.getHostName() + " 473 " + server.getClients()[clientFd]->getNickName() + " " + channelName + " :Cannot join channel without invitation\r\n";
                    }
                }
                else{
                    response = ":" + server.getHostName() + " 475 " + nickName + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
                }
            }
            break;
        case 9 : // Command Part
        //send this message to all client after deleting this client of the channel
            parseChannelName(message, channelName);
            nickName = server.getClients()[clientFd]->getNickName();
            response = ":" + nickName + " PART " + channelName + "\r\n";
            for (it = server.getChannels().begin();it != server.getChannels().end(); ++it){
                if ((*it).getChannelName() == channelName){
                    // delete the fd from the op
                    if (isOperator((*it).getOpClientFd(), clientFd)){
						for (std::vector<int>::iterator iter = it->getOpClientFd().begin(); iter != it->getOpClientFd().end(); ++iter) {
                            if (*iter == clientFd) {
                              it->getOpClientFd().erase(iter);
                              break;  // Break out of the loop after erasing the element
                            }
                        }
					}
                    (*it).getInvitedClients().erase(nickName);
                    //send message to everyone saying he is left
                    if (isInChannel(clientFd,(*it)))
                    {
                        for(iter = (*it).getInvitedClients().begin(); iter != (*it).getInvitedClients().end();++iter){
                            int sendStatus = send(iter->second.getSocketFd(),response.c_str(),response.length(), 0);
                            if (sendStatus <= 0){
                                std::cout << Red << "Failed to send message" << Reset << std::endl;
                            }
                            else if (sendStatus > 0){
                                std::cout << Cyan<< server.getClients()[clientFd]->getNickName() << " from channel -> " << channelName << " sended: " << messageContent << "to " << iter->second.getNickName() << Reset << std::endl;
                            }
                        }
                    }
                    if ((*it).getInvitedClients().empty()) {
                        it = server.getChannels().erase(it);
                        --it;
                    }
					break;
                }
            }
            try{
                // if op use part the op is the next client
                if (getChannelByName(server.getChannels(), channelName).getOpClientFd().empty()){
				    getChannelByName(server.getChannels(), channelName).getOpClientFd().push_back(getChannelByName(server.getChannels(), channelName).getInvitedClients().begin()->second.getSocketFd());
			    }
            }
            catch(std::exception &e){
                std::cout << e.what() << std::endl;
            }
            response = ":" + nickName + "!" + nickName + "@" + server.getHostName() + " Part " + channelName + "\r\n";
            break;
        case 10 : // Command PRIVM
            parseMessage(message, channelName, messageContent);
            for (it = server.getChannels().begin();it != server.getChannels().end(); ++it){
                if ((*it).getChannelName() == channelName){
                    if (isInChannel(clientFd, (*it))){
                        for(iter = (*it).getInvitedClients().begin(); iter != (*it).getInvitedClients().end();++iter){
                            if (iter->second.getSocketFd() == server.getClients()[clientFd]->getSocketFd()){
                                //skip
                            }
                            else{
	    				        response = ":" + server.getClients()[clientFd]->getNickName() + "!~" + server.getClients()[clientFd]->getUserName() + " PRIVMSG " + channelName + " :" + messageContent + "\r\n";
                                sendExtraMessage(iter->second.getSocketFd(), response);
                            }
                        }
                    }
                }
            }
            response.clear();
            break;
        case 11 :
            response = ":QUIT :leaving\r\n";
            try {
                if (server.getClients()[clientFd]){
                    deleteClientFromAllChannels(server.getChannels(),clientFd, server.getClients()[clientFd]->getNickName());
                    std::cout << "Deleted" << std::endl;
                }
            }
            catch(std::exception &e){
                std::cout << e.what() << std::endl;
            }
            deleteClient(server.getClients(), clientFd);
            break;
        case 12 : // Command Kick / ps debugg if he tries to kick himself
            parseKickMessage(message, channelName ,nickName);
            response = NOTICE_MSG(channelName, channelName, " You have been KICKED from the Channel </part> to quit the Channel!");
            for (it = server.getChannels().begin();it != server.getChannels().end(); ++it){
                if ((*it).getChannelName() == channelName){
					if (isOperator((*it).getOpClientFd() , clientFd)){
                        if (isOperator(getChannelByName(server.getChannels(),channelName).getOpClientFd(), getClientByName(server.getClients(), nickName).getSocketFd())){
                            response = NOTICE_MSG(server.getHostName(), channelName, "The client you try to kick is Op");
                            sendExtraMessage(clientFd, response);
                            response.clear();
                            break;
                        }
                        try {
                            getClientByName(server.getClients(),nickName);
                        }
                        catch(...){
                            response = "dont exit";
                            break;
                        }
                    	for(iter = (*it).getInvitedClients().begin(); iter != (*it).getInvitedClients().end();++iter){
                            if (iter->second.getNickName() == nickName){
                                sendExtraMessage(iter->second.getSocketFd(), response);
                                response = ":" + nickName + "!" + nickName + "@" + server.getHostName() + " Part " + channelName + "\r\n";
                                break;
                            }
                    	}
                        (*it).getInvitedClients().erase(nickName);
                        //delete channel if there is no one
                    	if ((*it).getInvitedClients().empty()) {
                    	    it = server.getChannels().erase(it);
                    	    --it;
                    	}
                        break;
					}
					else{
						response = NOTOPERATOR(server.getHostName(), channelName);
                        break;
					}
                }
            }
            // response = ":" + nickName + "!" + nickName + "@" + server.getHostName() + " Part " + channelName + "\r\n";
            break;
        case 13 : // Command Invite
            try{
                while (iss >> word) {
                words.push_back(word);
                }
                nickName = words[1];
                //std::cout << channelName << std::endl;
                parseChannelName(message, channelName);
                if (getChannelByName(server.getChannels(),channelName).getIMode() == true){
                    getChannelByName(server.getChannels(),channelName).getInvitedClientsToChannel().push_back(getClientByName(server.getClients(),extractInvitedClient(message)).getSocketFd());
                    getChannelByName(server.getChannels(), channelName).getInvitedClientsToChannel().push_back(getClientByName(server.getClients(), nickName).getSocketFd());
                    response = ":" + server.getHostName() + " 341 " + server.getClients()[clientFd]->getNickName() + " " + channelName + " :Inviting " + nickName + " to " + channelName + "\r\n";
                    sendExtraMessage(getClientByName(server.getClients(), nickName).getSocketFd(), NOTICE_MSG(server.getHostName(), nickName, "You have been Invited to join Channel " + channelName));
                }
                else{
                    response = "NOTICE " + channelName + " :You need to set mode +i to invite someone to the channel\r\n";
                }
            }
            catch(std::exception &e){
                std::cout << e.what() << std::endl;
                response = ":" + server.getHostName() + " 401 " + server.getClients()[clientFd]->getNickName() + " :No such nick\r\n";
            }
            break;
        case 14 : // Command Topic
            try{
                parseChannelName(message, channelName);
                if (isOperator(getChannelByName(server.getChannels(), channelName).getOpClientFd(), clientFd) || getChannelByName(server.getChannels(), channelName).getTMode() == false){
                    getChannelByName(server.getChannels(), channelName).setTopic(extractTopic(message));
                    response = ":" + server.getHostName() + " 332 " + server.getClients()[clientFd]->getNickName() + " " + channelName + getChannelByName(server.getChannels(), channelName).getTopic() + "\r\n";
                    messageAllChannelClients(getChannelByName(server.getChannels(), channelName), clientFd, response);
                }
                else{
                    response = ":" + server.getHostName() + " 482 " + server.getHostName() + " " + channelName + " :You're not a channel operator\r\n";;
                }
            }
            catch(std::exception &e){
                std::cout << e.what() << std::endl;
            }
            break;
        case 15 : //WHO
            break;
        case 16 :
            response = getAllChannelsNames(server.getChannels());
            break;
        default :
            response = "Unknown command.\r\n";
            break;
    }
}

bool CommandHandler::parseChannelName(const std::string& message, std::string& channelName)
{
    // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    // Extract the nickName and username
    if (words[0] == "INVITE"){
        channelName = words[2];
    }
    else{
        channelName = words[1];
    }
    return true;
}

bool CommandHandler::parseMessage(std::string message, std::string &channelName, std::string &messageContent){
     // Find the position of the channel
   std::size_t channelStartPos = message.find("#");
    std::size_t channelEndPos = message.find(" ", channelStartPos);
    channelName = message.substr(channelStartPos, channelEndPos - channelStartPos);
	// std::cout << "testomg " << channelName << std::endl;
    // Find the position of the message
    std::size_t messageStartPos = message.find(":", channelEndPos);
    messageContent = message.substr(messageStartPos + 1);
    return true;
}
//Reads a file and output a string
std::string CommandHandler::readFile(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    std::stringstream buffer;
    
    if (file) {
        buffer << file.rdbuf();
        file.close();
    }
    
    return buffer.str();
}

bool CommandHandler::parseNickNameMessage(const std::string& message, std::string& nickName, std::string& username, std::string& hostName, std::string&serverHostName, std::string& realName, std::string &password){
    // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "PASS"){
            password = words[i + 1];
        }
        else if (words[i] == "NICK"){
            nickName = words[i + 1];
        }
        else if (words[i] == "USER"){
            username = words[i + 1];
            hostName = words[i + 2];
            serverHostName = words[i + 3];
            realName = words[i + 4];
            return true;
        }
    }
    return false;
}

int CommandHandler::getMessageType(std::string message){
    if (message.find("CAP LS") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 0;
    }
    else if (message.find("NICK ") != std::string::npos && message.find("USER ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 1;
    }
    else if (message.find("NICK ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 2;
    }
    else if (message.find("WHOIS ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 3;
    }
    else if (message.find("CAP END") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 4;
    }
    else if (message.find("PING ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 5;
    }
    else if (message.find("MODE ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 6;
    }
    else if(message.find("CAP ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 7;
    }
    else if (message.find("JOIN ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 8;
    }
    else if (message.find("PART ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 9;
    }
    else if (message.find("PRIVMSG ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 10;
    }
    else if (message.find("QUIT ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 11;
    }
    else if (message.find("KICK ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 12;
    }
    else if (message.find("INVITE ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 13;
    }
    else if (message.find("TOPIC ") != std::string::npos && message.find("\r\n") != std::string::npos){
        return 14;
    }
    else if (message.find("WHO ") != std::string::npos){
        return 15;
    }
    else if (message.find("LIST ") != std::string::npos){
        return 16;
    }
    return -1;
}

std::string CommandHandler::handleCapabilityNegotiation(const std::string& message) {
    // Check if the received message is CAP LS
    if (message.substr(0, 8) != "CAP LS\r\n")
        return "Unknown command.";
    // Prepare the list of supported capabilities
    std::string capabilities = "CAP LIST";
    // Construct the response
    std::string response = "CAP * LS :" + capabilities + "\r\n";
    return response;
}

bool CommandHandler::doesChannelExist(std::vector<Channel> &channels, std::string channelName) {
    for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->getChannelName() == channelName) {
            return true;
        }
    }
    return false;
}

bool CommandHandler::parseKickMessage(std::string message, std::string &channelName, std::string &nickName){
     // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "KICK"){
            channelName = words[i + 1];
            nickName = words[i + 2];
        }
    }
    return false;
}

bool CommandHandler::isInChannel(int clientFd, Channel &channel) {
    for (std::map<std::string, Client>::iterator it = channel.getInvitedClients().begin(); it != channel.getInvitedClients().end(); ++it) {
        if (it->second.getSocketFd() == clientFd) {
            return true;
        }
    }
    return false;
}

bool CommandHandler::isOperator(std::vector<int> operators, int clientFd){
	for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); ++it){
		if (*it == clientFd){
			return true;
		}
	}
	return false;
}

std::string CommandHandler::extractMode(std::string &message, std::string &channelName){
     // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "MODE"){
            channelName = words[i + 1];
            if (words.size() == 2){
                return "MODE_REQUEST";
            }
            else{
                return words[i + 2];
            }
        }
    }
    return NULL;
}

std::string CommandHandler::extractChannelPassword(std:: string &message){
     // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "MODE"){
            return words[i + 3];
        }
    }
    return NULL;
}

Channel& CommandHandler::getChannelByName(std::vector<Channel> &channels, std::string channelName){
    for (std::vector<Channel>::iterator it = channels.begin();it != channels.end(); ++it){
        if (it->getChannelName() == channelName){
            return *it;
        }
    }
    throw std::runtime_error("No Channel found");
}
Client& CommandHandler::getClientByName(std::map<int, Client*> &clients, std::string clientName){
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it){
        if (it->second->getNickName() == clientName){
            return *it->second;
        }
    }
    throw std::runtime_error("No Client found");
}

std::string CommandHandler::extractInvitedClient(std:: string &message){
      // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "INVITE"){
            return words[i + 1];
        }
    }
    return NULL;
}

std::string CommandHandler::extractTopic(std::string message){
          // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    std::string topic;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 2; i < words.size(); i++){
        topic += " ";
        topic += words[i];
    }
    return topic;
}

void CommandHandler::messageAllChannelClients(Channel channel, int client, std::string message){
    for (std::map<std::string, Client>::iterator it = channel.getInvitedClients().begin(); it != channel.getInvitedClients().end(); ++it){
        if (it->second.getSocketFd() != client){
            send(it->second.getSocketFd(), message.c_str(), message.length(), 0);
        }
    }
}

void CommandHandler::extractNewOp(std::string message, std::string &nickName, std::string &channelName){
    // Split the message into words using whitespace as delimiter
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    std::string topic;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if(words[i] == "MODE"){
            channelName = words[i + 1];
            nickName = words[i + 3];
            break;
        }
    }
}

void CommandHandler::removeElementByFd(std::vector<int>& vec, int id) {
    for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it) {
        if (*it == id) {
            vec.erase(it);
            return;
        }
    }
}

void CommandHandler::sendExtraMessage(int fd, std::string message){
    ssize_t sendStatus;
    sendStatus = send(fd, message.c_str(), message.length(), 0);
    if (sendStatus <= 0){
        std::cout << Red << "Failed to send message" << Reset << std::endl;
    }
    else if (sendStatus > 0){
        std::cout << Blue << "=> Server Sended Response with: " << Reset << message << std::endl;
    }
    else{
        std::cout << Blue << "=> Server Sended Response with: " << Reset << message;
    }
}

void CommandHandler::deleteClient(std::map<int, Client*>& clients, int clientFd) {
    std::map<int, Client*>::iterator iter = clients.find(clientFd);
    if (iter != clients.end()) {
        delete iter->second;
        clients.erase(iter);
    }
}

void CommandHandler::deleteClientFromAllChannels(std::vector<Channel>& channels, int clientFd, std::string nickName){
    int status = 0;
    for (size_t i = 0; i < channels.size();i++) {
        if (isInChannel(clientFd, channels[i])){
            //delete Client from Channel
            channels[i].getInvitedClients().erase(nickName);
            status++;
            //if is Operator remove it
            if (isOperator(channels[i].getOpClientFd(), clientFd)){
                removeElementByFd(channels[i].getOpClientFd(), clientFd);
                //if there is no clients on channel
                if (channels[i].getInvitedClients().empty()){
                    //deleteChannel
                    channels.erase(channels.begin() + i);
                }
                else{
                    //if still clients and there is no operator the first one will be it
                    if (channels[i].getOpClientFd().empty()){
                        std::map<std::string, Client>::iterator it = channels[i].getInvitedClients().begin();
                        channels[i].getOpClientFd().push_back(it->second.getSocketFd());
                    }
                }
            }
        }
    }
    if (status == 0)
    {
        throw std::runtime_error("Client Not In Any Channel");
    }
}

void CommandHandler::updateClient(std::vector<Channel>& channels, int clientFd, std::string nickName, std::string newName) {
    for (size_t i = 0; i < channels.size(); i++) {
        if (isInChannel(clientFd, channels[i])) {
            std::map<std::string, Client>& invitedClients = channels[i].getInvitedClients();
            std::map<std::string, Client>::iterator it = invitedClients.find(nickName);
            if (it != invitedClients.end()) {
                Client& client = it->second;

                // Remove the old element from the map

                // Modify the key and update the client's properties
                client.setNickName(newName);
                client.setUserName(newName);
                client.setHostName(newName);

                // Insert the modified element back into the map with the new key
                std::pair<std::string, Client> newElement(newName, client);
                invitedClients.insert(newElement);
                invitedClients.erase(it);
                break;
            }
        }
    }
}


bool CommandHandler::isInvitedToChannel(std::vector<int> &invitedVector, int clientFd){
    for(std::vector<int>::iterator it = invitedVector.begin(); it != invitedVector.end();++it){
        if (*it == clientFd){
            invitedVector.erase(it);
            return true;
        }
    }
    return false;
}

bool CommandHandler::isPasswordGood(std::string message, std::string password){
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    std::string topic;
    while (iss >> word) {
        words.push_back(word);
    }
    if (words[2] == password){
        return true;
    }
    return false;
}

std::string CommandHandler::getNameOfClients(std::map<std::string, Client> clients){
    std::string clientsName = "@";
    for(std::map<std::string, Client>::iterator it = clients.begin(); it != clients.end(); ++it){
        clientsName += it->second.getNickName();
        clientsName += " ";
    }
    return clientsName;
}

bool CommandHandler::isRequest(std::string message){
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    std::string topic;
    while (iss >> word) {
        words.push_back(word);
    }
    if (words.size() == 2){
        return true;
    }
    return false;
}

bool CommandHandler::instanceExist(std::map<std::string, Client> clientsOnChannel, std::string nickName){
    for (std::map<std::string, Client>::iterator it = clientsOnChannel.begin(); it != clientsOnChannel.end(); ++it){
        std:: cout << "KEY: " << it->first << " nickName: " << it->second.getNickName() << "Fd: " << it->second.getSocketFd() << std::endl;
        if (it->first == nickName){
            return true;
        }
    }
    return false;
}

std::string CommandHandler::getAllChannelsNames(std::vector<Channel> channels){
    std::string response = "LIST: ";
    for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it){
        response += it->getChannelName();
        response += " ";
    }
    response += "\r\n";
    return response;
}

unsigned int CommandHandler::extractMaxClients(std::string message){
    std::istringstream iss(message);
    std::string word;
    std::vector<std::string> words;
    std::string topic;
    while (iss >> word) {
        words.push_back(word);
    }
    for (size_t i = 0; i < words.size(); i++){
        if (words[i] == "MODE"){
            return static_cast<unsigned int>(atoi(words[i + 3].c_str()));
        }
    }
    return 200;
}

bool CommandHandler::isValidMode(std::string mode){
    std::string validModes[] = {"+i", "-i", "+t", "-t", "+k", "-k", "+o", "-o", "+l", "-l"};
    for (int i = 0; i < 10; i++){
        if(validModes[i] == mode){
            return true;
        }
    }
    return false;
}
