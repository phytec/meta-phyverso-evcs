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
OSAL_INCLUDE = $(LIBPATH)/osal/inc
CFLAGS = $(CROSS_CFLAGS) -I ${INCLUDE_PUB} -I $(OSAL_INCLUDE) -I $(INCLUDE) -I$(INCLUDE_COMMON_HEADERS)
CPPFLAGS = $(CROSS_CPPFLAGS)
TARGET = $(LIBPATH)/mem_pool.a

SRC = src/mem_pool.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))
LIBS = $(LIBPATH)/osal.a

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)

