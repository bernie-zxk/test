.SUFFIXES: .idl .cpp .o


PROGRAM = sem_test

OBJECTS = sem.o 
         


OBJ_EXT =  

CC	= `makeopts.sh cc`
CCFLAGS = `makeopts.sh ccflag $(DATABASE)_inc `  
LDFLAGS = `makeopts.sh ldflag` 
LIBS	= `makeopts.sh sam para` -lpthread   #-ldb_rep
# Valid LIBS args: syslib odb_api sam para msg marshal_lib corba_lib odb_net socket $(DATABASE)_lib`

.idl.cpp:
	@echo	$(IDL_CC) $<
	@	$(IDL_CC) $<


.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

$(PROGRAM): $(OBJECTS) $(OBJ_EXT)
	@echo	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT)  $(LDFLAGS) $(LIBS)
	@	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS) 


# optional part

install:

RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

cleano:
	$(RM) $(OBJECTS)

cleanp:
	$(RM) $(PROGRAM)

