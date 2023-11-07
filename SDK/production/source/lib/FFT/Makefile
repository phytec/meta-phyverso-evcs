######################################################################
# Other Declarations:                                                #
######################################################################
include ../../../CompilerDeclaration

ifeq ($(OS),Windows_NT)
	export CC=$(CROSS)gcc
endif
LIBPATH  = ../

INCLUDE  = inc
CFLAGS = $(CROSS_CFLAGS) -I $(INCLUDE)
CPPFLAGS = $(CROSS_CPPFLAGS)
TARGET = $(LIBPATH)/FFT.a

SRC = FFT.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))
LIBS =

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)

