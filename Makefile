CC=gcc
LIBS=alsa dbus-1
CFLAGS=-std=gnu99 -Wall -pedantic -Wextra `pkg-config --cflags ${LIBS}`
LDFLAGS=`pkg-config --libs ${LIBS}`

all: main

main: main.o
	${CC} ${LDFLAGS} main.o -o alsa-volume-monitor

main.o:
	${CC} ${CFLAGS} -c main.c
