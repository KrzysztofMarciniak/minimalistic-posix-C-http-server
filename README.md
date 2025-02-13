# Minimalistic POSIX C HTTP Server

A simple, minimalistic HTTP server written in C using POSIX system calls and pthreads. This server is designed to serve static HTML files based on defined routes.

## Features
- Handles HTTP GET requests.
- Serves HTML files defined in a routing configuration.
- Uses POSIX system calls for socket programming and file handling.
- Responds with a 404 Not Found message if the requested route is not found.

## Configuration
The server's behavior can be configured through the `config.h` file:
- The port number on which the server listens (`PORT`).
- A routing table that maps URL paths to corresponding HTML files.

### Example Routes
The routing table in `config.h` allows you to define custom routes. For example:
```c++
#ifndef CONFIG_H
#define CONFIG_H

#define PORT 80

typedef struct {
    char *route;
    char *filename;
} Route;

Route ROUTES[] = {
    {"/", "index.html"},
    {"/contact", "contact.html"},
    {NULL, NULL}  
};

#endif
```
## Usage
1. Compile the server:
```bash
make
```
2. Run the server:
```
sudo ./bin/mpchs
```
