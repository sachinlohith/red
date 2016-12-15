# Implementation of **Random Early Detection (RED)** TCP congestion control algorithm.
*This program will only work on Linux or Unix systems*

Instructions :
- Clone the repository with `git clone https://github.com/sachinlohith/red` or download the `zip` file
- Go the the path of the repository with `$ cd ./red`
- Make `run` executable through `$ chmod a+x ./run`
- Run the program through the following command
```bash
$ ./run host_address port_no queue_size burst_size timeout
```
- Example : 
```bash 
$ ./run 127.0.0.1 57171 10000 10 5
```

Dependencies required to run:
- `gcc` version > 4.0
- `netinet` and `sys` Linux/Unix libraries
- `python` version > 2.7

> Note: Do not run as superuser
