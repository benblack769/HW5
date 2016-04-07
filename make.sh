gcc-5 -std=c99  -O3 -I "impl/" -c impl/hash_cache.c impl/helper.c impl/lru_replacement.c
g++-5 -std=c++11 -O3 -pthread -Wall -I "impl/" -I "asio/include/" -o server hash_cache.o helper.o lru_replacement.o server.cpp #-lws2_32
gcc-5 -std=gnu99 -O3 -D TIME_GETS -w -c test/basic_test.c test/lrutests.c test/main.c test/test.c test/test_helper.c
g++-5 -std=c++11 -O3 -pthread -Wall -I "test/" -I "asio/include/" -o client_tests basic_test.o lrutests.o main.o test.o test_helper.o client.cpp #-lws2_32
