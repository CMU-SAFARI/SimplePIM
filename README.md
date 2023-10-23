# SimplePIM: A Software Framework for Productive and Efficient In-Memory Processing
This project implements SimplePIM, a software framework for easy and efficient in-memory-hardware programming. The code is implemented on UPMEM, an actual, commercially available PIM hardware that combines traditional DRAM memory with general-purpose in-order cores inside the same chip. SimplePIM processes arrays of arbitrary elements on a PIM device by calling iterator functions from the host and provides primitives for communication among PIM cores and between PIM and the host system. 

We implement six applications with SimplePIM on UPMEM: 
- Vector Addtition
- Reduction
- K-Means Clustering
- Histogram
- Linear Regression
- Logistic Regression

Previous manual UPMEM implementations of the same applications can be found in [PrIM Benchmark](https://github.com/CMU-SAFARI/prim-benchmarks), [dpu_kmeans](https://github.com/upmem/dpu_kmeans) and [prim-ml](https://github.com/CMU-SAFARI/pim-ml). These previous implementations can serve as baseline for measuring SimplePIM's performance as well as productivity improvements.

## Citation
Please cite the following papers if you find this repository useful.
Jinfan Chen, Juan Gómez-Luna, Izzat El Hajj, Yuxin Guo and Onur Mutlu, "[SimplePIM: A Software Framework for Productive and Efficient In-Memory Processing](https://arxiv.org/abs/2310.01893)" , International Conference on Parallel Architectures and Compilation Techniques (PACT), 2023.

Bibtex entries for citation:
```
@article{Chen2023SimplePIMPACT,
  title={SimplePIM: A Software Framework for Productive and Efficient Processing-in-Memory},
  author={Jinfan Chen and Juan G'omez-Luna and Izzat El Hajj and Yu-Yin Guo and Onur Mutlu},
  year={2023},
  booktitle = {PACT}
}
```

## Installation

### Prerequisites
Running PIM-ML requires installing the [UPMEM SDK](https://sdk.upmem.com). This benchmark designed to run on a server with real UPMEM modules, but they are also able to be run by the functional simulator in the UPMEM SDK.

### Getting Started
Clone the repository:
```
$ git clone https://github.com/CMU-SAFARI/SimplePIM.git
$ cd SimplePIM
```

## Repository Structure
```
.
+-- LICENSE
+-- README.md
+-- gitignore
+-- benchmarks/
|   +-- hist/
|   +-- kmeans/
|   +-- linear_reg/
|   +-- log_reg/
|   +-- red/
|   +-- va/
+-- lib/
|   +-- communication/
    |   +-- CommOps.c
    |   +-- CommOps.h
|   +-- management/
    |   +-- Management.c
    |   +-- Management.h
|   +-- processing/
    |   +-- gen_red
    |   +-- map
    |   +-- zip
```

## APIs 
SimplePIM provides three APIs to the users. The management interface is under SimplePIM/lib/management/. The management interface code sets up the UPMEM hardware, records and manages information about the PIM arrays. The communication interface under SimplePIM/lib/communication/ contains code for PIM-to-PIM and host-PIM communication operators (gather, scatter, broadcast, allreduce, and allgather). Finally, the processing interface under SimplePIM/lib/processing/ contains the UPMEM implementation of array map, array zip and array reduction. Many workloads like histogram, kmeans and vector addition can be abstracted as a combination of the communication and processing operators.

SimplePIM/lib/ contains other files that are helper functions for the ease of framework development.

## Running SimplePIM
Each benchmark folder includes Makefiles to run the experiments:
To run vector addition, redcution and histogram, one could simply go to each benchmark folder and run make. For example, to run vector addition, one could run 
```
$ cd benchmarks/va
$ make
$ ./bin/host
```
One can observe that SimplePIM produces the exact same result as the CPU code. One can change the parameters (number of elements, number of DPU used) in the Param.h file.

To run Linear Regresion, Logitic Regression, and KMeans, one needs to generate the input data with a python script under each benchmark folder. For example, to run linear regression, one firstly needs to run 
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
If you have any suggestions for improvement, please contact georgcjf at gmail dot com. If you find any bugs or have further questions or requests, please post an issue at the [issue page](https://github.com/CMU-SAFARI/SimplePIM/issues).

## Acknowledgement
We acknowledge support from the SAFARI Research Group’s industrial partners, especially Google, Huawei, Intel, Microsoft, VMware, and the Semiconductor Research Corporation. This research was partially supported by the ETH Future Computing Laboratory and the European Union’s Horizon programme for research and innovation under grant agreement No. 101047160, project BioPIM (Processing-in-memory architectures and pro- gramming libraries for bioinformatics algorithms). This research was also partially supported by ACCESS – AI Chip Center for Emerging Smart Systems, sponsored by InnoHK funding, Hong Kong SAR.
