.SUFFIXES: .idl .cpp .o


PROGRAM = tool

OBJECTS = main.o mytool.o

OBJ_EXT =

CC	= g++ -fpermissive
CCFLAGS = -c -g -I. -D_LINUX  -fPIC -Wall -Wno-format-y2k 
LDFLAGS = 
LIBS	= 



.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

$(PROGRAM): $(OBJECTS) $(OBJ_EXT)
	@echo	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS) 
	@	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS) 




RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

cleano:
	$(RM) $(OBJECTS)

cleanp:
	$(RM) $(PROGRAM)

