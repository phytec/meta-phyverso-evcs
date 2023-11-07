######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  =  ../../source/lib
INCLUDE_OSAL = ${LIBPATH}/osal/inc
INCLUDE_COMM_LIB = "${LIBPATH}/comm_mgr/inc"
INCLUDE_MEM_POOL = ${LIBPATH}/mem_pool/inc
INCLUDE_CG_LIB = "../../include"
INCLUDE_PUB = "../../../public"
INLCUDE_COMMON_H = ${INCLUDE_PUB}/common
INCLODE_HOST_H = ${INCLUDE_PUB}/host
INCLUDE_CONF = ${INCLUDE_PUB}/production

CFLAGS = $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I ${INCLUDE_OSAL} -I ${INCLUDE_MEM_POOL} -I $(INCLUDE_COMM_LIB) -I$(INCLUDE_CONF) -I$(INCLUDE_CG_LIB) -I$(INCLODE_HOST_H) -I$(INLCUDE_COMMON_H)
TARGET = $(TARGET_NAME)

LIBS = $(LIBPATH)/host_lib.a $(LIBPATH)/comm_mgr.a $(LIBPATH)/mem_pool.a $(LIBPATH)/osal.a -lm -lpthread -lgpiod

SRC = $(SRC_FILES)
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))
TARGET_NAME = host_loading_service
SRC_FILES = host_loading_service.c

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all : $(OBJ)
	$(CC) $(CFLAGS) $(DEBUG) -o $(TARGET) $(OBJ) $(LIBS) $(CROSS_LIBS)

clean:
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)
