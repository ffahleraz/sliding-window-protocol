<img src="logo.png">

Sliding Window Protocol
&middot;
[![GitLab license](https://img.shields.io/github/license/Day8/re-frame.svg)](LICENSE)
=====
The sliding window protocol is a feature of datagram-oriented transmission protocols. Sliding window protocols are used where reliable in-order delivery of packets is required. In this case, we implemented a sliding window protocol to ensure in-order reliable transmission for UDP sockets.

## Table Of Contents
- [Introduction](#introduction)
- [Installation](#Installation)
- [How Sliding Windows Protocol Works](#how-sliding-windows-protocol-works)
- [Run Simulation](#run-simulation)
- [Documentation](#documentation)
- [Authors](#authors)
- [Words from Authors](#words-from-authors)
- [References](#references)

## Introduction
This is an implementation of a sliding window protocol using **C++**. It is open-source and everyone can contribute to this project by creating pull request.

## How Sliding Windows Protocol Works

## Installation
You need to install **g++** and run it on **Ubuntu** Operating System. List of build commands:
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
2. Start Sender
```
$ ./sendfile <filename> <window_size> <buffer_size> <ip> <port>
```
## Documentation
## Authors
1. Faza Fahleraz https://github.com/ffahleraz
2. Nicholas Rianto Putra https://github.com/nicholaz99
3. Abram Perdanaputra https://github.com/abrampers

## Words from Authors
Thanks to Mr. Achmad Imam Kistijantoro ST,M.Sc.,Ph.D. for his amazing project about *Lossles Data Transfer*

## References
* [Computer Networks: A Systems Approach (The Morgan Kaufmann Series in Networking) 5th Edition](https://www.amazon.com/Computer-Networks-Fifth-Approach-Networking/dp/0123850592)
