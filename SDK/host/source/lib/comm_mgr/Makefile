######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../
INCLUDE  = inc
INCLUDE_OSAL = ${LIBPATH}/osal/inc
INCLUDE_PUB = ../../../../public
INCLUDE_MEM_POOL = ${LIBPATH}/mem_pool/inc
INCLUDE_COMMON_HEADERS = ../../../../public/common

CFLAGS = $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I ${INCLUDE_OSAL} -I${INCLUDE_MEM_POOL} -I${INCLUDE_PUB} -I $(INCLUDE) -I$(INCLUDE_COMMON_HEADERS)
TARGET = $(LIBPATH)/comm_mgr.a

SRC = src/comm_mgr_lib.c
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

