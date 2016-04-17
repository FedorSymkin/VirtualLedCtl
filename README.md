# VirtualLedCtl
This is a test task in base linux programming

Tiny C++ server for virtual "LED" (it visualized by string in console) and bash client for controlling it via FIFOs. 

Server and client should have access to /tmp folder.
Server has no exit command, it should be killed by CTRL+C

Client usage:


```sh
control commands:
./ledctl.sh on|off
./ledctl.sh color red|green|blue
./ledctl.sh rate 0|1|2|3|4|5

get state commands:
./ledctl.sh getstate
./ledctl.sh getcolor
./ledctl.sh getrate

get whole state:
./ledctl.sh
```

