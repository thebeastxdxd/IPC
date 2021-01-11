# Stages 

Stage1: They will need to create a hard coded white-list of process names. Using a named pipe 
they will check if a name exists in that list.

Stage2: After checking if the process name is included is in the white-list they will check if there is a 
running process that includes the name. They will need to fork and execute ps, then parse its output.

Stage3: Now they need to write a server which contains the actaul white-list. Using unix sockets
their code will request the whitelist from the server.
They will need to create a message API so that they can communicate between the server and the client.

stage4->: ideas
1. instead of running the binary ps, they can read and parse proc.
2. implement shared memory between the client and the server so that they can transfer more data in
an easier/faster way

# Notes
first thing they will get is a small explanation
about mkfifo in the shell.

1. open 2 shells, in one write 
```bash
mkfifo tester # creates a backing file named tester
cat tester # type the pipe's contestns to stdout
```
in the other shell
```bash
cat > tester # redirect keyboard input to the pipe
```


