all:
	g++ -pthread -Wall -o ledsrv src/ledsrv.cpp src/common.cpp
	
clean:
	rm -f ledsrv
