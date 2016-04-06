gcc -std=c99 -D NDEBUG -O2 -I "impl/" -c impl/hash_cache.c impl/helper.c impl/lru_replacement.c
g++ -std=c++11 -O2 -fpermissive -pthread -w -fpermissive -I "impl/" -I "asio/include/" -o server hash_cache.o helper.o lru_replacement.o server.cpp #-lws2_32
gcc -std=c99 -D NDEBUG -O2 -w -c test/basic_test.c test/lrutests.c test/main.c test/test.c test/test_helper.c
g++ -std=c++11 -w -O2 -fpermissive -pthread -I "test/" -I "asio/include/" -o client_tests basic_test.o lrutests.o main.o test.o test_helper.o client.cpp #-lws2_32
