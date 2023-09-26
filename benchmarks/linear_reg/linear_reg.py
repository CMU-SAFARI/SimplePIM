num_threads=32
import os
from joblib import parallel_backend
os.environ["OMP_NUM_THREADS"] = str(num_threads)
os.environ["OPENBLAS_NUM_THREADS"] = str(num_threads) 
os.environ["MKL_NUM_THREADS"] = str(num_threads) 
os.environ["BLIS_NUM_THREADS"] = str(num_threads) 

import time
import random
import numpy as np
import pandas as pd
import torch
from torch import float32
from torch.autograd import Variable
from torch.nn.functional import linear
random.seed(10)
np.set_printoptions(precision=4)
torch.set_printoptions(precision=4)
torch.set_default_dtype(float32)
torch.set_num_threads(num_threads)

class linearRegression(torch.nn.Module):
    def __init__(self, inputSize, init_weight):
        super(linearRegression, self).__init__()
        self.inputSize = inputSize
        self.weights = init_weight
        self.criterion = torch.nn.MSELoss(reduction='mean') 

    def forward(self, x, y):
        out = torch.squeeze(linear(x, self.weights))
        loss = self.criterion(out, y)
        return loss

def main():
    num_dpus = 5
    dim, num_elements, iter, lr = 10, 1000*num_dpus, 1, 1e-4

    df = pd.DataFrame([dim, num_elements, iter, lr])
    init_vector = np.zeros((dim), dtype=np.float32)
    input = np.zeros((num_elements, dim+1), dtype=np.float32)

    groud_truth = np.zeros((dim), dtype=np.float32)
    for i in range(dim):
        groud_truth[i] = random.randint(-2, 2) 

    for i in range(num_elements):
        for j in range(dim):
            r1, r2 = random.uniform(0, 1), random.uniform(0, 1)/dim
            input[i][j] = (int)((i-num_elements/2)*r1 + j*r2)%10 if j%2 == 0 else (int)(-1*((i-num_elements/2)*r1 + j*r2))%10
        input[i][dim] = groud_truth.dot(input[i][:-1])
    
 
    
    #np.savetxt("data/args.csv", np.array([dim, num_elements, iter, lr]), delimiter=",", fmt='%s')
    np.savetxt("data/input.csv", input, delimiter=",", fmt='%f')

    x_train, y_train = (input.transpose(1, 0)[0:-1]).transpose(1, 0), (input.transpose(1, 0)[-1])

    if torch.cuda.is_available():
        inputs = Variable(torch.from_numpy(x_train).cuda(), requires_grad=True)
        labels = Variable(torch.from_numpy(y_train).cuda(), requires_grad=True)
        init_weights = torch.nn.Parameter(Variable(torch.from_numpy(init_vector).cuda(), requires_grad=True))
    else:
        inputs = Variable(torch.from_numpy(x_train), requires_grad=True)
        labels = Variable(torch.from_numpy(y_train), requires_grad=True)
        init_weights = torch.nn.Parameter(Variable(torch.from_numpy(init_vector), requires_grad=True))
    

    model = linearRegression(dim, init_weights)
    optimizer = torch.optim.SGD(model.parameters(), lr=lr)

    start = time.time()
    for epoch in range(iter):
    # Clear gradient buffers because we don't want any gradient from previous epoch to carry forward, dont want to cummulate gradients
        optimizer.zero_grad()

    # get output from the model, given the inputs
        loss = model(inputs, labels)
        loss.backward()
        
        # update parameters
        optimizer.step()
    end = time.time()

    t = (end-start)*1000
    print("the time consumed is "+str(t)+"ms")
    print("linear model weights: ")
    print(model.weights.detach().numpy())

    print("groud truth: "+str(groud_truth))

    '''
    print("$$$$$")
    print((x_train[0].dot(init_vector)-y_train[0])*x_train[0])
    print((x_train[1].dot(init_vector)-y_train[1])*x_train[1])
    print((x_train[0].dot(init_vector)-y_train[0])*x_train[0] + (x_train[1].dot(init_vector)*init_vector-y_train[1])*x_train[1])
    print((x_train@init_vector-y_train)@x_train)
    '''
    if not os.path.exists("results/"):
        os.makedirs("results/")

    path = 'results/cpu_'+str(dim)+"_"+str(num_elements)+".csv"
    np.savetxt(path, np.array([t]))

if __name__ == "__main__":
    if not os.path.exists("data/"):
        os.mkdir("data/")
    with parallel_backend('threading', n_jobs=num_threads):
        main()
    
