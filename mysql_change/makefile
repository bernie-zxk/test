.SUFFIXES: .cpp .o

PROGRAM = mysql_test

OBJECTS = mysql_api.o \
	  sql_change.o \
	  test.o \

CC = g++   

INCLUDES =  -I . \
           -I /usr/include \

LIBS = -L /usr/lib64/mysql -L /usr/lib64/   \
       -L . \

LIB  = -lmysqlclient -lsql_anls

CCFLAGS = -g -Wall   
#all:$(PROGRAM)

.cpp.o:
	#@echo	$(CC) -c $< $(CCFLAGS)
	$(CC) -c $(CCFLAGS)	$<

$(PROGRAM) : $(OBJECTS)
	#@echo	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(LIB)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(LIB)

clean:
	rm -rf *.o    


