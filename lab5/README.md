# CS360 Lab 5
Networking

## Dependencies
* gcc
* make

## Running the Program
To get started, first compile the project by running `make`. This will compile the two programs into the bin directory.

From there, `cd` into the bin directory and run each file in a different terminal window using `sudo ./server.bin` and `sudo ./client.bin`

**Note:** make sure to run `sever.bin` before `client.bin` or the client won't be able to connect and will terminate.

## Commands

All interaction with the system will be through the client window. Commands are separated into two genres: Server and Client.
### Server
All commands are exectued remotely on the server
* `get <filename>` : downloads a file from the sever.
* `put <filename>` : uploads a file to the server.
* `ls` : lists the files in current working directory of the server.
* `cd <dirname>` : changes the current working directory of the server.
* `pwd`: prints the working directory on the server.
* `mkdir <dirname>` : makes a new dir with name `<dirname>` on the server.
* `rmdir <dirname>` : removes dir with name `<dirname>` on the server.
* `rm <filename>`: removes file with name `<filename>` on the server.

### Client
All commands are executed locally on the client
* `lcat <filename>` : prints the content of a file to the console.
* `lls` : lists the files in current working directory of the client.
* `lcd  <dirname>` : changes the current working directory of the client.
* `lpwd` : prints the working directory of the client.
* `lmkdir <dirname>` : makes a new dir with name `<dirname>` on the client.
* `lrmdir <dirname>` : removes a dir with name `<dirname>` on the client.
* `lrm <filename>`: removes file with name `<filename>` on the client.

## Authors 
* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **Zach Nett** - [zjnet](https://github.com/zjnett)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)