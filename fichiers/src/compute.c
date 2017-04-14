
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter); //séq simple
unsigned compute_v1 (unsigned nb_iter); //OpenMP
unsigned compute_v2 (unsigned nb_iter); //OpenMP
unsigned compute_v3 (unsigned nb_iter); //openCL

void_func_t first_touch [] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
};

char *version_name [] = {
  "Séquentielle",
  "OpenMP",
  "OpenMP zone",
  "OpenCL",
};

unsigned opencl_used [] = {
  0,
  0,
  0,
  1,
};

///////////////////////////// Version séquentielle simple


unsigned compute_v0 (unsigned nb_iter)
{



int nb_voisins = 0;
  for (unsigned it = 1; it <= nb_iter; it ++) {

        for(int i=0; i<DIM ; i++) {
          for(int j=0; j < DIM ; j ++){
            if(i!=0){
              if(j!=0)
                nb_voisins += cur_img(i-1,j-1) != 0;
              nb_voisins += cur_img(i-1,j) != 0;           
              nb_voisins += cur_img(i-1,j+1) != 0;
                
              }
              if(j!=0){
                nb_voisins += cur_img(i+1,j-1) != 0;
                nb_voisins += cur_img(i,j-1) != 0;
                }
              nb_voisins += cur_img(i,j+1) != 0;       
              nb_voisins += cur_img(i+1,j) != 0;
              nb_voisins += cur_img(i+1,j+1) != 0;
                
              
                
            /*
              Si la cellule actuelle est vivante
            */
            if(cur_img(i,j)!= 0 ){
                if(nb_voisins <2 || nb_voisins >3)
                  next_img(i,j) = 0;
                else
                  next_img (i, j) = cur_img (i, j);
            }
            /*
              Si la cellule actuelle est morte
            */
            else{
              if(nb_voisins == 3)
                  next_img(i,j) = 1;
                else
                  next_img (i, j) = cur_img (i, j);

            }
            /*
              Réinitialiser le nombre de voisins a 0
            */
            swap_images ();
            nb_voisins =0;
          }
        }
    }

  return 0;
    

  // retourne le nombre d'étapes nécessaires à la
  // stabilisation du calcul ou bien 0 si le calcul n'est pas
  // stabilisé au bout des nb_iter itérations
}


///////////////////////////// Version OpenMP de base

void first_touch_v1 ()
{
  int i,j ;

#pragma omp parallel for
  for(i=0; i<DIM ; i++) {
    for(j=0; j < DIM ; j += 512)
      next_img (i, j) = cur_img (i, j) = 0 ;
  }
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v1(unsigned nb_iter)
{
  return 0;
}



///////////////////////////// Version OpenMP optimisée

void first_touch_v2 ()
{

}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter)
{
  return 0; // on ne s'arrête jamais
}


///////////////////////////// Version OpenCL

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute (nb_iter);
}

