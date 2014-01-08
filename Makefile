PKGS=gtk+-3.0 glib-2.0 gmodule-2.0
CFLAGS= -g3 `pkg-config --cflags $(PKGS)`
LDLIBS=`pkg-config --libs $(PKGS)` -lm -rdynamic
CC=gcc
CPPFLAGS= -DFOR_PILOT_COMPAT
OUT=moe

main: main.o app.o callbacks.o engine.o util.o scoring.o 
	$(CC) -o $(OUT) $^ $(LDLIBS) $(CPPFLAGS)

scoring.o: jstroke/scoring.c
	$(CC) -c -o scoring.o $(CFLAGS) $(LIBS) $(CPPFLAGS) -Ijstroke jstroke/scoring.c 

util.o: jstroke/util.c
	$(CC) -c -o util.o $(CFLAGS) $(LIBS) $(CPPFLAGS) -Ijstroke jstroke/util.c

#Construct the database from the stroke data
jdata.dat: jstroke/strokedata.h conv_jdata.pl
	perl conv_jdata.pl < jstroke/strokedata.h > jdata.dat

clean:
	rm *.o
	rm $(OUT)
