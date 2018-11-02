<img src="logo.png">

Sliding Window Protocol
&middot;
[![GitLab license](https://img.shields.io/github/license/Day8/re-frame.svg)](LICENSE)
=====
The sliding window protocol is a feature of datagram-oriented transmission protocols. Sliding window protocols are used where reliable in-order delivery of packets is required. In this case, we implemented a sliding window protocol to ensure in-order reliable transmission for UDP sockets.

# Table Of Contents
- [Introduction](#introduction)
- [Installation](#installation)
- [How Sliding Windows Protocol Works](#how-sliding-window-protocol-works)
- [Documentation](#documentation)
- [Run Simulation](#run-simulation)
- [Authors](#authors)
- [Words from Authors](#word-from-authors)
- [References](#references)

# Introduction
This is an implementation of a sliding window protocol on **UDP socket** using **C++**. It is open-source and everyone can contribute to this project by creating pull request.

## How Sliding Window Protocol Works
<img src="sliding-window-protocol.gif">

We implemented the Sliding Window Protocol using Selective Repeat Protocol that requires the receiver to receive out of order data. Because of this requirement, the receiver must be able to buffer the packets. This protocol retransmits the data under 2 condition:
1. The receiver received the packet, but the packet(s) are not ACKed. We assume that the packet(s) are lost in transmission, hence the sender retransmits the packet.
2. The receiver received the packet, but the packet(s) are invalid or rejected by the receiver. Then the receiver sends NAK to notify the sender to begin retransmission immediately.

Sender and receiver has their own respective window to mark Last Acknowledgement Received, Last Frame Sent, and Last Frame Received, Largest Acceptable Frame. First the sender sends all the sequence in the window and waits for each ACK. If the ACK was received, the sender will move the LAR and the LFS by one sequence. When the receiver accepts a frame, it will automatically move the LFR and LAF by one sequence and able to receive.

## Documentation
In our program, we implemented the program to actually do the sliding window protocol on **UDP socket** using **C++**. We create helper functions that are responsible to create and read frame, and create and read ACKs. We also implemented the sliding window in each corresponding file (`recvfile.cpp` and `sendfile.cpp`) that handles everything from sending ACK if the frame valid, resend the frame if timeout occurs, and resend the frame right away if NAK is received.

### sendfile
We use two threads on our `sendfile.cpp` in order to implement the sliding window protocol. The first thread is to send the packets to the client, while the other one is to receive ACK or NAK from the client. We run **listen_ack()** function on `sendfile.cpp` on a new thread. Using multiple threads, our sendfile could send frames and receive ACK on the same time.
We also implement a mutex lock in order to maintain the integrity of the data that was used parallelly between two threads.

### recvfile
On our `recvfile.cpp`, we will receive frame from the sendfile, check whether the frame that recvfile received is correct or not using checksum, and send ACK/NAK to the sendfile. Recvfile will write buffer per buffer into a new file.

## Installation
You need to install **g++** and run it on **Ubuntu**/**Mac** Operating System. List of build commands:
1. Make
```
$ make
```
2. Clean executable file
```
$ make clean
```

## Run Simulation
1. Start Receiver
```
$ ./recvfile <filename> <window_size> <buffer_size> <port>
```
example:
```
$ ./recvfile data/sample.txt 20 1000 5000
```
2. Start Sender
```
$ ./sendfile <filename> <window_size> <buffer_size> <ip> <port>
```
example:
```
$ ./sendfile data/received.txt 20 1000 127.0.0.1 5000
```

**NOTE:**

to simulate 50% packet loss, use this command:
```
$ sudo tc qdisc add dev lo root netem loss 50%
```

to reset:
```
$ sudo tc qdisc change dev lo root netem loss 0%
```

> `buffer_size` and `window_size` on recvfile and sendfile must be the same. 

## Data
The default place to store data to send and store the received data is on the `data` folder.

### Authors
1. Faza Fahleraz https://github.com/ffahleraz
2. Nicholas Rianto Putra https://github.com/nicholaz99
3. Abram Perdanaputra https://github.com/abrampers

### Word from Authors
Thanks to Mr. Achmad Imam Kistijantoro ST,M.Sc.,Ph.D. for his amazing project about *Lossles Data Transfer*

### References
* [Computer Networks: A Systems Approach (The Morgan Kaufmann Series in Networking) 5th Edition](https://www.amazon.com/Computer-Networks-Fifth-Approach-Networking/dp/0123850592)
