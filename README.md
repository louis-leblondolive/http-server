# C HTTP Server 

![CI](https://github.com/louis-leblondolive/http-server/actions/workflows/ci.yml/badge.svg)

A simple HTTP server built in C from scratch.

>[!IMPORTANT]
>This project was made under macOS. Other platforms are not supported yet.
>HTTP version is HTTP/1.1, other versions such as HTTP/2 or HTTP/3 are not supported. 

## Main Features
- HTTP/1.1 support (GET, HEAD, OPTIONS)
- Request parsing and response generation with proper headers
- keep-alive connection supported 
- Static file serving (HTML, CSS, JS, Images...)
- Standard status codes (200, 404, 500...)
- Configuration file support (port, backlog, request maximal size...)


## Usage

Start the server :
```bash
./build/main
```

Then open your browser at `http://localhost:3490` (or whichever port is set in `config.h`).

Place your files in the `www/` directory and start the server. For example :

```text
www/
├── index.html
└── style.css
```
They will be accessible at `http://localhost:3490/index.html`, `http://localhost:3490/style.css`, etc.


## Build

### Prerequisites
- macOS (see [Important] notice above)
- `clang` or `gcc`
- `make`

### Compile
```bash
make
```

### Run
```bash
./build/main
```

> [!TIP]
> You can edit `src/config.h` before building to configure the port, backlog, etc.


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
│
└── Makefile
```

- **`src`**

    This directory contains all the server code. 

    - **`lib`**  
    
        This folder contains the server code, divided in two folders : 
        - `http` where the protocol is implemented
        - `net` where server execution and communication is handled

    - **`config.h`** 

        This files allows you to change server parameters, including : 
        - Port and backlog
        - Server name and version 
        - Default path to use when meeting a `/`request 
        - Request size parameters  

    - **`main.c`**

        The server entry point, which should remain untouched. 

- **`www`**
    
    This directory contains the static files that will be served to the client. By default, the server will try to send `www/index.html`, this can be overriden in `config.h`. 



## References
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
