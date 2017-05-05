
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
unsigned compute_v7 (unsigned nb_iter); //openMP task - opt
unsigned compute_v8 (unsigned nb_iter); //openCL naive et opt
void_func_t first_touch[] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute[] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
  compute_v4,
  compute_v5,
  compute_v6,
  compute_v7,
  compute_v8,
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
    if(!isStable[i][j]){
      return false;
    }
    if(i== 0){
      return (isStable[i][j+1] 
              && isStable[i][j-1]
              && isStable[i+1][j] 
              && isStable[i+1][j+1]
              && isStable[i+1][j-1] );
    }else if(i== DIM-1){
      return (isStable[i-1][j+1] 
              && isStable[i][j+1]
              && isStable[i][j-1] 
              && isStable[i-1][j-1] 
              && isStable[i-1][j] );
    }else if(j==0){
      return (isStable[i-1][j+1] 
              && isStable[i][j+1]
              && isStable[i+1][j] 
              && isStable[i+1][j+1]
              && isStable[i-1][j] );
    }
    else if(j== DIM-1){
        return (isStable[i][j-1] 
              && isStable[i-1][j-1] 
              && isStable[i+1][j] 
              && isStable[i+1][j-1]
              && isStable[i-1][j] );
    }else{
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


bool ** init_tab(int tile){
  bool** isStable= malloc(tile*sizeof(bool*));
  for (int i = 0; i < tile; i++)
    isStable[i] = malloc(tile*sizeof(bool));
  for(int i = 0; i < tile; i++)
   for(int j = 0; j < tile; j++)
     isStable[i][j] = false;
  return isStable;
}

void free_tab(bool ** isStable, int tile){
  for(int i = 0; i < tile;i++)
    free(isStable[i]);
  free(isStable);
}
//Version Sequentielle - opti
unsigned compute_v2 (unsigned nb_iter){
  unsigned tile = DIM/TILESIZE + 2;
  bool** isStable = init_tab(tile);
 
  for (unsigned it = 1; it <= nb_iter; it++){
    for (int i = 1; i < DIM - 1; i += TILESIZE){
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        if(!verif_isStable_voisins(isStable, i/TILESIZE, j/TILESIZE))
          isStable[i/TILESIZE][j/TILESIZE] = tuile_cal(i,j);
      }
    }
    swap_images ();
  }
  free_tab(isStable, tile);
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

void lancer_calcul(int i, int j){
omp_set_nested(1);
  #pragma omp parallel for collapse(2) 
  for (int k = i; k < i + TILESIZE; k++)
    for (int l = j; l < j + TILESIZE; l++)
      if(l < DIM - 1 && k < DIM -1){
        calcul_pixel (l, k);  
      }
}


// Version OpenMp for - tuilée
unsigned compute_v4 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++){ 
  #pragma omp parallel for collapse(2) schedule(dynamic)
   for (int i = 1; i < DIM - 1; i += TILESIZE)
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        lancer_calcul(i,j);

      }
    swap_images (); 
  }
  return 0;
}

bool tuile_cal_v5(int i, int j){
  bool state = true;
  #pragma omp parallel for collapse(2)
  for (int l = i; l < i + TILESIZE; l++)
    for (int k = j; k < j + TILESIZE; k++)
      if(l < DIM - 1 && k < DIM -1){
        if(!calcul_pixel_opti (l, k))
          state = false;
      }
   return state;
}

// Version OpenMp for - optimisée
unsigned compute_v5 (unsigned nb_iter){
 unsigned tile = DIM/TILESIZE + 2;
  bool** isStable = init_tab(tile);
 
  for (unsigned it = 1; it <= nb_iter; it++){
        #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int i = 1; i < DIM - 1; i += TILESIZE){
      for (int j = 1; j < DIM - 1; j += TILESIZE){  
        if(!verif_isStable_voisins(isStable, i/TILESIZE, j/TILESIZE))
          isStable[i/TILESIZE][j/TILESIZE] = tuile_cal_v5(i,j);
      }
    }
    swap_images ();
  }
  free_tab(isStable, tile);
  return 0;
}

////////////////////////////////////////// Version OpenMP task
// Version OpenMp task - tuilée
unsigned compute_v6 (unsigned nb_iter){
  int i, j, k, l = 1;
  for (unsigned it = 1; it <= nb_iter; it++){
  #pragma omp parallel
  #pragma omp single
  {
    for (i = 1; i < DIM - 1; i += TILESIZE)
      for (j = 1; j < DIM - 1; j += TILESIZE)
        #pragma omp task  firstprivate(i, j, k, l)
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
bool tuile_cal_v7(int i, int j){
  bool state = true;
  
  for (int l = i; l < i + TILESIZE; l++)
    for (int k = j; k < j + TILESIZE; k++)
      if(l < DIM - 1 && k < DIM -1){
        if(!calcul_pixel_opti (l, k))
          state = false;
      }
  
   return state;
}

// Version OpenMP task- optimisée
unsigned compute_v7 (unsigned nb_iter){
 unsigned tile = DIM/TILESIZE + 2;
  bool** isStable = init_tab(tile);
  for (unsigned it = 1; it <= nb_iter; it++){
    #pragma omp parallel
    #pragma omp single
    {
      for (int i = 1; i < DIM - 1; i += TILESIZE){
        for (int j = 1; j < DIM - 1; j += TILESIZE){  
          #pragma omp task  firstprivate(i, j)
          if(!verif_isStable_voisins(isStable, i/TILESIZE, j/TILESIZE))
            isStable[i/TILESIZE][j/TILESIZE] = tuile_cal_v7(i,j);
        }
      }
          #pragma omp taskwait
      swap_images ();
    }
  }
  free_tab(isStable, tile);
  return 0;
}

////////////////////////////////////////// Version OpenCL
unsigned compute_v8 (unsigned nb_iter){
  return ocl_compute (nb_iter);
}
