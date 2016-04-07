# HW5

Files | descriptions
--- | ---
telnet.sh | allows basic testing of server without client
client.cpp | client code

### Expirence with ASIO

Any library which uses a core concept that has an explicitly listed disadvantage of increased code complexity should be approached with caution (ASIO Documentation/Overview/Proactor design pattern/disadvantages). Some tasks which would have been trivial with threads were painful without them. Notably, placing a timeout on UDP receive calls requires a whole asynchronous deadline system even though the underlying socket does not require this.

### Correctness status

All of the tests pass except those that use unimplemented features, the testing helper framework I made that seems to have a bug or two, or assume that values can hold arbitrary data. Except "delete_not_in", which works when I run it in the debugger and not when it runs on its own. So I don't know what is wrong with it.

To ensure that working with huge values (megabytes) actually worked, I made a new test (number 12 get_huge_value), which sets and gets an enormous value.

### Memory leak status

There is one problem with the API that seems to require memory leaks. 

### Perfomance status
