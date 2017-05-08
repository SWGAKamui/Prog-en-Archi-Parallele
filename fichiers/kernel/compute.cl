__kernel void game_of_life_naif (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);
  unsigned local_color = in[y * DIM + x];

  if(x != (DIM - 1) && x != 0 && y != (DIM-1) && y != 0 ){
    int nb_voisins = 0;
    /*  Calcul nombre de voisins   */
    nb_voisins = (in[y * DIM + (x+1)]!=0 && in[y * DIM + (x+1)]!=0xFF0000FF)
                + (in[y * DIM + (x-1)]!=0 && in[y * DIM + (x-1)]!=0xFF0000FF)
                + (in[(y+1) * DIM + x]!=0 && in[(y+1) * DIM + x]!=0xFF0000FF)
                + (in[(y-1) * DIM + x]!=0 && in[(y-1) * DIM + x]!=0xFF0000FF)
                + (in[(y-1) * DIM + (x-1)]!=0 && in[(y-1) * DIM + (x-1)]!=0xFF0000FF)
                + (in[(y+1) * DIM + (x+1)]!=0 && in[(y+1) * DIM + (x+1)]!=0xFF0000FF)
                + (in[(y-1) * DIM + (x+1)]!=0 && in[(y-1) * DIM + (x+1)]!=0xFF0000FF)
                + (in[(y+1) * DIM + (x-1)]!=0 && in[(y+1) * DIM + (x-1)]!=0xFF0000FF);

      /* Si la cellule actuelle est morte */
    if(local_color == 0 || local_color == 0xFF0000FF){
      if(nb_voisins == 3)
        out[y*DIM + x] = 0x00FF00FF; //green
      else
        out[y*DIM + x] = 0;
    } /* Si la cellule actuelle est vivante */
    else{
      if(nb_voisins < 2 || nb_voisins > 3)
        out[y*DIM + x] = 0xFF0000FF ; //red
      else
        out[y*DIM + x] = 0xFFFF00FF; //yellow
      }
  }
}

__kernel void game_of_life_opt (__global unsigned *in, __global unsigned *out)
{
 int x = get_global_id (0);
  int y = get_global_id (1);
  unsigned local_color = in[y * DIM + x];
  __local float tile[TILEY][TILEX+1];

  int xloc = get_local_id (0);
  int yloc = get_local_id (1);

  int haut, bas, gauche, droite ;

  haut   = ( xloc == 0       && x > 0);
  bas    = ( xloc == TILEX-1 && x < DIM-1);
  gauche = ( yloc == 0       && y > 0);
  droite = ( yloc == TILEY-1 && y < DIM-1);

     /*initialiser le tampon tile */ 
     //premiere partie 
     tile [yloc + 1][xloc + 1] = in [y * DIM + x];
     //deuxieme partie
     if(haut || bas)
        tile [yloc + 1][xloc + 1 - haut + bas] = in [y * DIM + x - haut + bas];
     //troisieme partie
      if(droite || gauche)
        tile [yloc + 1 - gauche + droite][xloc + 1] = in [(y - gauche + droite) * DIM + x];
     //quatrieme partie
      if((haut || bas) && (gauche || droite) )
         tile [yloc + 1 - gauche + droite][xloc + 1 - haut + bas] = in [(y - gauche + droite) * DIM + x - haut + bas];

     //barriere obligatoire pour attendre l'initialisation de tile
     barrier (CLK_LOCAL_MEM_FENCE);

     int nb_voisins = 0;
     if(!(haut || bas || droite || gauche) )
         nb_voisins = (tile[yloc][xloc + 1]!=0     && tile[yloc][xloc + 1]!=0xFF0000FF)
                    + (tile[yloc + 1][xloc + 2]!=0 && tile[yloc + 1][xloc + 2]!=0xFF0000FF)
                    + (tile[yloc + 1][xloc]!=0     && tile[yloc + 1][xloc]!=0xFF0000FF)
                    + (tile[yloc][xloc]!=0         && tile[yloc][xloc]!=0xFF0000FF)
                    + (tile[yloc + 2][xloc + 2]!=0 && tile[yloc + 2][xloc + 2]!=0xFF0000FF)
                    + (tile[yloc + 2][xloc + 1]!=0 && tile[yloc + 2][xloc + 1]!=0xFF0000FF)
                    + (tile[yloc][xloc + 2]!=0     && tile[yloc][xloc + 2]!=0xFF0000F)
                    + (tile[yloc + 2][xloc]!=0     && tile[yloc + 2][xloc]!=0xFF0000F);


     /* Si la cellule actuelle est morte */
    if(local_color == 0 || local_color == 0xFF0000FF){
      if(nb_voisins == 3)
        out[y*DIM + x] = 0x00FF00FF; //green
      else
        out[y*DIM + x] = 0;
    } /* Si la cellule actuelle est vivante */
    else{
      if(nb_voisins < 2 || nb_voisins > 3)
        out[y*DIM + x] = 0xFF0000FF ; //red
      else
        out[y*DIM + x] = 0xFFFF00FF; //yellow
      }

}

// NE PAS MODIFIER
static float4 color_scatter (unsigned c)
{
  uchar4 ci;

  ci.s0123 = (*((uchar4 *) &c)).s3210;
  return convert_float4 (ci) / (float4) 255;
}

// NE PAS MODIFIER: ce noyau est appelé lorsqu'une mise à jour de la
// texture de l'image affichée est requise
__kernel void update_texture (__global unsigned *cur, __write_only image2d_t tex)
{
  int y = get_global_id (1);
  int x = get_global_id (0);
  int2 pos = (int2)(x, y);
  unsigned c;

  c = cur [y * DIM + x];

  write_imagef (tex, pos, color_scatter (c));
}
