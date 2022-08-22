/* PROJECT
 * Comp 7850 - Parallel Push Relabel Max Flow
 * Student Name - Guneet Kaur
 * Student ID - 07919992
 * Synchronous implementation of push relabel maximum
 * flow algorithm using locks in OpenMP
 * Max flow from source to sink
 * Source - 0
 * Sink - (N-1)
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "omp.h"

/* Constants */
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define INF INT_MAX

/* 
 * For queue 
 * to store active vertices
 * implemented as FIFO 
 */
int queue[INF]; 
int rear = - 1;
int front = - 1;
int itemCount = 0;

/* for graph size */
int N;

/* locks */
omp_lock_t queue_lock, excessflow_lock;

/* Prototypes */
void show(); // to display the queue
int dequeue(); // to remove from FIFO queue
void enqueue(int element); // to add element in the queue

void init_flow(int *flow); // initialize flow network
void init_height(int *height); // initialize height of vertices
void init_excess_flow(int *excess_flow); // initialize excess flow of vertices

void create_graph(int *G, int N, int random_flag); // create random graph

void relabel(int u, int *flow, int *C, int *height); //perform relabel for node u
bool push(int u, int *flow, int *C, int *excess_flow, int *height); //perform push for node u

// to display the queue
void show()
{
    if (front == - 1)
        printf("No active vertex \n");
    else
    {
        printf("\nActive vertices: ");
        for (int i = front; i <= rear; i++)
            printf("%d ", queue[i]);
        printf("\n");
    }
}

// to remove from FIFO queue
int dequeue()
{
    if (front == - 1 || front > rear)
    {
        printf("No active vertex \n");
        front = rear = -1;
        return -1;
    }
    else
    {
        int element = queue[front];
        front++;
        itemCount--;
        return element;
    }
} 

// to add element in the queue
void enqueue(int element)
{
    if (rear == INF - 1)
       printf("\nMax size of queue reached\n");
    else
    {
        if (front == - 1)
        {
            front = 0;
        }
        rear++;
        queue[rear] = element;
        itemCount++;
    }
} 

// initialize flow network
void init_flow(int *flow)
{
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            flow[i * N + j] = 0;
        }
    }
}

// initialize height of vertices
void init_height(int *height)
{
    for(int i = 0; i < N; i++)
    {
        height[i] = 0;
    }
}

// initialize excess flow of vertices
void init_excess_flow(int *excess_flow)
{
    for(int i = 0; i < N; i++)
    {
        excess_flow[i] = 0;
    }
}

// create random graph
void create_graph(int *G, int N, int random_flag)
{
    int source = 0;
    int sink = N - 1;

    //to set all the edge weights = 0
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            G[i * N + j] = 0;
        }
    }

    //number of paths to generate
    int num_paths = (rand() % (int)(N/2)) + 1; 

    int generate[N-2];

    for(int i = 0; i < num_paths; i++)
    {
        for (int j = 0; j < N - 2; j++)
        {
            generate[j] = j + 1;
        }

        // length of each path , can vary from 2 to n-2
        int length_path = (rand() % (N - 2)) + 1;

        //shuffle nodes
        for (int k = 0; k < N - 2; k++)
        {
            int u = rand() % (N - 2);
            int v = rand() % (N - 2);
            int swap = generate[u];
            generate[u] = generate[v];
            generate[v] = swap;
        }

        //connecting source to first node
        if(!random_flag)
        {
            G[source * N + generate[0]] = 1;
        }
        else
        {
            G[source * N + generate[0]] = (rand() % (N + length_path))+ 1;
        }
        

        for (int k = 0; k < length_path - 1; k++)
        {
            if(!random_flag)
            {
                G[generate[k] * N + generate[k + 1]] = 1; 
            }
            else
            {
                G[generate[k] * N + generate[k + 1]] = (rand() % (N + length_path)) + 1;
            }
        }

        if(!random_flag)
        {
            G[generate[length_path - 1] * N + sink] = 1;
        }
        else
        {
            G[generate[length_path - 1] * N + sink] = (rand() % (N + length_path)) + 1;
        }
        
    }
}

//perform relabel for node u
void relabel(int u, int *flow, int *C, int *height)
{
    int mh = INF;

    //1. find its neighbors
    for(int i = 0; i < N; i++)
    {
        //if flow of that edge is already equal to its capacity, no more flow can occur
        if((C[u * N +i] - flow[u * N + i]) > 0)
        {
            if(height[i] < mh)
            {
                mh =  height[i];
            }
         }
    }
    omp_set_lock(&queue_lock); //acquire lock to add active vertex in queue, if any
    height[u] = mh + 1; //relabel 
    enqueue(u); //add in queue
    omp_unset_lock(&queue_lock); //release lock
}

//perform push for node u
bool push(int u, int *flow, int *C, int *excess_flow, int *height)
{
    //1. find its neighbors
    for(int v = 0; v < N; v++)
    {
        if(height[u] == height[v] + 1 && ((C[u * N + v] - flow[u * N + v]) > 0))
        {

            omp_set_lock(&excessflow_lock); //acquire lock for performing a push

            //perform flow, it can be min of excess flow and edge capacity
            int f = min(C[u * N + v] - flow[u * N + v], excess_flow[u]);

            excess_flow[u] -= f; //reduce excess flow of vertex u
            excess_flow[v] += f; //increase excess flow of vertex v

            flow[u * N + v] += f;  //add to flow
            flow[v * N + u] -= f;  //add residual flow

            omp_unset_lock(&excessflow_lock); //release lock after performing a push

            omp_set_lock(&queue_lock); //acquire lock to add active vertex in queue, if any
            if (excess_flow[u] > 0) { 
                enqueue(u); // u becomes active vertex
            }
            if (excess_flow[v] > 0) { 
                enqueue(v); // v becomes active vertex
            }
            omp_unset_lock(&queue_lock); //release lock after adding vertex in queue.
        
            return true;
        }       
    }
    return false;
}


int main(int argc, char *argv[])
{
    int i, j;

    N = 500; //atoi(argv[1]); //take graph size as input from user

    double start_time; // use these for timing
    double stop_time;

    //instantiate the locks
    omp_init_lock(&queue_lock);
    omp_init_lock(&excessflow_lock);

    //for capacity graph
    int* G;
	G = (int*) malloc(N * N * sizeof(int));

    //for flow network
    int* flow;
	flow = (int*) malloc(N * N * sizeof(int));

    //height and excess flow of vertices
    int height[N], excess_flow[N]; 

    int source = 0;
    int sink = N - 1;

    /* 
     * Preflow Initialization
     */    
    init_height(height); //initialize height of every vertex -> 0
    height[source] = N; //set the height of source vertex -> N

    init_excess_flow(excess_flow);  //initialize excess flow of every vertex -> 0
    
    init_flow(flow); //initialize flow of every edge -> 0
    
    /* 
     * creating graph
     * we can manipulate 
     * the density of graph by 
     * changing the number of edges
     */ 
    create_graph(G, N, 1);

    start_time = omp_get_wtime(); // can use this function to grab a
                                  // timestamp (in seconds)

    //perform the first step of saturating the edges adjacent to source
#pragma omp parallel for 
    for(i = 0; i < N; i++)
    {
        if(G[source * N + i] != 0)
        {
    #pragma omp critical
            //send the flows
            flow[source * N + i] += G[source * N + i];

            //add residual capacity
            flow[i * N + source] -= G[source * N + i];

            excess_flow[source] -= G[source * N + i];
            excess_flow[i]  += G[source * N + i];
        }
    }   

    for (int i = 0; i < N; i++) 
    { 
        if (i != source && i != sink && excess_flow[i] > 0) 
        { 
            enqueue(i); 
        }
    }

    // printf("\n\nexcess flow of vertices are: \n");
    // for(i = 0; i  < N; i++)
    // {
    //     printf("%d ", excess_flow[i]);
    // }

    // printf("\n\nHeight of vertices are: \n");
    // for(i = 0; i  < N; i++)
    // {
    //     printf("%d ", height[i]);
    // }

    //show(); //queue after preflow

    int u;
    while(itemCount > 0)
    {
#pragma omp parallel for private(u) schedule(static, 1)
        for(i=0; i < itemCount; i++)
        {
            omp_set_lock(&queue_lock);
            u = dequeue();
            omp_unset_lock(&queue_lock);

            //printf("\nThread %d working on vertex %d\n", omp_get_thread_num(), u);
            while(u != source && u != sink && excess_flow[u] > 0)
            {
                if(!push(u, flow, G, excess_flow, height))
                {
                    relabel(u, flow, G, height);
                }
            }
        }
    }

    printf("\n\nexcess flow of vertices are: \n");
    for(i = 0; i  < N; i++)
    {
        printf("%d ", excess_flow[i]);
    }

    stop_time = omp_get_wtime();

    printf("\n\nMax flow is %d\n", excess_flow[sink]);
    printf("\n\nTotal time (sec): %f\n", stop_time - start_time);

    return EXIT_SUCCESS;
}


