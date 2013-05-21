#Compilers:
#For linux / windows (MinGW)
CC = gcc
#For crosscompiling for windows on Linux
WCROSS_CC = i686-pc-mingw32-gcc

#definitions to be carried on compilation
#To use Windows features like winsock
WDEFINES = -D__WIN32__
#To stay with linux features
DEFINES = -D__LINUX__

#Flags for compilation
#Linux:
C_FLAGS = -Wall -ggdb -I ../common/
#Windows:
WC_FLAGS = -Wall -ggdb -I ../common/ -I ../freeglut/include/

#Linker flags:
#Linux:
LD_FLAGS = -lglut -lm -lGL -lpthread -lGLU -lrt
#Windows
WLD_FLAGS = -L../freeglut/lib/ -lfreeglut -lm -lpthread -lopengl32 -lglu32 -lws2_32

#Sources and targets
C_SRC =  pong_client_queue.c pong_client_netwIF.c pong_client_starter.c pong_drawer.c drawable_text.c  movable_item.c drawable_balls.c drawable_paddles.c drawable_walls.c ../common/pong_dataStorage.c ../common/pong_IdFifo.c ../common/pong_misc_func.c ../common/pong_netw.c ../common/pong_queue.c
TGT = pongClient
WTGT = $(TGT).exe
WCROSS_TGT = $(TGT)_crosscompiled.exe

#Test defines
TESTSRC = test.c
TESTOUT = test
WTESTOUT = test.exe
WCROSSTESTOUT = test_crosscompiled.exe


all: $(C_SRC)
	$(CC) $(C_FLAGS) $(C_SRC) -o $(TGT) $(LD_FLAGS) $(DEFINES)

win: $(C_SRC)
	$(CC) $(WC_FLAGS) $(C_SRC) -o $(WTGT) $(WLD_FLAGS) $(WDEFINES)
	@echo 'Copy freeglut.dll from ../freeglut/bin to folder where $(WTGT) resides!'

wincross: $(C_SRC)
	$(WCROSS_CC) $(WC_FLAGS) $(C_SRC) -o $(WCROSS_TGT) $(WLD_FLAGS) $(WDEFINES)
	@echo 'Copy freeglut.dll from ../freeglut/bin to folder where $(WTGT) resides!'
test: $(TESTSRC)
	$(CC) $(C_FLAGS) $(TESTSRC) -o $(TESTOUT) $(LD_FLAGS)

wintest:$(TESTSRC)
	$(CC) $(WC_FLAGS) $(TESTSRC) -o $(WTESTOUT) $(WLD_FLAGS)

wincrosstest:$(TESTSRC)
	$(WCROSS_CC) $(WC_FLAGS) $(TESTSRC) -o $(WCROSSTESTOUT) $(WLD_FLAGS)
clean:
	rm -rf $(TGT)
	rm -rf $(WTGT)
	rm -rf $(WCROSS_TGT)
	rm -rf *.o
	rm -rf ../common/*.o
	rm -rf $(TESTOUT)
	rm -rf $(WTESTOUT)
	rm -rf $(WCROSSTESTOUT)

#gcc -Wall pong.c pong_client_netwIF.c pong_client_starter.c pong_drawer.c ../common/pong_dataStorage.c ../common/pong_IdFifo.c ../common/pong_misc_func.c ../common/pong_netw.c -o pong -D__LINUX__ -I ../common/ -lglut -lm -lGL -lpthread -lGLU
