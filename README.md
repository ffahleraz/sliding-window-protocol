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
- [Run Simulation](#run-simulation)
- [Documentation](#documentation)
- [Authors](#authors)
- [Words from Authors](#word-from-authors)
- [References](#references)

# Introduction
This is an implementation of a sliding window protocol on **UDP socket** using **C++**. It is open-source and everyone can contribute to this project by creating pull request.

## How Sliding Window Protocol Works
In our program, we implemented the program to actually do the sliding window protocol on **UDP socket** using **C++**. We create helper functions that are responsible to create and read frame, and create and read ACKs. We also implemented the sliding window in each corresponding file (`recvfile.cpp` and `sendfile.cpp`) that handles everything from sending ACK if the frame valid, resend the frame if timeout occurs, and resend the frame right away if NAK is received.


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

**BUFFER SIZE recvfile & BUFFER SIZE sendfile MUST BE SAME!!**

## Data
The default place to store data to send and store the received data is on the `data` folder.

## Documentation
### Authors
1. Faza Fahleraz https://github.com/ffahleraz
2. Nicholas Rianto Putra https://github.com/nicholaz99
3. Abram Perdanaputra https://github.com/abrampers

### Word from Authors
Thanks to Mr. Achmad Imam Kistijantoro ST,M.Sc.,Ph.D. for his amazing project about *Lossles Data Transfer*

### References
* [Computer Networks: A Systems Approach (The Morgan Kaufmann Series in Networking) 5th Edition](https://www.amazon.com/Computer-Networks-Fifth-Approach-Networking/dp/0123850592)
