#Intel(R) Xeon(R) Silver 4215 CPU @ 2.50GHz, 11264 KB cache
num_threads=32
import os
from joblib import parallel_backend
os.environ["OMP_NUM_THREADS"] = str(num_threads) 
os.environ["OPENBLAS_NUM_THREADS"] = str(num_threads)
os.environ["MKL_NUM_THREADS"] = str(num_threads) 
os.environ["BLIS_NUM_THREADS"] = str(num_threads)

import time
import random
import sys
import numpy as np
from sklearn.cluster import KMeans

np.set_printoptions(precision=32)
random.seed(10)
np.set_printoptions(threshold=sys.maxsize)

def distance(a, b):
    print(a-b)
    return np.sum((a-b)**2)

def main():
    num_dpus = 5
    k, dim, num_elements, iter = 10, 10, 1000*num_dpus, 1

    init_centroids = np.zeros((k, dim), dtype=np.int32)
    input = np.zeros((num_elements, dim), dtype=np.int32)

    for i in range(num_elements):
        for j in range(dim):
            r1, r2 = random.uniform(10, 100), random.uniform(0, 100)
            input[i][j] = (int)((i%100)*r1 + j*r2) if i%2 == 0 else (int)(-1*((i%100)*r1 + j*r2))
    
    for i in range(k):
        init_centroids[i] = input[i]
    

    np.savetxt("data/args.csv", np.array([k, dim, num_elements, iter], dtype=np.intc).astype(int), delimiter=",", fmt='%s')
    np.savetxt("data/input.csv", input, delimiter=",", fmt='%d')

    start = time.time()
    kmeans = KMeans(algorithm = "full", n_clusters=k, init=init_centroids, n_init=1, max_iter=iter, tol=0)
    kmeans.n_iter_ = iter

    kmeans.fit(input)
    end = time.time()

    t = (end-start)*1000
    print("the time consumed is "+str(t)+"ms")
    
    print("centroids of kmeans: ")
    print(np.rint(kmeans.cluster_centers_))

    print("number of iterations: "+str(kmeans.n_iter_))


    if not os.path.exists("results/"):
        os.makedirs("results/")

    path = 'results/cpu_'+str(dim)+"_"+str(num_elements)+"_"+str(k)+".csv"
    np.savetxt(path, np.array([t]))
    '''
    print("data points ")
    print(input)


    print("the labels of the data points")
    print(kmeans.labels_)

    print("number of iterations: "+str(kmeans.n_iter_))

    print("distance to centroid1 "+str(distance(init_centroids[1], input[6])))
    print("distance to centroid2 "+str(distance(init_centroids[2], input[6])))
    print("label: "+str(kmeans.labels_[6]))
    '''
    

if __name__ == "__main__":
    with parallel_backend('threading', n_jobs=num_threads):
        main()
    



