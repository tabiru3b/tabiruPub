# F: kie.mak
# Coded at: 2021.8.18 by T. Abi from k0.mak
# Modified: 2022.2.23 c11 style

SYHM=C:\opt\Octave-6.4.0\mingw64
CINC=$(SYHM)\include
CLIB=$(SYHM)\lib

ifndef PRG
PRG=kie
endif
ifdef DEBUG
OPT=-ggdb
else
OPT=-Wall
endif

$(PRG).exe : $(PRG).c
	gcc -mwindows -std=c11 $(OPT) -o $(PRG).exe $(PRG).c

clean:
ifdef DEBUG
endif
	-rm $(PRG).exe
