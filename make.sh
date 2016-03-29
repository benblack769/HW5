g++ -std=c++11 -O2 -pthread -w -fpermissive -D ASIO_STANDALONE -I "impl/" -I "asio/include/" -o server server.cpp impl/hash_cache.c impl/helper.c impl/lru_replacement.c
#g++ -std=c++11 -O2 -I "test/" -I "asio/include/" -o client_tests client.cpp test/basic_test.c test/lrutests.c test/main.c test/test.c test/test_helper.c
./server -p 9300 -m 100000
#./client_tests
