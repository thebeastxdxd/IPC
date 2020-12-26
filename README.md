# First Stage

### first thing they will get is a small explanation
### about mkfifo in the shell.

1. open 2 shells, in one write 
```bash
mkfifo tester # creates a backing file named tester
cat tester # type the pipe's contestns to stdout
```
in the other shell
```bash
cat > tester # redirect keyboard input to the pipe
```

### Then they will need to write a client and a server
### the client, should wait to recieve a message from the server telling him to update him
### the server will just be a loop that every ~5 seconds will ask the client for updates

