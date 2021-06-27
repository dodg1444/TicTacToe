# TicTacToe

A multiplayer Tic Tac Toe game over TCP.


## Installation

This application depends on the [Boost](https://www.boost.org/) library. On Debian-based distributions it can be installed with this command:

`sudo apt install libboost-all-dev`

### Client and Server

```
chmod +x install.sh
./install.sh
```

## Usage

### Client

```
./client.exe [hostname] [port] [username]
```
Running without params will try connecting to 127.0.0.1:6666 with Default username (can be changed in settings)

### Server

```
./server.exe [port]
```
Running without params will start listening on 127.0.0.1:6666
