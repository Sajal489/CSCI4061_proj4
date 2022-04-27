#! /bin/bash

mkdir -p downloaded_files
./http_server server_files $PORT &
http_server_pid=$!

./testy test_http_server.org $testnum

kill -INT $http_server_pid
