# HW5

Files | descriptions
--- | ---
telnet.sh | allows basic testing of server without client
client.cpp | client code



### Experience with ASIO

Any library which uses a core concept that has an explicitly listed disadvantage of increased code complexity should be approached with caution (ASIO Documentation/Overview/Proactor design pattern/disadvantages). Some tasks which would have been trivial with raw sockets were painful without them. Notably, calling the underlying setsockopt required setting up an object with 4 functions which returned the arguments for setsockopt, instead of just calling a function.

### Correctness status

All of the tests pass except those that use unimplemented features, the testing helper framework I made that seems to have a bug or two, or assume that values can hold arbitrary data. Except "delete_not_in", which sometimes works and most of the time fails (even when working on localhost), so I don't know what is wrong with it.

To ensure that working with huge values (megabytes) actually worked, I made a new test (number 12 get_huge_value), which sets and gets a single enormous value, and checks to see if it is correct.

### Memory leak status

There is one problem with the API that seems to require memory leaks. This is that when cache_get is called, the server copies a value to the client which then exposes that value to the user. The client cannot hold on to the data (or else the server cache would be pointless), and the user does not delete the data, as the cache is supposed to deal with its own memory.

There is one solution to this problem I can think of. Whenever cache_get is called, it stores a single value in somewhere, and returns the pointer to that. Then, when cache_get is called on a different key, it deletes the previous value from this spot and puts the new one in. So it is basically a client side cache that stores a single value. The only problem is that this means that any call to cache_get invalidates returned pointers, which was not assumed in many of the tests.  

### Performance status
