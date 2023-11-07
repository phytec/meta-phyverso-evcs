######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = source/lib
INCLUDE  = include
INCLUDE_OSAL = ${LIBPATH}/osal/inc
INCLUDE_PUB = ../public
INLCUDE_COMMON_H = ${INCLUDE_PUB}/common
INCLODE_HOST_H = ${INCLUDE_PUB}/host
INCLODE_PROD_H = ${INCLUDE_PUB}/production
INCLUDE_COMMUNICATION = ${LIBPATH}/comm_mgr/inc
INCLUDE_MEM_POOL = ${LIBPATH}/mem_pool/inc

CFLAGS = $(CROSS_CFLAGS) -DFW_LOAD_TIMING
CPPFLAGS = $(CROSS_CPPFLAGS) -I $(INCLUDE_OSAL)  -I${INCLUDE_MEM_POOL} -I $(INCLUDE) -I $(INCLUDE_PUB) -I$(INCLUDE_COMMUNICATION) -I$(INLCUDE_COMMON_H) -I$(INCLODE_HOST_H) -I$(INCLODE_PROD_H)
TARGET = $(LIBPATH)/host_lib.a

SRC = source/HLB_apcm.c source/HLB_protocol.c source/HLB_helper.c source/HLB_host.c source/HLB_nscm.c source/HLB_legacy_commands.c source/HLB_fwload.c source/HLB_noise_floor.c source/HLB_ind_listener.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
SU_FILES = $(addsuffix .su, $(basename $(SRC)))
LIBS =

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)
	$(MAKE) -C source/lib all
	$(MAKE) -C examples all

clean:
	$(MAKE) -C source/lib clean
	rm -f $(SU_FILES)
	rm -f $(OBJ)
	rm -f $(TARGET)
	$(MAKE) -C examples clean