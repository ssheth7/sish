CC = cc
CFLAGS= -Wall -Werror -Wextra 
TARGET = sish
SOURCES = sish.o str.o builtins.o

debug: $(SOURCES)  
	$(CC) $(CFLAGS) $(SOURCES) -g -o $(TARGET)

$(TARGET): $(SOURCES) 
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

sish.o: sish.c sish.h
	${CC} ${CFLAGS} -g -c sish.c 
str.o: str.c str.h
	${CC} ${CFLAGS} -g -c str.c 
builtins.o: builtins.c builtins.h
	${CC} ${CFLAGS} -g -c builtins.c
clean:
	rm -f sish *.o *.core
