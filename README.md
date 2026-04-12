# C HTTP Server 

![CI](https://github.com/louis-leblondolive/http-server/actions/workflows/ci.yml/badge.svg)

A simple HTTP server built in C from scratch.

>[!IMPORTANT]
>This project was made under macOS. Other platforms are not supported yet.
>HTTP version is HTTP/1.1, other versions such as HTTP/2 or HTTP/3 are not supported. 

## Main Features
- HTTP/1.1 support (GET, HEAD, OPTIONS)
- Request parsing and response generation with proper headers
- Static file serving (HTML, CSS, JS, Images...)
- Standard status codes (200, 404, 500...)
- Configuration file support (port, backlog, request maximal size...)

## Build
- ...

## Repository Structure 
This repository has the following structure : 
```text
./
├── src/
│   ├── lib/
│   │   ├── net
│   │   └── http
│   ├── config.h
│   └── main.c
├── www/
│   ├── index.html
│   └── .../
├── build/
│   └── main
├── obj/
│   └── .../
└── Makefile
```

* **`src`**
This directory contains all the server code. 

    * **`lib`** folder 
    This folder contains the server code, divided in two folders : 
    - `http` where the protocol is implemented
    - `net` where server execution and communication is handled




## References
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)