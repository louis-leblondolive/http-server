<h1 align="center">skiff</h1>
<p align="center"><i>A low level HTTP server implemented in C from scratch</i></p>

<p align="center">
  <img src="https://github.com/louis-leblondolive/skiff/actions/workflows/build.yml/badge.svg" alt="Build">
  <img src="https://github.com/louis-leblondolive/skiff/actions/workflows/test.yml/badge.svg" alt="Test">
  <img src="https://img.shields.io/github/v/release/louis-leblondolive/skiff" alt="Release">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/-Linux-FCC624?logo=linux&logoColor=black" alt="Linux">
  <img src="https://img.shields.io/badge/-macOS-000000?logo=apple&logoColor=white" alt="macOS">
</p>


## Table of Contents
- [Main Features](#main-features)
- [Build and Run](#build-and-run)
- [Usage](#usage)
- [Architecture & Internals](#architecture--internals)
    - [Server General Architecture](#server-general-architecture)
    - [Abstract Event System](#abstract-event-system)
    - [Request Parsing](#robust-request-parsing)
    - [Response Emission Optimization](#response-emission-optimization)
    - [Security](#security)
    - [System Reliability](#system-reliability--signal-handling)
- [Tests](#tests)
- [Benchmark](#benchmark)
    - [Current Performances]()
    - [Architectures Comparison]()
- [Known Limitations]()
- [Repository Structure](#repository-structure)
- [References](#references)



## Main Features
- **HTTP/1.1 support** and 15+ standard status codes for `GET`, `HEAD`, `OPTIONS`, and `POST`.
- **Concurrency model** : I/O multiplexing single-threaded (for now) architecture based on a `kqueue`/`epoll` abstraction.
- **Zero-copy send** : Kernel space file transfer via `sendfile()` for response optimization. 
- **Keep-Alive support** : Connection persistence using a ring buffer for allocation efficiency.
- **FSM parser** : Handling partial request reception.
- **Full static serving** : Served with proper MIME types and `If-Modified-Since` cache support.
- **CGI support** : Environment-based execution following RFC 3875 output constraints.
- **Security features** : Built-in `realpath` based protection against path traversal and buffer overflow.
- **Demo website** : Start the server and test it at `http://localhost:3490`.

>[!IMPORTANT]
>This project includes macOS and Linux support.
>HTTP version is **HTTP/1.1**, other versions such as HTTP/2 or HTTP/3 are not supported. 


## Build and Run 

### Prerequisites
- macOS or Linux (see [Important] notice above)
- `cc`
- `make`

### Installation 
```bash
git clone https://github.com/louis-leblondolive/http-server.git
cd http-server
```

### Compilation

The server features 2 compilation profiles :
- **`debug` (default)**: using ASan and compilation flags. Best for development and memory safety checks.
- **`release`**: compiling with optimization and without ASan. Best for benchmarks.

To compile, just type in :

```bash
make debug      # or simply 'make'
make release
```

> [!TIP]
> You can edit `src/config.h` before building to configure the port, backlog, etc.

### Run

You can run the generated binaries from the `build` directory, depending on the compilation profile you chose :

```bash
# For debuging and safety checks
./build/debug/main      

# For performance testing 
./build/release/main  
```

> [!TIP]
> Use the verbose mode (`-v`) to display debug information or the quiet mode (`-q`) to silence logs.
> (Colors supported). 

>[!NOTE]
>The **debug mode** might introduce a slight latency (approx. 40ms) due to ASan overhead. For better performance (<1ms response time), always use the **release mode**.


## Usage

Place your files in the `www/` directory. For example :

```text
www/
├── index.html
└── style.css
```

Then start the server (either on release or debug mode) :
```bash
./build/release/main
```

Open your browser at `http://localhost:3490` (or whichever port is set in `config.h`).


Your files will be accessible at `http://localhost:3490/index.html`, `http://localhost:3490/style.css`, etc.


## Architecture & Internals

### Server General Architecture 

`skiff` is based on a single threaded event-driven architecture:

```mermaid
flowchart LR
    N_evt([New Event]) 
    N_evt -->S_evt([Server Event])
    N_evt -->C_evt([Client Event])

    S_evt -->Acc([accept])
    Acc -->Reg["Register New Client"<br><small>READ Event</small>]

    C_evt -->C_R_evt([READ])
    C_evt -->C_W_evt([WRITE])
    C_evt -->C_T_evt([TIMER])

    C_R_evt -->Prcss[Process Request]
    Prcss -->Resp([Response fully sent ?]) 
    Resp -->|No|RegCW["Register Client"<br><small>WRITE Event</small>]
    Prcss -->KeepAlive([Keep Alive ?]) 
    KeepAlive -->|Yes|RegCR["Register Client"<br><small>READ Event</small>]

    C_W_evt -->Resp
    
    C_T_evt -->Close[Close Client Session]
```

The main advantage of `skiff`'s request processing logic is that both data reception and emission mechanisms are designed to be **non-blocking**. Therefore, the parser can postpone partial requests completion to future exchanges, and sending responses will not pause the server if the client socket is not ready. To do so, each client is assigned a session containing, in particular, parsing progression information and a queue for responses waiting to be sent. The request processing logic is designed as follows: 


```mermaid 
flowchart TD
    Data_Recv(["Data Reception"<br>Stored in ring buffer</br>]) 
    Empty_Buffer([Buffer is empty ?])
    Parse[Parse Data]
    Check[Check Result]
    Router[Router]
    Handler[Method-specific Handler]
    ErrHandler[Error Handler]
    Send[Sender]
    SendCPL([Send Complete ?])
    SendAddQ[Add to Send queue]
    Close([Connection Close ?])
    Done([Done])

    Data_Recv -->Empty_Buffer
    Empty_Buffer -->|Yes|Done
    Empty_Buffer -->|No|Parse
    Parse -->|Parser Stopped|Check
    Check -->|Error|ErrHandler 
    Check -->|Ok|Router
    Router -->Handler
    Handler -->Send
    ErrHandler-->Send
    Send -->SendCPL
    SendCPL -->|Yes|Close
    SendCPL -->|No|SendAddQ
    SendAddQ -->Close
    Close -->|Yes|Done
    Close -->|No|Empty_Buffer
```

An error during parsing is captured by the checker so as not to crash the entire process. 
>[!NOTE] The [parsing](#robust-request-parsing) and [server response](#response-emission-optimization) mechanisms are detailed in the sections below. 


### Abstract Event System 


### Robust Request Parsing 

Instead of using fragile string splitting, this server implements a Finite State Machine. This allows the server to pause and resume whenever data is partially received over the network. 

```mermaid
flowchart TD
    A([PARSING_METHOD]) --> |space found| B([PARSING_PATH])
    B --> |space found| C([PARSING_VERSION])
    C --> |'\r' found| D([EXPECTING_LF])
    D --> |'\n' found| E([PARSING_NEW_LINE])
    E --> |'\r' found| F([EXPECTING_FINAL_LF]) 
    E --> |char found| G([PARSING_HEADER_KEY]) 
    G --> |':' found| H([PARSING_HEADER_VALUE])
    H --> |'\r' found| D 
    F --> |body exists| I([PARSING_BODY])
    I --> |Content-Length reached| J([END_PARSING])
```


### Response Emission Optimization


### Security 

The server treats every input as hostile:

- **Path Sanitization**: Uses `realpath()` to resolve and verify that requested files are strictly within the `www/` jail.
- **Strict Buffer Limits**: Every parsing state (Method, URI, Headers) is guarded by customizable maximum lengths to prevent Buffer Overflow attacks.


### System Reliability & Signal Handling 

To ensure 100% uptime and clean resource management, the server implements:

- `SA_RESTART` **flags** : Prevents system calls (`accept`, `read`) from being interrupted by internal signals.
- **Atomic Signal Handlers**: Uses a non-blocking `waitpid` loop to reap child processes, preventing "zombie" accumulation.
- `errno` **Preservation**: Careful restoration of `errno` within handlers to avoid corruption of the main thread's state.





## Tests 

This project includes a Python tester used throughout the development to report and correct bugs. 

### Test Categories

The following error categories were tested : 
- Basic valid requests
- Error codes correctness
- Request format (malformed, oversized requests and headers issues)
- URI edge cases (path traversal or wrong path)
- Connection handling and response format 

### Tester Usage

#### Prerequisites

```bash
python3 --version   #Python 3.13.7 or >=
pip show rich | grep Version    #Version: 15.0.0 or >= 
```
#### Usage
```bash
python3 tester/test_runner.py
```

## Benchmark
Tested with `wrk -c 100` on Macbook Air (M2). 
Requests/sec:   4477.65
Transfer/sec:     26.08MB

`fork()` causes Requests/sec to be quite low (due to memory duplication), but it also reinforces safety by isolating processes from one another.

## Repository Structure 
This repository has the following structure : 
```text

./
├── src/
│   ├── lib/
│   │   ├── http
│   │   ├── net
│   │   └── utils
│   ├── config.h
│   └── main.c
├── www/
│   ├── cgi-bin/
│   │   └── .../
│   ├── index.html
│   └── .../
├── tester/
│   ├── test_runner.py
│   └── test_suite.py
│
└── Makefile
```

- **`src`**

    This directory contains all the server code. 

    - **`lib`**  
    
        This folder contains the server code, divided in two folders : 
        - `http` where the protocol is implemented (FSM parser, Router, Response)
        - `net` where server execution and communication is handled (Socket setup and Listening loop)
        - `utils` where various tools are implemented 

    - **`config.h`** 

        This file allows you to change server parameters, including : 
        - Port and backlog
        - Server name and version 
        - Default path to use when meeting a `/`request 
        - Request size parameters  

    - **`main.c`**

        The server entry point, which should remain untouched. 

- **`tester`**

    This folder contains a Python tester used to report bugs during development. The `test_runner.py` file 
    runs all tests contained in `test_suite.py`.

- **`www`**
    
    This directory contains the static files that will be served to the client. By default, the server will try to send `www/index.html`, this can be overriden in `config.h`. 

    Place your executables in the `cgi-bin` folder.

>[!CAUTION]
>The server will try to run CGI scripts with `execl`. Make sure your scripts are either compiled or include a 
>relevant shebang.


## References
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [RFC 9112 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc9112)
- [The C10K Problem](https://www.kegel.com/c10k.html)