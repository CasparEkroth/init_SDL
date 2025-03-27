# Makefile-exempel: bygger server/client med SDL2-stöd

# ==== OS-detektering ====
OS := $(shell uname -s 2>/dev/null)
ifeq ($(OS),)
    OS := Windows_NT
endif

ifeq ($(OS), Darwin)
# --- macOS Settings ---
    CC = clang
    CFLAGS = -fsanitize=address -fsanitize=undefined -g -Wall -Wextra \
             -I/opt/homebrew/include/SDL2 \
             -I/opt/homebrew/include/SDL2_image \
             -I/opt/homebrew/include/SDL2_ttf \
             -I/opt/homebrew/include/SDL2_mixer \
             -I/opt/homebrew/include/SDL2_net
    LDFLAGS = -fsanitize=address \
              -L/opt/homebrew/lib \
              -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net
    REMOV = rm -f
    SERVER_TARGET = server
    CLIENT_TARGET = client
    RUN = ./
else ifeq ($(OS), Windows_NT)
# --- Windows (MinGW/MSYS) Settings ---
    CC = gcc
    INCLUDE = C:/msys64/mingw64/include/SDL2
    CFLAGS = -g -Wall -Wextra -I$(INCLUDE)
    LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf \
              -lSDL2_mixer -lSDL2_net -mwindows
    REMOV = del /f
    SERVER_TARGET =server.exe
    CLIENT_TARGET =client.exe
    RUN = ./
endif

# ==== Vanliga variabler ====

SRCDIR = source
NETDIR = source/network

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(NETDIR)/server.c
	$(CC) $(CFLAGS) $(NETDIR)/server.c -o $(SERVER_TARGET) $(LDFLAGS)

$(CLIENT_TARGET): $(NETDIR)/client.c
	$(CC) $(CFLAGS) $(NETDIR)/client.c -o $(CLIENT_TARGET) $(LDFLAGS)

clean:
	$(REMOV) *.o $(SERVER_TARGET) $(CLIENT_TARGET)

run_server:
	$(RUN)$(SERVER_TARGET)

run_client:
	$(RUN)$(CLIENT_TARGET)

ifeq ($(OS), Windows_NT)
run_clients:
	powershell -Command "Start-Process .\$(CLIENT_TARGET)"
	powershell -Command "Start-Process .\$(CLIENT_TARGET)"
else
run_clients:
	./$(CLIENT_TARGET) & ./$(CLIENT_TARGET)
endif
# main.o: $(SRCDIR)/main.c
# 	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o main.o 

# initSDL.o: $(SRCDIR)/initSDL.c
# 	$(CC) $(CFLAGS) $(SRCDIR)/initSDL.c -o initSDL.o

# toolSDL.o: $(SRCDIR)/toolSDL.c
# 	$(CC) $(CFLAGS) $(SRCDIR)/toolSDL.c -o toolSDL.o

# server.o: source/network/server.c
# 	$(CC) $(CFLAGS) source/server.c -o server.o

# client.o: source/network/client.c
# 	$(CC) $(CFLAGS) source/client.c -o client.o
