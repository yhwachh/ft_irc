#include "../classes/Server.hpp"

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGQUIT) {
		exit(-1);
	}
	std::cout << "signal detected" << std::endl;
}

int main(int argc, char**argv){
	if (argc != 3){
		std::cerr << "Not enough arguments to run the program." << std::endl;
		return -1;
	}
	try{
		std::string password(argv[2]);
		unsigned int port = atoi(argv[1]);
		Server *server = new Server(port, password);
		server->run();
		delete server;
	}
	catch(std::exception &e){
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}