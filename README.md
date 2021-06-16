# Sea [![Build Status](https://travis-ci.com/ValHayot/Sea.svg?branch=master)](https://travis-ci.com/ValHayot/Sea) [![codecov](https://codecov.io/gh/ValHayot/Sea/branch/master/graph/badge.svg)](https://codecov.io/gh/ValHayot/Sea)
![Sea logo](https://lh3.googleusercontent.com/_QD6PrqDMG50G8crgmnIa75jB3I4xnX1cREsHU-aQ0eTqAwBn4UoJ-NFwmVS6Q2xEXFqT2nQCNuhBf64NpuqDErX20Fd6iEKPAI1prmr)


Sea can provide the speed up of [Apache Spark](http://spark.apache.org) but with no instrumentation.

It is hierarchical filesystem implementation using LD_PRELOAD, adapted from
the passthrough of [XtreemOS](https://github.com/xtreemfs/xtreemfs).

## Requirements (if building from source)
- gcc-c++
- libiniparser
- gtest (optional)
- bash

## Compilation

`make pass`
or 
`make` (to compile tests)

## Usage
### Configuration file
In order to successfully run Sea alongside your application, you will need to provide some details in an `.ini` file called `sea.ini`.
The following are the properties of the file:
- `mount_dir`: This is the folder your application will directly interact with to access files in Sea. It is the "view" to Sea.
- `n_sources`: The number of source directories in your file system. Source directories are the directories that you would like Sea to use in order
               to speed up computation (e.g. tmpfs directories, local disk and Lustre)
- `source_<X>` : The path of the source. `<x>` is a digit representing at which level of the hierarchy this file system should be found at (starts at 0 - top).
                 For instance, a tmpfs path should likely be at `source_0` and a Lustre path should be found at the bottom of the hierarchy (e.g. `source_2`
                 if you have a local disk fs at `source_1`). These directories must exist before launching the application.
- `log_level` : This is an interger representing the amount of verbosity you desire in you application. There are currently 5 levels (0:None, 1:Error,
                2: Warning, 3: Info, 4: debug).
- `log_file` : location of file in which to store the logs
- `max_fs` : Pipeline maximum file size in Bytes. It is currently required that the maximum file size is passed to Sea in order for Sea to be able to determine
             if there is sufficient space in the source directions.
- `n_threads`: The maximum number of application concurrent threads or processes. Also used to determine if there is sufficient space.

An example configuration file may look like:
```                                                   
[Sea]                                                                  
mount_dir = /lustre/seamount ;                                    
n_sources = 3 ;                                                        
source_0 = /dev/shm/seasource ;                                           
source_1 = /localscratch/seasource ;                                         
source_2 = /lustre/seasource ;                                    
log_level = 0 ;                                 
log_file = /lustre/sea.log ;                             
max_fs = 646971392 ;                                                   
n_threads = 16 ;    
```

### Flushlist

Sea relies on a file called `.sea_flushlist` to determine which files need to be be flushed from the upper-level sources to the bottom-level source directory.
Exact filenames need not be provided and regex is accepted (can be thought or as similar to a `.gitignore`!). All filepaths resolved by the regex specified in the flushlist file will be flushed and evicted whenever possible.


### Program execution

In order to launch your application with Sea, it is crucial to first set the `SEA_HOME` variable. This variable is used to indicated the folder in which the `sea.ini` and `.sea_flushlist` files are located.

The `sea_launch.sh` executable is used to launch the flushers and execute the program. However, your program must still set the `LD_PRELOAD` variable prior to execution in order to use the Sea file system. The LD_PRELOAD must be set with Sea's `passthrough.so`.

e.g.

`SEA_HOME=$PWD ./sea_launch.sh <myprogram>`

myprogram
```
#!/bin/bash
LD_PRELOAD=passthrough.so <myactualprogram>
```
