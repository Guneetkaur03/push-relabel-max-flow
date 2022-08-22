#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>


void generate_graph()
{
    int N = 10;

    int G[N][N];

    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            G[i][j] = 0;
        }
    }

    int source = 0;
    int sink = N - 1;

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
        G[source][generate[0]] = (rand() % (N + length_path))+ 1;


        for (int k = 0; k < length_path - 1; k++)
        {
            G[generate[k]][generate[k + 1]] = (rand() % (N + length_path)) + 1;
        }
        G[generate[length_path - 1]][sink] = (rand() % (N + length_path)) + 1;
    }

    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            printf("%d ", G[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    generate_graph();

    return EXIT_SUCCESS;
}