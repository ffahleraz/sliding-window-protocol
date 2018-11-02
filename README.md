<img src="logo.png">

# Sliding Window Protocol
&middot;
[![GitLab license](https://img.shields.io/github/license/Day8/re-frame.svg)](LICENSE)
=====
The sliding window protocol is a feature of datagram-oriented transmission protocols. Sliding window protocols are used where reliable in-order delivery of packets is required. In this case, we implemented a sliding window protocol to ensure in-order reliable transmission for UDP sockets.

## Table Of Contents
- [Introduction](#introduction)
- [How The Sliding Windows Protocol Works](#how-the-sliding-window-protocol-works)
- [Documentation](#documentation)
- [Installing](#installing)
- [Running](#running)
- [Simulating Packet Loss](#simulating-packet-loss)
- [Authors](#authors)
- [Words from Authors](#words-from-authors)
- [References](#references)

## Introduction
This is an implementation of a sliding window protocol on **UDP socket** using **C++**. It is open-source and everyone can contribute to this project by creating a pull request.

## How The Sliding Window Protocol Works
<img src="sliding-window-protocol.gif">

We implemented the Sliding Window Protocol using Selective Repeat Protocol that requires the receiver to receive out of order data. Because of this requirement, the receiver must be able to buffer the packets. This protocol retransmits the data under 2 condition:
1. The receiver never received the packet, or the packet is recieved but the Acknowlendgement (ACK) of that packet never reaches the sender. We assume that the packet or the ACK is lost in transmission, hence the sender retransmits the packet after a timeout.
2. The receiver received the packet, but the packet is invalid because it has a wrong checksum. Then the receiver sends a Negative Acknowledgement (NAK) to notify the sender to begin retransmission immediately.

### Packets and Buffers
Data sent using the sliding window protocol is divided into packets (frames) that has a fixed maximum length. Each packet has a unique sequence number that is used reorder the packets into the complete data once the transmission is complete. Since the data being sent can be quite large, the data is first divided into buffers and the transmission is done in a per-buffer basis. In each buffer, the packets are given a sequence number that starts from zero. Hence, to disambiguate between buffers at the reciever, the buffer size must be more than twice the window size.

### Sender
The sender defined a window that is marked by the Last Acknowledgement Received (LAR) and Last Frame Sent (LFS) variables. At first, the sender sends all the packets in the window and waits for each of their corresponsing ACKs. Everytime an ACK is recieved, the sender will shifts its window until the smallest un-ACK-ed sequence number is at LAR + 1. Notice that the window is not shifted everytime an ACK arrives, this is because ACKs can arrive in out of order. That is why the window is shifted only if it is possible.

### Receiver
The receiver defined a window that is marked by the Last Frame Received (LFR) and Largest Acceptable Frame (LAF) variables. When the receiver accepts a frame, it will sends and ACK of that frame if the checksum is valid, and else it will sends a NAK.
Then, just like the sender, the receiver will shifts its window until the smallest unreceived sequence number is at LFR + 1.

## Documentation
In our program, we implemented the sliding window protocol to reliably transmit data over a **UDP socket** using **C++**. We create helper functions that are responsible to create and read frame, and create and read ACKs. We also implemented the sliding window in each corresponding file (`recvfile.cpp` and `sendfile.cpp`) that handles everything from sending ACK if the frame is valid, resending the frame if timeout occurs, and resend the frame right away if NAK is received.

### sendfile
We use two threads on our `sendfile.cpp` in order to implement the sliding window protocol. The first thread is to send the packets to the client, while the other one is to receive ACK or NAK from the client. We run **listen_ack()** function on `sendfile.cpp` on a new thread. Using multiple threads, our sendfile could send frames and receive ACK on the same time.
We also implement a mutex lock in order to maintain the integrity of the data that was used parallelly between two threads.

### recvfile
On our `recvfile.cpp`, we will receive frame from the sendfile, check whether the frame that recvfile received is correct or not using checksum, and send ACK/NAK to the sendfile. Recvfile will write buffer per buffer into a new file.

## Installing
You need to install **g++** and run it on **Ubuntu** or **macOS**. List of build commands:
1. Make
```
$ make
```
2. Clean executable file
```
$ make clean
```

## Running
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

## Simulating Packet Loss
To test the ability of retaining integrity and order of the transmission in the case of a packet loss, we can simulate a packet loss using this command on Linux:
```
$ sudo tc qdisc add dev lo root netem loss <loss_rate>
```
example:
```
$ sudo tc qdisc add dev lo root netem loss 50%
```
to reset:
```
$ sudo tc qdisc change dev lo root netem loss 0%
```

> `buffer_size` and `window_size` on both recvfile and sendfile must be the same. 

## Authors
1. Faza Fahleraz https://github.com/ffahleraz
2. Nicholas Rianto Putra https://github.com/nicholaz99
3. Abram Perdanaputra https://github.com/abrampers

## Words from Authors
Thanks to Mr. Achmad Imam Kistijantoro ST,M.Sc.,Ph.D. for his amazing project about *Lossles Data Transfer*

## References
* [Computer Networks: A Systems Approach (The Morgan Kaufmann Series in Networking) 5th Edition](https://www.amazon.com/Computer-Networks-Fifth-Approach-Networking/dp/0123850592)
