CFLAGS = -Wall -Werror -g
CC = gcc $(CFLAGS)
port = 8000

.PHONY: all test test-setup test-concurrent test-concurrent-setup clean zip

all: http_server concurrent_open.so

http_server: http_server.c http.o connection_queue.o
	$(CC) -o $@ $^ -lpthread

http.o: http.c http.h
	$(CC) -c http.c

connection_queue.o: connection_queue.c connection_queue.h
	$(CC) -c connection_queue.c

concurrent_open.so: concurrent_open.c
	$(CC) $(CFLAGS) -shared -fpic -o $@ $^ -ldl

test-setup:
	@chmod u+x testy
	@chmod u+x run_server_tests.sh

test: test-setup http_server clean-tests
	PORT=$(port) ./run_server_tests.sh

test-concurrent-setup:
	@chmod u+x testy
	@chmod u+x run_concurrent_server_tests.sh

test-concurrent: test-concurrent-setup http_server concurrent_open.so clean-tests
	PORT=$(port) ./testy test_concurrent_http_server.org

clean:
	rm -rf *.o concurrent_open.so http_server

clean-tests:
	rm -rf test-results
	rm -rf downloaded_files

zip:
	@echo "ERROR: You cannot run 'make zip' from the part2 subdirectory. Change to the main proj4-code directory and run 'make zip' there."
