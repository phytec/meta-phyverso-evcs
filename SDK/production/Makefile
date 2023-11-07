include CompilerDeclaration

ifeq ($(OS),Windows_NT)
	export CC=$(CROSS)gcc
endif

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = .
INCLUDE_FFT = source/lib/FFT
INCLUDE_PUB = ../public
INCLUDE  = ${INCLUDE_PUB}/production
INLCUDE_COMMON_H = ${INCLUDE_PUB}/common
CFLAGS = $(CROSS_CFLAGS) -I ${INCLUDE_PUB} -I $(INCLUDE) -I $(INLCUDE_COMMON_H) -I $(INCLUDE_FFT)
CPPFLAGS = $(CROSS_CPPFLAGS) 
TARGET = $(LIBPATH)/production.a

SRC = source/PROD_lib.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))
LIBS =

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)
	$(MAKE) -C source/lib/ all
	$(MAKE) -C examples/production_tool all

clean:
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)
	$(MAKE) -C source/lib/ clean
	$(MAKE) -C examples/production_tool clean