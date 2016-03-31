gcc -std=c99 -O2 -I "impl/" -c impl/hash_cache.c impl/helper.c impl/lru_replacement.c
g++ -std=c++11 -O2 -fpermissive -pthread -w -fpermissive -D ASIO_STANDALONE -I "impl/" -I "asio/include/" -o server hash_cache.o helper.o lru_replacement.o server.cpp
g++ -std=c++11 -w -O2 -fpermissive -pthread  -I "test/" -I "asio/include/" -o client_tests client.cpp test/basic_test.c test/lrutests.c test/main.c test/test.c test/test_helper.c

#./server -p 8230 -m 100000 > out.txt  &
#./client_tests > clout.txt &
