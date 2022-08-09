# Chat
The program simulates a simple chat application, using shared memory and semaphores. Messages are sent from one process to another, passing through some intermediate processes that may modify the message. The checksum of the message is compared to its hash and if they are different, the message is sent again. The probability of the messsage changing is given as input from the user.

## Clean
```
make clean
```

## Compile
```
make
```
## Run - in the first shell
```
./p -p x
```
x stands for the probability of the message changing (e.g. 0.2)

## Run - in the second shell
```
./p
```
## A diagram illustrating the processes
![diagram](https://github.com/NefeliTav/Chat/blob/main/diagram.png?raw=true)
