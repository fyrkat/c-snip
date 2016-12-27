CFLAGS=-Wall -std=c11 -lev

all: echo client proxy
echo: echo.c socket_get.c
client: client.c socket_get.c
proxy: proxy.c socket_get.c
