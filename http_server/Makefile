CPP = g++

NAME = SAMPLE

CCFLAGS = -g -Wall
CPPFLAGS = -std=c++11
CPPFLAGS += $(CCFLAGS)
THREADFLAGS = -lpthread


###########################
# all .cpp src
# #########################
SOCK_SRC = ./socket
POLL_SRC = ./event
EVENT_SRC = ./event
SAMPLE_SRC = ./sample
COMMON_SRC = ./common
EXCEPTION_SRC = ./exception
THREADPOOL_SRC = ./threadpool
MUTEX_SRC = ./mutex

SOCK_CPP = SocketImp.cpp
SOCK_CPP := $(addprefix ./socket/, $(SOCK_CPP))

POLL_CPP = PollImp.cpp
POLL_CPP := $(addprefix ./event/, $(POLL_CPP))

EVENT_CPP = Event.cpp
EVENT_CPP := $(addprefix ./event/, $(EVENT_CPP))

EXCEPTION_CPP = Exception.cpp
EXCEPTION_CPP := $(addprefix ./exception/, $(EXCEPTION_CPP))

THREAD_CPP = Thread.cpp
THREAD_CPP := $(addprefix ./threadpool/, $(THREAD_CPP))

THREADPOOL_CPP = ThreadPool.cpp
THREADPOOL_CPP := $(addprefix ./threadpool/, $(THREADPOOL_CPP))

CONDITION_CPP = Condition.cpp
CONDITION_CPP := $(addprefix $(MUTEX_SRC)/, $(CONDITION_CPP))

MUTEXLOCK_CPP = MutexLock.cpp
MUTEXLOCK_CPP := $(addprefix $(MUTEX_SRC)/, $(MUTEXLOCK_CPP))

SCOPEDMUTEXLOCK_CPP = ScopedMutexLock.cpp
SCOPEDMUTEXLOCK_CPP := $(addprefix ./mutex/, $(SCOPEDMUTEXLOCK_CPP))

SAMPLE_CPP = TestThreadPool.cpp
SAMPLE_CPP := $(addprefix ./sample/, $(SAMPLE_CPP))

###########################
# log file

LOGGER_CPP = Logger.cpp
LOGGER_CPP := $(addprefix ./log/, $(LOGGER_CPP))

MSGTEXT_CPP = MsgText.cpp
MSGTEXT_CPP := $(addprefix ./log/, $(MSGTEXT_CPP))

FILEHANDLE_CPP = FileHandle.cpp
FILEHANDLE_CPP := $(addprefix ./log/, $(FILEHANDLE_CPP))

LOGFILES = $(LOGGER_CPP)
LOGFILES += $(MSGTEXT_CPP)
LOGFILES += $(FILEHANDLE_CPP)

LOGINC = ./log
##########################
CPPFILES = $(SAMPLE_CPP)
CPPFILES += $(EVENT_CPP)
CPPFILES += $(POLL_CPP)
CPPFILES += $(SOCK_CPP)
CPPFILES += $(EXCEPTION_CPP)
CPPFILES += $(THREAD_CPP)
CPPFILES += $(THREADPOOL_CPP)
CPPFILES += $(CONDITION_CPP)
CPPFILES += $(MUTEXLOCK_CPP)
CPPFILES += $(SCOPEDMUTEXLOCK_CPP)
CPPFILES += $(LOGFILES)

INCLUDE = -I./socket -I./tcp -I./http -I./event
INCLUDE += -I./sample 
INCLUDE += -I./common
INCLUDE += -I./exception -I./mutex
INCLUDE += -I./threadpool
INCLUDE += -I $(LOGINC)

OBJ = $(CPPFILES:.cpp=.o)

######################
#complie files
#####################
all : $(NAME)

$(NAME): $(OBJ)
	$(CPP) -o  $(NAME) $(OBJ) $(THREADFLAGS)

.SUFFIXES : .cpp

.cpp.o:
	$(CPP) -c -o $*.o $(CPPFLAGS) $(INCLUDE) $*.cpp



.PYTHON : clean
clean:
	rm -f $(CPPFILES:.cpp=.o) $(NAME)
