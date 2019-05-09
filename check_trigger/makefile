.SUFFIXES: .idl .cpp .o


PROGRAM = check_trigger

OBJECTS = check_trigger.o common.o dcisg.o


OBJ_EXT =


PROGRAM1 = export_trigger

OBJECTS1 = export_trigger.o  common.o dcisg.o


OBJ_EXT1 =


CC	= `makeopts.sh cc`
CCFLAGS = `makeopts.sh ccflag $(DATABASE)_inc` 
LDFLAGS = `makeopts.sh ldflag` 
LIBS	= `makeopts.sh sam para` -lpthread -lrt -lc -lm -lrte -lcomm -ldmdci -lservices
ifeq ("$(DB_TYPE)","kingbase")
	LIBS= `makeopts.sh sam para` -lpthread -lrt -lc -lm -lrte -lcomm -ldcikdb -lservices
else
	LIBS= `makeopts.sh sam para` -lpthread -lrt -lc -lm -lrte -lcomm -ldmdci  -lservices
endif

.idl.cpp:
	@echo	$(IDL_CC) $<
	@	$(IDL_CC) $<


.cpp.o:
	@echo	$(CC) $(CCFLAGS) $<
	@	$(CC) $(CCFLAGS) $<

all: $(PROGRAM)  $(PROGRAM1)

$(PROGRAM): $(OBJECTS) $(OBJ_EXT)
	@echo	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS) 
	@	$(CC) -o $(PROGRAM)  $(OBJECTS) $(OBJ_EXT) $(LDFLAGS) $(LIBS) 
	cp $(PROGRAM) $(SRC_HOME)/bin

$(PROGRAM1): $(OBJECTS1) $(OBJ_EXT1)
	@echo	$(CC) -o $(PROGRAM1)  $(OBJECTS1) $(OBJ_EXT1) $(LDFLAGS) $(LIBS) 
	@	$(CC) -o $(PROGRAM1)  $(OBJECTS1) $(OBJ_EXT1) $(LDFLAGS) $(LIBS) 
	cp $(PROGRAM1) $(SRC_HOME)/bin


# optional part

install:
	cp -rp $(SRC_HOME)/bin/$(PROGRAM) $(D5000_HOME)/bin
	cp -rp $(SRC_HOME)/bin/$(PROGRAM1) $(D5000_HOME)/bin

RM 	= rm -f

clean:
	$(RM) $(PROGRAM) $(OBJECTS)
	$(RM) $(PROGRAM1) $(OBJECTS1)

cleano:
	$(RM) $(OBJECTS)
	$(RM) $(OBJECTS1)

cleanp:
	$(RM) $(PROGRAM)
	$(RM) $(PROGRAM1)
