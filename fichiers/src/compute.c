
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter); //séq simple
unsigned compute_v1 (unsigned nb_iter); //OpenMP for - base
unsigned compute_v2 (unsigned nb_iter); //OpenMP for - tuilée
unsigned compute_v3 (unsigned nb_iter); //openMP for - opt
unsigned compute_v4 (unsigned nb_iter); //openCL task - tuile
unsigned compute_v5 (unsigned nb_iter); //openCL task - opt
unsigned compute_v6 (unsigned nb_iter); //openCL naive et opt

void_func_t first_touch [] = {
  //[ !!!!! TODO]attendre réponse prof
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute [] = {
  compute_v0,//seq - base
  compute_v1,//seq - tuile
  compute_v2,//seq - opti
  compute_v3,//for - base
  compute_v4,//for - tuile
  compute_v5,//for - opti
  compute_v6,//task - tuile
  compute_v7,//task - opt
  compute_v8,//opencl
};

char *version_name [] = {
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

unsigned opencl_used [] = {
  0,//seq - base
  0,//seq - tuile
  0,//seq - opti
  0,//for - base
  0,//for - tuile
  0,//for - opti
  0,//task - tuile
  0,//task - opt
  1,//opencl
};

Uint32 red =    0xFF0000FF;
Uint32 green =  0x00FF00FF;
Uint32 blue =   0x0000FFFF;
Uint32 yellow = 0xFFFF00FF;
Uint32 cyan =   0x00FFFFFF;
Uint32 magenta = 0xFF00FFFF;


void first_touch_v1 (){
  int i,j ;
#pragma omp parallel for
  for(i=0; i<DIM ; i++) {
    for(j=0; j < DIM ; j += 512)
      next_img (i, j) = cur_img (i, j) = 0 ;
  }
}

void first_touch_v2 (){

}

void calcul_pixel(int i, int j){
        /*
        Si la cellule actuelle est morte (rouge ou noire)
        */
      int nb_voisins = 0;
      nb_voisins += cur_img(i-1,j) != 0 && cur_img(i-1,j) != red;           
      nb_voisins += cur_img(i-1,j-1) != 0 && cur_img(i-1,j-1) != red;
      nb_voisins += cur_img(i-1,j+1) != 0 && cur_img(i-1,j+1) != red;
      nb_voisins += cur_img(i,j-1) != 0 && cur_img(i,j-1) != red;
      nb_voisins += cur_img(i+1,j-1) != 0 && cur_img(i+1,j-1) != red;
      nb_voisins += cur_img(i+1,j) != 0 && cur_img(i+1,j) != red;
      nb_voisins += cur_img(i,j+1) != 0 && cur_img(i,j+1) != red;       
      nb_voisins += cur_img(i+1,j+1) != 0 && cur_img(i+1,j+1) != red; 
      if(cur_img(i,j)== 0 || cur_img(i,j)== red ){                
          if(nb_voisins == 3)
            next_img(i,j) = green;
          else
            next_img (i, j) = 0;
      }
       /*
        Si la cellule actuelle est vivante
       */
      else{
              if(nb_voisins <2 || nb_voisins >3)
            next_img(i,j) = red;
          else
            next_img (i, j) = yellow; 
      }
}

////////////////////////////////////////// Version séquentielle simple
unsigned compute_v0 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it ++) {
        for(int i=1; i<DIM-1 ; i++) {
          for(int j=1; j < DIM-1 ; j ++){      
                calcul_pixel(i,j);         
          }
        }
      swap_images();
    }
  return 0;
  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
}

//Version Sequentielle - tuile
unsigned compute_v1(unsigned nb_iter){
  unsigned TILESIZE_i = 32;
  unsigned TILESIZE_j = 32;
  unsigned TILESIZE = 32;
  unsigned tranche = DIM / TILESIZE;

    for (unsigned it = 1; it <= nb_iter; it ++) {
//#pragma omp parallel for collapse(4) schedule(dynamic) 
        for(int i = 1; i <= DIM  - 1; i += TILESIZE_i) {
          for(int j = 1; j <= DIM - 1; j += TILESIZE_j){      
            for(int l = i; l < TILESIZE + i ; l++){
              for(int k = j; k < TILESIZE + j; k++){
                if(TILESIZE_i == 30)
                  l+=2;
                if(TILESIZE_j == 30)
                  k+=2;
                if(l < DIM - 1 && k < DIM -1)
                  calcul_pixel(l,k); 

              }
            }
            if(j == DIM - TILESIZE)
              TILESIZE_j = 30;
            else
              TILESIZE_j = 32;
          }
          if(i == DIM - TILESIZE)
              TILESIZE_i = 30;
            else
              TILESIZE_i = 32;
        }
      swap_images();
    }

  return 0;
}
//Version Sequentielle - opti
unsigned compute_v2(unsigned nb_iter){
  return 0;
}

////////////////////////////////////////// Version OpenMP for (??)
//Version OpenMP for - de base
unsigned compute_v3(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it ++) {
      #pragma omp parallel for
        for(int i=1; i<DIM-1 ; i++) {
          for(int j=1; j < DIM-1 ; j ++){      
                calcul_pixel(i,j);         
          }
        }
      swap_images();
    }
  return 0;
}

// Version OpenMp for - tuilée
unsigned compute_v4(unsigned nb_iter){
  return 0;
}

// Version OpenMp for - optimisée
unsigned compute_v5(unsigned nb_iter){
  return 0;
}

////////////////////////////////////////// Version OpenMP task
// Version OpenMp task - tuilée
unsigned compute_v6(unsigned nb_iter){
  return 0; // on ne s'arrête jamais
}

// Version OpenMP task- optimisée
unsigned compute_v7(unsigned nb_iter){
  return 0; // on ne s'arrête jamais
}

////////////////////////////////////////// Version OpenCL
unsigned compute_v8 (unsigned nb_iter){
  return ocl_compute (nb_iter);
}

