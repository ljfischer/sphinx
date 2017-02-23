PROG = sphinx
CC = g++
CPPFLAGS = -g -pthread
LDFLAGS = -lwiringPi -lncurses -lstdc++ -lbluetooth -lm -pthread
OBJS = sphinx.o servo.o uart.o ranger.o cli.o bluet.o

$(PROG): $(OBJS)
	gcc $(LDFLAGS) -o $(PROG) $(OBJS)

sphinx.o: sphinx.cpp
	$(CC) $(CPPFLAGS) -c sphinx.cpp

servo.o: servo.cpp servo.h
	$(CC) $(CPPFLAGS) -c servo.cpp

uart.o: uart.cpp uart.h
	$(CC) $(CPPFLAGS) -c uart.cpp

ranger.o: ranger.cpp ranger.h
	$(CC) $(CPPFLAGS) -c ranger.cpp

cli.o: cli.cpp cli.h
	$(CC) $(CPPFLAGS) -c cli.cpp

bluet.o: bluet.cpp bluet.h
	$(CC) $(CPPFLAGS) -c bluet.cpp

clean:
	rm -f core $(PROG) $(OBJS)

