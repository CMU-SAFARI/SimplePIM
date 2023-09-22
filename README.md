# SimplePIM
This project implements SimplePIM, a software framework for easy and efficient in-memory-hardware programming. The code is implemented on UPMEM, an actual, commercially available PIM hardware that combines traditional DRAM memory with general-purpose in-order cores inside the same chip. SimplePIM processes arrays of arbitrary elements on a PIM device by calling iterator functions from the host and provides primitives for communication among PIM cores and between PIM and the host system. 

We implement six applications with SimplePIM on UPMEM: 
- Vector Addtition
- Reduction
- K-Means Clustering
- Histogram
- Linear Regression
- Logistic Regression

Previous manual UPMEM implementations of the same applications can be found in PrIM benchmark (https://github.com/CMU-SAFARI/prim-benchmarks), dpu_kmeans (https://github.com/upmem/dpu_kmeans) and prim-ml (https://github.com/CMU-SAFARI/pim-ml). These previous implementations can serve as baseline for measuring SimplePIM's performance as well as productivity improvements.

## Citation
Please cite the following papers if you find this repository useful.
Jinfan Chen, Juan Gómez-Luna, Izzat El Hajj, Yuxin Guo and Onur Mutlu, "SimplePIM: A Software Framework for Productive and Efficient In-Memory Processing", International Conference on Parallel Architectures and Compilation Techniques (PACT), 2023.

## Installation
Running PIM-ML requires installing the [UPMEM SDK](https://sdk.upmem.com). This benchmark designed to run on a server with real UPMEM modules, but they are also able to be run by the functional simulator in the UPMEM SDK.

Getting Started

Clone the repository:
```
$ git clone https://github.com/JinfanC/SimplePIM.git
$ cd SimplePIM
```

## Running SimplePIM
Each benckmark folder includes Makefiles to run the experiments:
To run vector addition, redcution and histogram, one could simply go to each benchmark folder and run make. For example, to run vector addition, one could run 
```
$ cd benchmarks/va
$ make
$ ./bin/host
```
One can observe that SimplePIM produces the exact same result as the CPU code. One can change the parameters (number of elements, number of DPU used) in the Param.h file.

To run Linear Regresion, Logitic Regression, and KMeans, one needs to generate the input data with a python script under each benckmark folder. For example, to run linear regression, one firstly needs to run 
```
$ cd benchmarks/linear_reg
$ python linear_reg.py
```
And then one can build and run the actual SimplePIM code as before with 
```
$ make
$ ./bin/host
```

## Getting Help
If you have any suggestions for improvement, please contact georgcjf at gmail dot com. If you find any bugs or have further questions or requests, please post an issue at the [issue page](https://github.com/JinfanC/SimplePIM/issues).
