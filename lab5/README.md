# CS360 Lab 5

Networking

## Running the Program

Compile the project by running `make`. This will compile the two programs into the `bin` directory.

From there, `cd` into the `bin` directory and run each file in a different terminal window using `sudo ./server.bin` and `sudo ./client.bin`

**Note:** make sure to run `sever.bin` before `client.bin` or the client won't be able to connect and will terminate.

## Key Concepts

### TCP Socket Lifecycle

```
Server                              Client
------                              ------
socket()                            socket()
bind(IP:port)                       connect(server_IP:port)
listen()                            ↕ (3-way handshake completes)
accept() ←─────────────────────────
  │                                 write(cmd)
  read(cmd) ──────────────────────→
  write(response)                   read(response) ...loop...
  write(EOT)                        is_end_of_transmission?
close()                             close()
```

### File Transfer Protocol (size-prefix + chunked)

`get` and `put` use a simple protocol to handle files larger than one network read:

1. **Sender** writes the file size as an ASCII decimal string (MAX bytes)
2. **Receiver** reads and parses the size
3. **Sender** writes file data in MAX-byte chunks
4. **Receiver** reads until `bytes_remaining == 0`

This is necessary because TCP is a stream protocol — `read()` may return fewer bytes than requested, so you can't rely on a single `read()` getting the whole file.

### Permission Bit Decoding

`st_mode` from `stat()` encodes both file type and permissions in one integer:

```
Bits 15-12: file type  (0x8000=regular, 0x4000=dir, 0xA000=symlink)
Bits  8- 0: rwx bits   (bit 8=owner-r, bit 7=owner-w, bit 6=owner-x, ...)
```

The `ls_file()` function decodes this by masking the top bits for type, then iterating bits 8→0 for permissions.

### Security Note

`chroot("./")` in server.c confines the server's filesystem view to its launch directory, preventing clients from accessing files outside that tree via path traversal (`../../etc/passwd`). However, `chroot` alone is not a complete security boundary.

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
