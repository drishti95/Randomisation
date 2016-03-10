
#include <igraph.h>
#include <stdio.h>
#include <igraph_interrupt_internal.h>
#include <igraph_memory.h>
#include <igraph_progress.h>
#include "igraph_types_internal.h"
#include <math.h>

 

int igraph_i_minimum_spanning_tree_prim(const igraph_t* graph,
    igraph_vector_t* res, const igraph_vector_t *weights) {

  long int no_of_nodes=igraph_vcount(graph);
  long int no_of_edges=igraph_ecount(graph);
  char *already_added;
  char *added_edges;

  igraph_d_indheap_t heap=IGRAPH_D_INDHEAP_NULL;
  igraph_integer_t mode=IGRAPH_ALL;
  
  igraph_vector_t adj;

  long int i, j,ar[100];

  igraph_vector_clear(res);

  if (weights == 0)
    return igraph_i_minimum_spanning_tree_unweighted(graph, res);

  if (igraph_vector_size(weights) != igraph_ecount(graph)) {
    IGRAPH_ERROR("Invalid weights length", IGRAPH_EINVAL);
  }

  added_edges=igraph_Calloc(no_of_edges, char);
  if (added_edges==0) {
    IGRAPH_ERROR("prim spanning tree failed", IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_free, added_edges);
  already_added=igraph_Calloc(no_of_nodes, char);
  if (already_added == 0) {
    IGRAPH_ERROR("prim spanning tree failed", IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_free, already_added);
  IGRAPH_CHECK(igraph_d_indheap_init(&heap, 0));
  IGRAPH_FINALLY(igraph_d_indheap_destroy, &heap);
  IGRAPH_VECTOR_INIT_FINALLY(&adj, 0);
  
   int size=igraph_vector_size(weights);
   for(i=0;i<100;i++)
   ar[i]=0;
   for(i=0;i<size;i++)
   {
   if(VECTOR(*weights)[i]>100) //The value 100 here is just an example.Can be changed according to the requirements of the program.
     {
           
      printf("not possible.. too large weigths!! . Aborting !\n");
      exit(1);
      }
   ar[(int)VECTOR(*weights)[i]]++;
   }
   int f=0;
   for(i=0;i<100;i++)
   {
     
    if(ar[i]>f)
    {
     f=ar[i];
    }
  }
     
  long int ra[100];
  int a,p;
srand(time(NULL));
for(a=0;a<100;a++)
    ra[a]=rand();
    p=0;
double b =(log(f)/log(2)); int x=ceil(b);
   long int v=pow(2,32-x);
   for(i=0;i<size;i++)
  {
    

    
    int a=ra[p++]%(ar[(int)VECTOR(*weights)[i]]);
     if(VECTOR(*weights)[i]>v) 
     {
           
      printf("not possible.. too large weigths!! . Aborting !\n");
      exit(1);
      }
    (VECTOR(*weights)[i])*=pow(2,x);
    VECTOR(*weights)[i]+=a;
    if(p==100)
    p=0;
 
  }
     
 //igraph_vector_print(weights);
  srand(time(NULL));
  int n=rand()%no_of_nodes;

  for (i=n; i<no_of_nodes; i++) {
    if (already_added[i]>0) { continue; }
    IGRAPH_ALLOW_INTERRUPTION();

    already_added[i]=1;
    /* add all edges of the first vertex */
    igraph_incident(graph, &adj, (igraph_integer_t) i, (igraph_neimode_t) mode);
    for (j=0; j<igraph_vector_size(&adj); j++) {
      long int edgeno=(long int) VECTOR(adj)[j];
      igraph_integer_t edgefrom, edgeto;
      long int neighbor;
      igraph_edge(graph, (igraph_integer_t) edgeno, &edgefrom, &edgeto);
      neighbor= edgefrom != i ? edgefrom : edgeto;
      if (already_added[neighbor] == 0) {
	IGRAPH_CHECK(igraph_d_indheap_push(&heap, -VECTOR(*weights)[edgeno], i,
					   edgeno));
      }
    }

    while(! igraph_d_indheap_empty(&heap)) {
      /* Get minimal edge */
      long int from, edge;
      igraph_integer_t tmp, to;
      igraph_d_indheap_max_index(&heap, &from, &edge);
      igraph_edge(graph, (igraph_integer_t) edge, &tmp, &to);
      
      /* Erase it */
      igraph_d_indheap_delete_max(&heap);

      /* Is this edge already included? */
      if (added_edges[edge]==0) {
        if (from==to) { to=tmp; }
        /* Does it point to a visited node? */      
        if (already_added[(long int)to]==0) {
          already_added[(long int)to]=1;
          added_edges[edge]=1;
          IGRAPH_CHECK(igraph_vector_push_back(res, edge));
          /* add all outgoing edges */
          igraph_incident(graph, &adj, to, (igraph_neimode_t) mode);
          for (j=0; j<igraph_vector_size(&adj); j++) {
            long int edgeno=(long int) VECTOR(adj)[j];
            igraph_integer_t edgefrom, edgeto;
            long int neighbor;
            igraph_edge(graph, (igraph_integer_t) edgeno, &edgefrom, &edgeto);
            neighbor= edgefrom != to ? edgefrom : edgeto;
            if (already_added[neighbor] == 0) {
              IGRAPH_CHECK(igraph_d_indheap_push(&heap, -VECTOR(*weights)[edgeno], to,
                   edgeno));
            }
          }
        } /* for */
      } /* if !already_added */
    } /* while in the same component */
  } /* for all nodes */

  igraph_d_indheap_destroy(&heap);
  igraph_Free(already_added);
  igraph_vector_destroy(&adj);
  igraph_Free(added_edges);
  IGRAPH_FINALLY_CLEAN(4);

  return IGRAPH_SUCCESS;
}


int main() {

  igraph_t g;
  igraph_vector_t edges;
  long int i;
  igraph_small(&g, 10, IGRAPH_DIRECTED,
	       0,1, 0,2, 0,3,    1,2, 1,4, 1,5,
	       2,3, 2,6,         3,2, 6,3,
	       4,5, 4,7,         5,6, 5,8, 5,9,
	       7,5, 7,8,         8,9,
	       5,2,
	       2,1,
	       -1);
  igraph_real_t weights2[] = { 0,2,1, 0,5,2, 1,1,0, 0,2,8, 1,1,3, 1,1,4, 2,1 };
  igraph_vector_t weights_vec;
  igraph_vector_view(&weights_vec, weights2, sizeof(weights2)/sizeof(igraph_real_t));
  /*FILE *fp;
  fp=fopen("abc.gml","w");
  igraph_write_graph_gml(&g,fp,NULL,0);*/
  igraph_vector_init(&edges, 0);
  igraph_i_minimum_spanning_tree_prim(&g, &edges, &weights_vec);
  igraph_vector_print(&edges);
  igraph_vector_destroy(&edges);
  igraph_destroy(&g);

  return 0;
}


