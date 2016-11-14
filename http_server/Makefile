CPP = g++

NAME = TEST_POLL

CCFLAGS = -g -Wall
CPPFLAGS = -std=c++11
CPPFLAGS += $(CCFLAGS)


###########################
# all .cpp src
# #########################
SOCK_CPP = SocketImp.cpp
SOCK_CPP := $(addprefix ./socket/, $(SOCK_CPP))

POLL_CPP = PollImp.cpp
POLL_CPP := $(addprefix ./event/, $(POLL_CPP))

EVENT_CPP = Event.cpp
EVENT_CPP := $(addprefix ./event/, $(EVENT_CPP))

TEST_CPP = testpoll.cpp
TEST_CPP := $(addprefix ./test/, $(TEST_CPP))

CPPFILES = $(TEST_CPP)
CPPFILES += $(EVENT_CPP)
CPPFILES += $(POLL_CPP)
CPPFILES += $(SOCK_CPP)


INCLUDE = -I./socket -I./tcp -I./http -I./event -I./test

OBJ = $(CPPFILES:.cpp=.o)

######################
#complie files
#####################
all : $(NAME)

$(NAME): $(OBJ)
	$(CPP) -o $(NAME) $(OBJ)


.SUFFIXES : .cpp

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCLUDE) -c $<



.PYTHON : clean
clean:
	-rm -rf *.o test

