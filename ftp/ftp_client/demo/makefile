.SUFFIXES:  .cpp .o


PROGRAM = demo 

OBJECTS = demo.o

CC	    = `makeopts.sh cc`
CCFLAGS = `makeopts.sh ccflag `
LDFLAGS = `makeopts.sh ldflag`
LIBS	= `makeopts.sh syslib ` -lftp_client

.cpp:
	@echo	$(IDL_CC) $<
	@	$(IDL_CC) $<
.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

$(PROGRAM): $(OBJECTS)
	@echo   $(CC) -o $(PROGRAM)  $(OBJECTS)  $(LDFLAGS) $(LIBS)
	@       $(CC) -o $(PROGRAM)  $(OBJECTS)  $(LDFLAGS) $(LIBS)

#	@echo   cp $(PROGRAM) $(SRC_HOME)/bin
#	@       cp $(PROGRAM) $(SRC_HOME)/bin



install:
#	cd $(LIB_PATH) && cp -rp $(PROGRAM) $(D5000_HOME)/lib

RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

cleano:
	$(RM) $(OBJECTS)

cleanp:
	$(RM) $(PROGRAM)

