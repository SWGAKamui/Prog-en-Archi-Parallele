
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;
int TILESIZE = 32;

void calcul_pixel (int i, int j);

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter); //séq simple
unsigned compute_v1 (unsigned nb_iter); //séq tuile
unsigned compute_v2 (unsigned nb_iter); //séq opti
unsigned compute_v3 (unsigned nb_iter); //openMP for - base
unsigned compute_v4 (unsigned nb_iter); //openMP for - tuile
unsigned compute_v5 (unsigned nb_iter); //openMP for - opt
unsigned compute_v6 (unsigned nb_iter); //openMP task - tuile
void compute_v6_tmp (int inc_i, int inc_j, int size_i, int size_j); //openMP task - tuile
unsigned compute_v7 (unsigned nb_iter); //openMP task - opt
unsigned compute_v8 (unsigned nb_iter); //openCL naive et opt
void_func_t first_touch[] = {
  //[ !!!!! TODO]attendre réponse pro
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute[] = {
  compute_v0,     //seq - base
  compute_v1,     //seq - tuile
  compute_v2,     //seq - opti
  compute_v3,     //for - base
  compute_v4,     //for - tuile
  compute_v5,     //for - opti
  compute_v6,     //task - tuile
  compute_v7,     //task - opt
  compute_v8,     //opencl
};

char *version_name[] = {
  "Séquentielle - base",
  "Séquentielle - tuile",
  "Séquentielle - optimisee",
  "OpenMP for - base",
  "OpenMP for - tuile",
  "OpenMP for - opt",
  "OpenMP task - tuile",
  "OpenMP task - opt",
  "OpenCL",
};

unsigned opencl_used[] = {
  0,        //seq - base
  0,        //seq - tuile
  0,        //seq - opti
  0,        //for - base
  0,        //for - tuile
  0,        //for - opti
  0,        //task - tuile
  0,        //task - opt
  1,        //opencl
};

Uint32 red = 0xFF0000FF;
Uint32 green = 0x00FF00FF;
Uint32 blue = 0x0000FFFF;
Uint32 yellow = 0xFFFF00FF;
Uint32 cyan = 0x00FFFFFF;
Uint32 magenta = 0xFF00FFFF;


void
first_touch_v1 (){
#pragma omp parallel for
    for (int i = 1; i < DIM - 1; i++)
      for (int j = 1; j < DIM - 1; j++)   
        calcul_pixel (i, j);
        
    swap_images ();
  
}

void first_touch_v2 (){

}

void calcul_pixel (int i, int j) {
  int nb_voisins = 0;
  nb_voisins = (cur_img (i - 1, j) != 0 && cur_img (i - 1, j) != red)
              + (cur_img (i - 1, j - 1) != 0 && cur_img (i - 1, j - 1) != red)
              + (cur_img (i - 1, j + 1) != 0 && cur_img (i - 1, j + 1) != red)
              + (cur_img (i, j - 1) != 0 && cur_img (i, j - 1) != red)
              + (cur_img (i + 1, j - 1) != 0 && cur_img (i + 1, j - 1) != red)
              + (cur_img (i + 1, j) != 0 && cur_img (i + 1, j) != red)
              + (cur_img (i, j + 1) != 0 && cur_img (i, j + 1) != red)
              + (cur_img (i + 1, j + 1) != 0 && cur_img (i + 1, j + 1) != red);
  
  /*
    Si la cellule actuelle est morte (rouge ou noire)
  */
  if (cur_img (i, j) == 0 || cur_img (i, j) == red){
      if (nb_voisins == 3)
        next_img (i, j) = green;
      else
        next_img (i, j) = 0;
    }
  /*
    Si la cellule actuelle est vivante
  */
  else{
    if (nb_voisins < 2 || nb_voisins > 3)
      next_img (i, j) = red;
    else
      next_img (i, j) = yellow;
  }
}

bool calcul_pixel_opti (int i, int j) {
  int nb_voisins = 0;
  nb_voisins = (cur_img (i - 1, j) != 0 && cur_img (i - 1, j) != red)
              + (cur_img (i - 1, j - 1) != 0 && cur_img (i - 1, j - 1) != red)
              + (cur_img (i - 1, j + 1) != 0 && cur_img (i - 1, j + 1) != red)
              + (cur_img (i, j - 1) != 0 && cur_img (i, j - 1) != red)
              + (cur_img (i + 1, j - 1) != 0 && cur_img (i + 1, j - 1) != red)
              + (cur_img (i + 1, j) != 0 && cur_img (i + 1, j) != red)
              + (cur_img (i, j + 1) != 0 && cur_img (i, j + 1) != red)
              + (cur_img (i + 1, j + 1) != 0 && cur_img (i + 1, j + 1) != red);
  
  if (cur_img (i, j) == 0 || cur_img (i, j) == red){
    if (nb_voisins == 3){
      next_img (i, j) = green;
      return false;
    }
    else{
      next_img (i, j) = 0;
      return true;
    }
  }
  else{
    if (nb_voisins < 2 || nb_voisins > 3){
      next_img (i, j) = red;
      return false;
    }
    else{
      next_img (i, j) = yellow;
      return true;
    }
  }
}

bool tuile_cal(int i, int j){
  bool state = true;
  for (int l = i; l < i + TILESIZE; l++)
    for (int k = j; k < j + TILESIZE; k++)
      if(l < DIM - 1 && k < DIM -1){
        if(!calcul_pixel_opti (l, k))
          state = false;
      }
   return state;
}

bool verif_isStable_voisins (bool** isStable, int i,int j){
 //   printf("verif isStable pour %d,%d --", i, j );
 //   printf("%s\n", isStable[i][j] ? "true" : "false");

    if(!isStable[i][j]){
   //       printf("isstable (%d,%d) est false\n", i, j );
      return false;
    }

    // if(i==0 && j==0){
    //   //  printf("i=0,j=0\n");
    //     return (isStable[i+1][j] && isStable[i][j+1] && isStable[i+1][j+1] );
    // }else if(i== 0 && j==DIM-1){
    //   //   printf("i=0,j=DIM - 1\n");
    //     return (isStable[i][j-1] && isStable[i+1][j] && isStable[i+1][j-1] );
    // }else if( i== DIM-1 && j==0){
    //      //       printf("i=DIM - 1,j=0\n");
    //     return (isStable[i][j+1] && isStable[i-1][j] && isStable[i-1][j+1] );   
    // }else if(i== DIM-1 && j==DIM-1){
    //     //     printf("iDIM - 1,j=DIM -1\n");

    //     return (isStable[i-1][j] && isStable[i][j-1] && isStable[i-1][j-1] );
    // }else 
    if(i== 0){
     //         printf("i=0\n");

      return (isStable[i][j+1] 
              && isStable[i][j-1]
              && isStable[i+1][j] 
              && isStable[i+1][j+1]
              && isStable[i+1][j-1] );
    }else if(i== DIM-1){
     // printf("i=dim - 1\n");

      return (isStable[i-1][j+1] 
              && isStable[i][j+1]
              && isStable[i][j-1] 
              && isStable[i-1][j-1] 
              && isStable[i-1][j] );
    }else if(j==0){
    //  printf("j==0\n");
      return (isStable[i-1][j+1] 
              && isStable[i][j+1]
              && isStable[i+1][j] 
              && isStable[i+1][j+1]
              && isStable[i-1][j] );
    }
    else if(j== DIM-1){
     // printf("j=dim - 1\n");
        return (isStable[i][j-1] 
              && isStable[i-1][j-1] 
              && isStable[i+1][j] 
              && isStable[i+1][j-1]
              && isStable[i-1][j] );
    }else{
     // printf("else\n");
     return (isStable[i-1][j+1] 
              && isStable[i][j+1]
              && isStable[i][j-1] 
              && isStable[i-1][j-1] 
              && isStable[i+1][j] 
              && isStable[i+1][j-1]
              && isStable[i+1][j+1]
              && isStable[i-1][j]);
    }
}


////////////////////////////////////////// Version séquentielle simple
unsigned compute_v0 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){
    for (int i = 1; i < DIM - 1; i++)
      for (int j = 1; j < DIM - 1; j++)   
        calcul_pixel (i, j);
        
    swap_images ();
  }
  return 0;
  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
}

//Version Sequentielle - tuile
unsigned compute_v1 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){
    for (int i = 1; i < DIM - 1; i += TILESIZE)
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        for (int l = i; l < i + TILESIZE; l++)
          for (int k = j; k < j + TILESIZE; k++)
            if(l < DIM - 1 && k < DIM -1)
              calcul_pixel (l, k);  
      }
    swap_images (); 
  }
  return 0;
}
//Version Sequentielle - opti
unsigned compute_v2 (unsigned nb_iter){
  unsigned tile = DIM/TILESIZE + 2;
  bool** isStable= malloc(tile*sizeof(bool*));
  int isStable_i, isStable_j;
  for (int i = 0; i < tile; i++)
      isStable[i] = malloc(tile*sizeof(bool));

     for(int i = 0; i < tile; i++)
      for(int j = 0; j < tile; j++)
        isStable[i][j] = false;

 
  for (unsigned it = 1; it <= nb_iter; it++){
    for (int i = 1; i < DIM - 1; i += TILESIZE){
      isStable_i =  (i/TILESIZE);
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        isStable_j = (j/TILESIZE); 
        if(!verif_isStable_voisins(isStable, isStable_i, isStable_j)){
              isStable[isStable_i][isStable_j] = tuile_cal(i,j);
        }
        else{
          printf("nothing\n");
        }
      }
    }
    swap_images ();
  }
  for(int i = 0; i < tile;i++)
    free(isStable[i]);
  free(isStable);
  return 0;
}
////////////////////////////////////////// Version OpenMP for (??)
//Version OpenMP for - de base
unsigned compute_v3 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){
      #pragma omp parallel for
      for (int i = 1; i < DIM - 1; i++)
        for (int j = 1; j < DIM - 1; j++)
         calcul_pixel (i, j);
      
  
    swap_images ();
  }
  return 0;
}

// Version OpenMp for - tuilée
unsigned compute_v4 (unsigned nb_iter){
  int i,j,k,l;
  int init_l = 1, init_k = 1;
  for (unsigned it = 1; it <= nb_iter; it++){
  #pragma omp parallel for collapse(4) schedule(static, 1)
   for (i = 1; i < DIM - 1; i += TILESIZE)
      for (j = 1; j < DIM - 1; j += TILESIZE){  
        for (l = init_l; l < init_l + TILESIZE; l++)
          for (k = init_k; k < init_k + TILESIZE; k++)
            if(l < DIM - 1 && k < DIM -1){
              calcul_pixel (l, k);  
              if(init_l != i)
                init_l = i;
              if(init_k != j)
                init_k = j;
            }
      }
    swap_images (); 
  }
  return 0;
}

// Version OpenMp for - optimisée
unsigned compute_v5 (unsigned nb_iter){
  unsigned tile = DIM/TILESIZE;
  bool** isStable= malloc(tile*sizeof(bool*));
  int isStable_i, isStable_j;
  for (int i = 0; i < tile; i++)
      isStable[i] = malloc(tile*sizeof(bool));;

  for(int i =0; i < tile; i++)
    for (int j = 0; j < tile ; j++)
      isStable[i][j] = false;

 
  for (unsigned it = 1; it <= nb_iter; it++){
    #pragma omp parallel for
    for (int i = 1; i < DIM - 1; i += TILESIZE){
      isStable_i =  (i/TILESIZE);
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        isStable_j = (j/TILESIZE); 
        if(!verif_isStable_voisins(isStable, isStable_i, isStable_j)){
            if(tuile_cal(i,j) == TILESIZE*TILESIZE)
              isStable[isStable_i][isStable_j] = true;
            else
              isStable[isStable_i][isStable_j] = false;
        }
        else{
          printf("nothing\n");
        }
      }
    }
    swap_images ();
  }
  return 0;
}

////////////////////////////////////////// Version OpenMP task
// Version OpenMp task - tuilée
unsigned compute_v6 (unsigned nb_iter){
  int i, j, k, l = 1;
  for (unsigned it = 1; it <= nb_iter; it++){
  #pragma omp parallel
  #pragma omp single firstprivate(i, j, k, l)
  {
    for (i = 1; i < DIM - 1; i += TILESIZE)
      for (j = 1; j < DIM - 1; j += TILESIZE)
        #pragma omp task
        for (k = i; k < i + TILESIZE; k++)
          for (l = j; l < j + TILESIZE; l++)
            if (k < DIM - 1 && l < DIM - 1)
              calcul_pixel (k, l);
    #pragma omp taskwait
    }
  swap_images ();
  }
  return 0;
}


// Version OpenMP task- optimisée
unsigned compute_v7 (unsigned nb_iter){

  return 0;     // on ne s'arrête jamais
}

////////////////////////////////////////// Version OpenCL
unsigned compute_v8 (unsigned nb_iter){
  return ocl_compute (nb_iter);
}
