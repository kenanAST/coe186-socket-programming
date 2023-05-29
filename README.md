# Socket Programming with AES Encryption

This repository provides an example implementation of socket programming using a Python server and clients in Python, C, C++, and Java. It also incorporates AES encryption for secure communication between the server and clients.

## Prerequisites

Before running the server and clients, ensure that you have the following software installed:

- Python (version 3.x) with the `pycryptodome` library installed
- C compiler (for C and C++ clients) with the OpenSSL library installed
- Java Development Kit (JDK) (for Java client)

## Setup

1. Clone this repository to your local machine or download the source code as a ZIP file.
2. Ensure you have the required dependencies for each programming language:
   - Python: Install the `pycryptodome` library using `pip`:
     ```
     pip install pycryptodome
     ```
   - C and C++: Install OpenSSL on your system. Refer to the OpenSSL documentation or your system's package manager for installation instructions.
   - Java: No additional dependencies required.
3. Compile the C and C++ clients using the OpenSSL library. For example, to compile the C client:
gcc client.c -o client -lssl -lcrypto

To compile the C++ client:
g++ client.cpp -o client -lssl -lcrypto

4. Modify the server and client configurations as needed. For example, you can change the server port number or the IP address in the client scripts.
