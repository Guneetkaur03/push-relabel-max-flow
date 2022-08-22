## Parallelising Push-Relabel Maximum Flow Algorithm

#### Name : Guneet Kaur
#### Student Number : 07919992

<hr>

To execute, type 
```make``` in the command line

Method 1:
```./max_flow -pe <num_of_threads>```
Method 2: 
Submit a job using 
```qsub myjob```

<hr>

_Synchronous implementation of push relabel maximum flow algorithm using locks in OpenMP_

<hr>

The algorithm works by generate a random graph of N vertices as network flow graph from a source to sink. The source is at 0 and sink is at N - 1.

Additionally, one can set random weights if random flag is set. Else, it will put weights as 1.

Push-Relabel algorithm is run on input graph.
