CPP = g++

NAME = HttpServer

##########################################
#complie flags
CCFLAGS = -g -Wall
CPPFLAGS = -std=c++11
CPPFLAGS += $(CCFLAGS)
THREADFLAGS = -lpthread


###########################
# network file

NETWORKINC = ./network

SOCK_CPP = Socketer.cpp
SOCK_CPP := $(addprefix $(NETWORKINC)/, $(SOCK_CPP))

NETWORKFILES = $(SOCK_CPP)


###########################
# event file
EVENTINC = ./event

POLL_CPP = PollImp.cpp
POLL_CPP := $(addprefix $(EVENTINC)/, $(POLL_CPP))

EVENT_CPP = Event.cpp
EVENT_CPP := $(addprefix $(EVENTINC)/, $(EVENT_CPP))

##the files of follow is new add which is replace the upon
CHANNEL_CPP = Channel.cpp
CHANNEL_CPP := $(addprefix $(EVENTINC)/, $(CHANNEL_CPP))

EPOLLER_CPP = EPoller.cpp
EPOLLER_CPP := $(addprefix $(EVENTINC)/, $(EPOLLER_CPP))

EVENTLOOP_CPP = EventLoop.cpp
EVENTLOOP_CPP := $(addprefix $(EVENTINC)/, $(EVENTLOOP_CPP))

EVENTFILES = $(POLL_CPP)
EVENTFILES += $(EVENT_CPP)
EVENTFILES += $(CHANNEL_CPP)
EVENTFILES += $(EPOLLER_CPP)
EVENTFILES += $(EVENTLOOP_CPP)


###########################
# exception file
EXCEPTIONINC = ./exception

EXCEPTION_CPP = Exception.cpp
EXCEPTION_CPP := $(addprefix ./exception/, $(EXCEPTION_CPP))

EXCEPTIONFILES = $(EXCEPTION_CPP)


####################################
#thread files
THREADINC = ./threadpool

THREAD_CPP = Thread.cpp
THREAD_CPP := $(addprefix $(THREADINC)/, $(THREAD_CPP))

THREADPOOL_CPP = ThreadPool.cpp
THREADPOOL_CPP := $(addprefix $(THREADINC)/, $(THREADPOOL_CPP))

THREADDFILES = $(THREAD_CPP)
THREADDFILES += $(THREADPOOL_CPP)

#################################
#mutex files
MUTEXINC = ./mutex

CONDITION_CPP = Condition.cpp
CONDITION_CPP := $(addprefix $(MUTEXINC)/, $(CONDITION_CPP))

MUTEXLOCK_CPP = MutexLock.cpp
MUTEXLOCK_CPP := $(addprefix $(MUTEXINC)/, $(MUTEXLOCK_CPP))

SCOPEDMUTEXLOCK_CPP = ScopedMutexLock.cpp
SCOPEDMUTEXLOCK_CPP := $(addprefix $(MUTEXINC)/, $(SCOPEDMUTEXLOCK_CPP))

MUTEXFILES = $(CONDITION_CPP)
MUTEXFILES += $(MUTEXLOCK_CPP)
MUTEXFILES += $(SCOPEDMUTEXLOCK_CPP)

############################################
#common files
COMMONINC = ./common

###########################
# log file

LOGINC = ./log

LOGGER_CPP = Logger.cpp
LOGGER_CPP := $(addprefix $(LOGINC)/, $(LOGGER_CPP))

MSGTEXT_CPP = MsgText.cpp
MSGTEXT_CPP := $(addprefix $(LOGINC)/, $(MSGTEXT_CPP))

FILEHANDLE_CPP = FileHandle.cpp
FILEHANDLE_CPP := $(addprefix $(LOGINC)/, $(FILEHANDLE_CPP))

LOGFILES = $(LOGGER_CPP)
LOGFILES += $(MSGTEXT_CPP)
LOGFILES += $(FILEHANDLE_CPP)


###########################
#obj files
TARGETINC = ./sample

SAMPLE_CPP = TestThreadPool.cpp
SAMPLE_CPP := $(addprefix $(TARGETINC)/, $(SAMPLE_CPP))

TARGETFILES = $(SAMPLE_CPP)


###########################
#
CPPFILES = $(TARGETFILES)
CPPFILES += $(EXCEPTIONFILES)
CPPFILES += $(THREADDFILES)
CPPFILES += $(MUTEXFILES)
CPPFILES += $(NETWORKFILES)
CPPFILES += $(EVENTFILES)
CPPFILES += $(LOGFILES)

INCLUDE = -I $(TARGETINC)
INCLUDE += -I $(COMMONINC)
INCLUDE += -I $(MUTEXINC)
INCLUDE += -I $(THREADINC)
INCLUDE += -I $(EXCEPTIONINC)
INCLUDE += -I $(LOGINC)
INCLUDE += -I $(EVENTINC)
INCLUDE += -I $(NETWORKINC)

OBJ = $(CPPFILES:.cpp=.o)

######################
#complie files

all : $(NAME)

$(NAME): $(OBJ)
	$(CPP) -o  $(NAME) $(OBJ) $(THREADFLAGS)

.SUFFIXES : .cpp

.cpp.o:
	$(CPP) -c -o $*.o $(CPPFLAGS) $(INCLUDE) $*.cpp



.PYTHON : clean
clean:
	rm -f $(CPPFILES:.cpp=.o) $(NAME)
