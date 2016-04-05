echo "POST /memsize/1234" | telnet localhost 9202
echo "PUT /thisisakey/thisisavalue" | telnet localhost 9202
echo "GET /thisisakey" | telnet localhost 9202
echo "HEAD /k" | telnet localhost 9202
echo "DELETE /thisisakey" | telnet localhost 9202
echo "POST /shutdown" | telnet localhost 9202
