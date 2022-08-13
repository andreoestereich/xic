# st - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:

X11LIB = /usr/X11R6/lib

CC=gcc

LIBS = -L$(X11LIB) -lX11 -lutil -lgd

SRC = xic.c
OBJ = $(SRC:.c=.o)

all: $(SRC)
	$(CC) -o xic $(SRC) $(LIBS)
