static unsigned saturate (unsigned input)
{
  for (int i = 0; i < 7; i++)
    input = (input << 1) | input;

  return input;
}

__kernel void game_of_life_naif (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);

  unsigned local_color = in[y * DIM + x];

  if(x != (DIM - 1) && x != 0 && y != (DIM-1) && y != 0 ){
    int nb_voisins = 0;
    
    nb_voisins += in[y * DIM + (x+1)]!=0 && in[y * DIM + (x+1)]!=saturate (0x11000011);
    nb_voisins += in[y * DIM + (x-1)]!=0 && in[y * DIM + (x-1)]!=saturate (0x11000011);
    nb_voisins += in[(y+1) * DIM + x]!=0 && in[(y+1) * DIM + x]!=saturate (0x11000011);
    nb_voisins += in[(y-1) * DIM + x]!=0 && in[(y-1) * DIM + x]!=saturate (0x11000011);
    nb_voisins += in[(y-1) * DIM + (x-1)]!=0 && in[(y-1) * DIM + (x-1)]!=saturate (0x11000011);
    nb_voisins += in[(y+1) * DIM + (x+1)]!=0 && in[(y+1) * DIM + (x+1)]!=saturate (0x11000011);
    nb_voisins += in[(y-1) * DIM + (x+1)]!=0 && in[(y-1) * DIM + (x+1)]!=saturate (0x11000011);
    nb_voisins += in[(y+1) * DIM + (x-1)]!=0 && in[(y+1) * DIM + (x-1)]!=saturate (0x11000011);
       /*
        Si la cellule actuelle est morte
       */
    if(local_color == 0 || local_color == saturate (0x11000011)){
      if(nb_voisins == 3)
        out[y*DIM + x] = saturate (0x00110011); //green
      else
        out[y*DIM + x] = 0;
    } /*
        Si la cellule actuelle est vivante
       */
    else{
      if(nb_voisins < 2 || nb_voisins > 3)
        out[y*DIM + x] = saturate (0x11000011); //red
      else
        out[y*DIM + x] = saturate (0x11110011); //yellow
      }
  }
}



__kernel void game_of_life_opt (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);
    __local float tile[TILEY][TILEX+1];

  int xloc = get_local_id (0);
  int yloc = get_local_id (1);

  //initialiser le tampon tile
  tile [xloc][yloc] = in [y * DIM + x];

  //barriere obligatoire
  barrier (CLK_LOCAL_MEM_FENCE);

  //ecrire le resultat
  out [(x-xloc +yloc) * DIM + y-yloc + xloc] = tile [yloc][xloc];
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
