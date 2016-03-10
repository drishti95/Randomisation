#include<igraph.h>
#include <igraph_types_internal.h>
#include "igraph_interrupt_internal.h"
#include <stdlib.h>
int igraph_i_closeness_estimate_weighted(const igraph_t *graph, 
				       igraph_vector_t *res, 
				       const igraph_vs_t vids, 
				       igraph_neimode_t mode,
				       igraph_real_t cutoff,
				       const igraph_vector_t *weights,
				       igraph_bool_t normalized) {

  /* See igraph_shortest_paths_dijkstra() for the implementation 
     details and the dirty tricks. */

  long int no_of_nodes=igraph_vcount(graph);
  long int no_of_edges=igraph_ecount(graph);
  
  igraph_2wheap_t Q;
  igraph_vit_t vit;
  long int nodes_to_calc;
  
  igraph_lazy_inclist_t inclist;
  long int i, j;
  
  igraph_vector_t dist;
  igraph_vector_long_t which;
  long int nodes_reached;
  
  if (igraph_vector_size(weights) != no_of_edges) {
    IGRAPH_ERROR("Invalid weight vector length", IGRAPH_EINVAL);
  }
  
  if (igraph_vector_min(weights) < 0) {
    IGRAPH_ERROR("Weight vector must be non-negative", IGRAPH_EINVAL);
  }
  
  IGRAPH_CHECK(igraph_vit_create(graph, vids, &vit));
  IGRAPH_FINALLY(igraph_vit_destroy, &vit);
  
  nodes_to_calc=IGRAPH_VIT_SIZE(vit);
  
  IGRAPH_CHECK(igraph_2wheap_init(&Q, no_of_nodes));
  IGRAPH_FINALLY(igraph_2wheap_destroy, &Q);
  IGRAPH_CHECK(igraph_lazy_inclist_init(graph, &inclist, mode));
  IGRAPH_FINALLY(igraph_lazy_inclist_destroy, &inclist);

  IGRAPH_VECTOR_INIT_FINALLY(&dist, no_of_nodes);
  IGRAPH_CHECK(igraph_vector_long_init(&which, no_of_nodes));
  IGRAPH_FINALLY(igraph_vector_long_destroy, &which);

  IGRAPH_CHECK(igraph_vector_resize(res, nodes_to_calc));
  igraph_vector_null(res);

  for (i=0; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit), i++) {
    
    long int source=IGRAPH_VIT_GET(vit);
    igraph_2wheap_clear(&Q);
    igraph_2wheap_push_with_index(&Q, source, 0);
    VECTOR(which)[source]=i+1;
    VECTOR(dist)[source]=0.0;
    nodes_reached=0;
    
    int ar[10],a;
   srand(time(NULL));
    for(a=0;a<10;a++)
    ar[a]=rand()%100+1;
    int x=0;
    while (!igraph_2wheap_empty(&Q)) {
      igraph_integer_t minnei=(igraph_integer_t) igraph_2wheap_max_index(&Q);
      igraph_real_t mindist=-igraph_2wheap_delete_max(&Q);
      
      /* Now check all neighbors of minnei for a shorter path */
      igraph_vector_t *neis=igraph_lazy_inclist_get(&inclist, minnei);
      long int nlen=igraph_vector_size(neis);

      VECTOR(*res)[i] += mindist;
      nodes_reached++;
      
      if (cutoff>0 && mindist>=cutoff) continue;    /* NOT break!!! */
      igraph_vector_shuffle(neis);
      for (j=0; j<nlen; j++) {
	long int edge=(long int) VECTOR(*neis)[j];
	long int to=IGRAPH_OTHER(graph, edge, minnei);
	igraph_real_t altdist=mindist+VECTOR(*weights)[edge];
	igraph_real_t curdist=VECTOR(dist)[to];
	if (VECTOR(which)[to] != i+1) {
	  /* First non-infinite distance */
	  VECTOR(which)[to]=i+1;
	  VECTOR(dist)[to]=altdist;
	  IGRAPH_CHECK(igraph_2wheap_push_with_index(&Q, to, -altdist));
	} else if (altdist < curdist) {
	  /* This is a shorter path */
	  VECTOR(dist)[to]=altdist;
	  IGRAPH_CHECK(igraph_2wheap_modify(&Q, to, -altdist));
	}
       else if(altdist==curdist)
      {
        
        int n=ar[x++];
        if (n>50) 
       {
        VECTOR(which)[to]=i+1;
       }
        if(x==10)
         x=0;
      }
      }

    } /* !igraph_2wheap_empty(&Q) */

    /* using igraph_real_t here instead of igraph_integer_t to avoid overflow */
    VECTOR(*res)[i] += ((igraph_real_t)no_of_nodes * (no_of_nodes-nodes_reached));
    VECTOR(*res)[i] = (no_of_nodes-1) / VECTOR(*res)[i];

  } /* !IGRAPH_VIT_END(vit) */

  if (!normalized) {
    for (i=0; i<nodes_to_calc; i++) {
      VECTOR(*res)[i] /= (no_of_nodes-1);
    }
  }

  igraph_vector_long_destroy(&which);
  igraph_vector_destroy(&dist);
  igraph_lazy_inclist_destroy(&inclist);
  igraph_2wheap_destroy(&Q);
  igraph_vit_destroy(&vit);
  IGRAPH_FINALLY_CLEAN(5);

  return 0;
}

int main() {

  igraph_t g;
  long int i;
  igraph_vector_t res;
  igraph_vs_t vs; 
  igraph_real_t weights[] = { 1, 2, 3, 4, 5, 1, 1, 1, 1, 1 }; 
  igraph_real_t weights2[] = { 0,2,1, 0,5,2, 1,1,0, 0,2,8, 1,1,3, 1,1,4, 2,1 };
   igraph_vector_t weights_vec;
  igraph_ring(&g, 10, IGRAPH_UNDIRECTED, 0, 1);  
  igraph_vector_init(&res,0);
 
  igraph_vs_vector_small(&vs, 1, 3,5,4, 6, -1);
  printf("The follwing examples show the shortest path for the vertices 1,3,5,4,6 respectively \n\n"); 
  printf(" A ring,  with weights \n");
   igraph_vector_view(&weights_vec, weights, sizeof(weights)/sizeof(igraph_real_t));
  igraph_i_closeness_estimate_weighted(&g,&res,vs,IGRAPH_OUT,0,&weights_vec,1);	      
  igraph_vector_print(&res);
  igraph_destroy(&g);
  printf("A complicated graph \n");
  igraph_small(&g, 10, IGRAPH_DIRECTED, 
	       0,1, 0,2, 0,3,    1,2, 1,4, 1,5,
	       2,3, 2,6,         3,2, 6,3,
	       4,5, 4,7,         5,6, 5,8, 5,9,
	       7,5, 7,8,         8,9,
	       5,2,
	       2,1,
	       -1);
   
  igraph_vector_view(&weights_vec, weights2, sizeof(weights2)/sizeof(igraph_real_t));  
  igraph_i_closeness_estimate_weighted(&g,&res,vs,IGRAPH_OUT,0,&weights_vec,1);
  igraph_vector_print(&res);
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);
  igraph_destroy(&g);

  return 0;
}


