.SUFFIXES: .cpp .o


PROGRAM = mysql_test

OBJECTS = mysql_api.o \
	  mysql_api_demo.o \

CC	= g++
CCFLAGS = -Wall
LDFLAGS = 
LIBS	= 


.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

$(PROGRAM): $(OBJECTS) 
	@echo	$(CC) -o $(PROGRAM)  $(OBJECTS)  $(LDFLAGS) $(LIBS) 
	@	$(CC) -o $(PROGRAM)  $(OBJECTS)  $(LDFLAGS) $(LIBS) 


RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

cleano:
	$(RM) $(OBJECTS)

cleanp:
	$(RM) $(PROGRAM)




