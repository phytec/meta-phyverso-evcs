######################################################################
# Other Declarations:                                                #
######################################################################
PROD_LIBPATH  =  ../../
INCLUDE_PUB = ../../../public
INCLUDE_PROD_LIB  =  ${INCLUDE_PUB}/production
INCLUDE_CG_LIB = ${INCLUDE_PUB}/host
INLCUDE_COMMON_H = ${INCLUDE_PUB}/common

CFLAGS = $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I$(INCLUDE_CG_LIB) -I$(INCLUDE_PUB) -I$(INCLUDE_PROD_LIB) -I$(INLCUDE_COMMON_H)
TARGET = production_tool

LIBS = $(PROD_LIBPATH)/production.a $(PROD_LIBPATH)/source/lib/FFT.a -lm

SRC = production_tool.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all : $(OBJ)
	$(CC) $(CFLAGS) $(DEBUG) -o $(TARGET) $(OBJ) $(LIBS) $(CROSS_LIBS)

clean:
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)
	rm -f $(TARGET).exe
