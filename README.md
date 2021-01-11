# stages 


 stage1: they will need to create a hard coded white list, and using a named pipe
        they will check if a name exists in that list
stage2: after checking if it is in the white list they will check if there is a 
        running process that includes the name, by forking and executing ps, then parsing its output
stage3: they will now get the white list from a server connected via unix socket
        they will need to create a message API so that they can communicate nicely
stage4->: there are a lot more things they can do. 
          1. instead of using the binary ps, they can read from proc and parse it
          2. implement shared memory so that the server and client can transfer more data in
             an easier way
          3. 
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


