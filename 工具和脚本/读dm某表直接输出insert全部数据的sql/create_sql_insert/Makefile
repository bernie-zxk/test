

.SUFFIXES: .o .cxx .cpp


CC=g++   

DEBUG=-g 
DEF= -D_MYSQL_  -D_ISQL_

D5000_INC=$(D5000_HOME)/src/include
D5000_LIB=$(D5000_HOME)/src/lib




LIBDIR=-L$(D5000_LIB)

#dm
LIBS=   -ldcisg_dm

#kingbase
#LIBS=    -ldcisg_kdb





BINDIR=../bin
VPATH=./src
INCDIR=-I./src   -I$(D5000_INC)
OUTDIR=./obj


.c.o .cpp.o:
	$(CC) $(DEBUG)  $(DEF)  $(INCDIR) -c $< -o $(OUTDIR)/$(@F)




CREATE_SQL_INSERT = create_sql_insert.o main.o
create_sql_insert:$(CREATE_SQL_INSERT)
	cd $(OUTDIR);$(CC)  $(DEBUG)  $(DEF)  -o $(BINDIR)/$@ $(LIBDIR) $(CREATE_SQL_INSERT) $(LIBS)



clean: 
	rm ./obj/*.o
