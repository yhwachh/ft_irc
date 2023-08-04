NAME = ircserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC = sources/main.cpp sources/Server.cpp \
		sources/Channel.cpp sources/Client.cpp sources/CommandHandler.cpp

OBJS = $(SRC:.cpp=.o)

.cpp.o :
	${CC} ${CFLAGS} -c $< -o $@

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

re :  fclean all

clean :
	rm -rf $(NAME) $(OBJS)
fclean : clean