#F: ctb.mak
# Modified: 2022.11.26 added CFLAGS
# Coded at: 2022.8.6 by T. Abi from ece.mak

CC=cc

CFLAGS=-std=c11 -O2 -Wall -fPIC -posix

PRG=ctb

ifdef DEBUG
  OPT=$(CFLAGS) -ggdb
else
  OPT=$(CFLAGS)
endif

$(PRG): $(PRG).c
	$(CC) $(OPT) -o $@ $^

clean:
	-rm $(PRG)

