.SUFFIXES: .cxx .o

CC	= `makeopts.sh cc`
CCFLAGS = `makeopts.sh ccflag`
LDFLAGS = `makeopts.sh ldflag`
LIBS	= `makeopts.sh syslib sam` -lpthread -lservices
INC_DIR = -I$(HOME)/src/platform/servicebus/include

.c.o:
	echo	$(CC) $(INC_DIR) $(CCFLAGS) $<
	$(CC) $(INC_DIR) $(CCFLAGS) $<
	
PROGRAM1 = DMailUDPClient 
OBJECTS1 = UDPClient.o
OBJ_EXT1 = 

all : $(PROGRAM1)

$(PROGRAM1): $(OBJECTS1) $(OBJ_EXT1) $(LOCAL_LIBRARIES)
	@echo	$(CC) -o $(PROGRAM1)  $(OBJECTS1) $(OBJ_EXT1) $(LDFLAGS) $(INC_DIR) $(LIBS) 
	@	$(CC) -o $(PROGRAM1)  $(OBJECTS1) $(OBJ_EXT1) $(LDFLAGS) $(INC_DIR) $(LIBS) 
	#cp $(PROGRAM1) $(SRC_HOME)/bin
