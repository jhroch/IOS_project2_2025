CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -g
LDFLAGS = -pthread -lrt

all: proj2

proj2: proj2.c
	$(CC) $(CFLAGS) proj2.c -o proj2 $(LDFLAGS)

run: proj2
	./proj2 4 4 10 10 10 

test: run
	./kontrola-vystupu.sh < proj2.out

#python3.12 ./ios_proj2_test.py run ./proj2 1555 1555 100 10 10

clean:
	rm -f proj2 proj2.out