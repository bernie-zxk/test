.SUFFIXES: .idl .cpp .o


PROGRAM = libList.so 

OBJECTS = List.o
OBJ_EXT = 

CC	    = `makeopts.sh cc`
CCFLAGS = `makeopts.sh ccflag `
LDFLAGS = `makeopts.sh ldflag`
LIBS	= `makeopts.sh syslib sam ` -lservices -lmstream
# Valid LIBS args: syslib odb_api sam log para msg marshal_lib corba_lib odb_net socket`
MAKE_SO = `makeopts.sh mk_so`
LIB_PATH = `makeopts.sh lib_path`

.idl.cpp:
	@echo	$(IDL_CC) $<
	@	$(IDL_CC) $<


.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

$(PROGRAM): $(OBJECTS) $(OBJ_EXT)
	@echo   $(MAKE_SO) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS)
	@       $(MAKE_SO) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS)

#	@echo   cp $(PROGRAM) $(LIB_PATH)
#	@       cp $(PROGRAM) $(LIB_PATH)


# optional part

install:
#	cd $(LIB_PATH) && cp -rp $(PROGRAM) $(D5000_HOME)/lib

RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

cleano:
	$(RM) $(OBJECTS)

cleanp:
	$(RM) $(PROGRAM)

