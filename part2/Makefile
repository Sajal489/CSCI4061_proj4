CFLAGS = -Wall -Werror -g
CC = gcc $(CFLAGS)

http_server: http_server.c http.o connection_queue.o
	$(CC) -o $@ $^ -lpthread

http.o: http.c http.h
	$(CC) -c http.c

connection_queue.o: connection_queue.c connection_queue.h
	$(CC) -c connection_queue.c

clean:
	rm -rf *.o http_server

zip:
	@echo "ERROR: You cannot run 'make zip' from the part2 subdirectory. Change to the main proj4-code directory and run 'make zip' there."
