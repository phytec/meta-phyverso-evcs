######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../

INCLUDE  = inc
INCLUDE_PUB = ../../../../public
INCLUDE_COMMON_HEADERS = ../../../../public/common
CFLAGS = $(CROSS_CFLAGS) -I ${INCLUDE_PUB} -I $(INCLUDE) -I $(INCLUDE_COMMON_HEADERS)
CPPFLAGS = $(CROSS_CPPFLAGS)
TARGET = $(LIBPATH)/osal.a

SRC = src/osal.c
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

