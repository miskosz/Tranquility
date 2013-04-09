CC=g++
CFLAGS= -O4
LDFLAGS= -W -Wall

all: tranquility uniq_merge

tranquility: tranquility.cpp
	${CC} -o tranquility ${CFLAGS} tranquility.cpp ${LDFLAGS}

unique_merge: uniq_merge.cpp
	${CC} -o uniq_merge ${CFLAGS} uniq_merge.cpp ${LDFLAGS}

.PHONY: clean
clean:
	rm -f tranquility uniq_merge
