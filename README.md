# HW5

Files | descriptions
--- | ---
telnet.sh | allows basic testing of server without client
client.cpp | client code
make.sh | bash build script
run_test.py | testing suite script
user_con.hpp | shared code between client and server


### Correctness status

All of the tests pass except those that use unimplemented features, the testing helper framework I made that seems to have a bug or two, or assume that values can hold arbitrary data. Except "delete_not_in", which sometimes works and most of the time fails (even when working on localhost), so I don't know what is wrong with it.

### Network interface conformance status

Currently it used badly non-standard messages (especially for error codes). It assumes that all messages are a single line long. Unfortunately, I ran out of time to make it more standard, and more able to communicate with other people's servers.

### Performance status

There are an absurd number of data copies made between cache_get being called and the data being returned. This not ideal, but it leaves lots of room for improvement. It also is all happening relatively fast compared to the network interactions, so it does not really matter that much.

I tested performance by running the server on mandu and the client on ravioli and making 1000 get requests.

#### Results

* UDP = ~0.24ms per request
* TCP = ~0.39ms per request

These times show that UDP is about twice as fast as TCP, as expected.

### Experience with ASIO

Any library which uses a core concept that has an explicitly listed disadvantage of increased code complexity should be approached with caution (ASIO Documentation/Overview/Proactor design pattern/disadvantages). Some tasks which would have been trivial with raw sockets were painful without them. Notably, calling the underlying setsockopt required setting up an object with 4 functions which returned the arguments for setsockopt, instead of just calling a function.
