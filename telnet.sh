echo "POST /memsize/1234" | telnet localhost 9200
echo "PUT /thisisakey/thisisavalue" | telnet localhost 9200
echo "GET /thisisakey" | telnet localhost 9200
echo "HEAD /k" | telnet localhost 9200
echo "DELETE /k" | telnet localhost 9200
echo "POST /shutdown" | telnet localhost 9200
