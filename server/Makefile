CC = gcc
WCROSS_CC = i686-pc-mingw32-gcc
WDEFINES = -D__WIN32__
DEFINES = -D__LINUX__
C_FLAGS = -Wall -ggdb -I ../common/
C_SRC =  place_counter_thread.c pong_server_accept_client.c serverStartup.c pong_server_queue.c ../common/pong_dataStorage.c ../common/pong_IdFifo.c ../common/pong_misc_func.c ../common/pong_netw.c ../common/pong_queue.c 
LD_FLAGS = -lglut -lm -lGL -lpthread -lGLU -lrt
WC_FLAGS = -Wall -I ../common/ -I ../freeglut/include/
WCROSS_FLAGS  = -Wall -ggdb -I ../common/ -I ../freeglut/include/
TGT = pongServer
WLD_FLAGS = -L../freeglut/lib/ -lfreeglut -lm -lpthread -lopengl32 -lglu32 -lws2_32
WTGT = $(TGT).exe
WCROSS_TGT = $(TGT)_crosscompiled.exe

all: $(C_SRC)
	$(CC) $(C_FLAGS) $(C_SRC) -o $(TGT) $(LD_FLAGS) $(DEFINES)

win: $(C_SRC)
	$(CC) $(WC_FLAGS) $(C_SRC) -o $(WTGT) $(WLD_FLAGS) $(WDEFINES)
	@echo 'Copy freeglut.dll from ../freeglut/bin to folder where $(WTGT) resides!'

wincross: $(C_SRC)
	$(WCROSS_CC) $(WCROSS_FLAGS) $(C_SRC) -o $(WCROSS_TGT) $(WLD_FLAGS) $(WDEFINES)
	@echo 'Copy freeglut.dll from ../freeglut/bin to folder where $(WTGT) resides!'

clean:
	rm -rf $(TGT)
	rm -rf $(WTGT)
	rm -rf $(WCROSS_TGT)
	rm -rf *.o
	rm -rf ../common/*.o
#gcc -Wall pong.c pong_client_netwIF.c pong_client_starter.c pong_drawer.c ../common/pong_dataStorage.c ../common/pong_IdFifo.c ../common/pong_misc_func.c ../common/pong_netw.c -o pong -D__LINUX__ -I ../common/ -lglut -lm -lGL -lpthread -lGLU
