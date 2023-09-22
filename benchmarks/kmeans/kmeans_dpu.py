import numpy as np
import time
from dpu_kmeans import KMeans as DPUKMeans
from dpu_kmeans import _dimm
from sklearn.datasets import make_blobs
from sklearn.metrics import adjusted_rand_score, mean_squared_error

if __name__ == "__main__":
    num_dpus = 1024
    k, dim, num_elements, iter = 10, 10, num_dpus*10000, 1
    input, labels, real_centroids = make_blobs(num_elements, dim, centers=k, random_state=42, return_centers=True)
    real_centroids = real_centroids[np.lexsort(np.rot90(real_centroids))]
    _dimm.set_n_dpu(num_dpus)

    dpu_kmeans = DPUKMeans(k, n_init=1, verbose=False, max_iter=iter, tol=1e-4)
    dpu_kmeans.n_iter_ = iter

    start = time.time()
    dpu_kmeans.fit(input)
    end = time.time()
    t = (end-start)*1000.

    result = dpu_kmeans.cluster_centers_
    result = result[np.lexsort(np.rot90(result))]
    duration = dpu_kmeans.dpu_run_time_ * 1000.
    iterations = dpu_kmeans.n_iter_

    print("the time consumed is "+str(t)+" ms")
    print("the kernel time consumed is "+str(duration)+" ms")
    print("the number of iterations "+str(iterations))
    print("the reduction time "+str(dpu_kmeans.pim_cpu_time_*1000)+" ms")
    print("centroids of kmeans: ")
    print(result)
    print("real centroids: ")
    print(real_centroids)
    print("RMSE: ")
    print(mean_squared_error(result, real_centroids, squared=False))
    print("adjusted rand score: ")
    print(adjusted_rand_score(dpu_kmeans.labels_, labels))
