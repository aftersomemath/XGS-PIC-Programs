//////////////////////////////////////////
//Levi Burner
//start date 12.5.10
//This is my raycasting engine API for the XGS PIC 16 bit
//////////////////////////////////////////

//Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libq.h> /* include fixed point library */

#include "XGS_PIC_GFX_DRV_V010.h"

#include "raycaster_engine.h"

void find_rays(){

int x = 0;

for(x = x; x < number_of_columns; x++)
    {
      //calculate ray position and direction 
      //_Q16  cameraX = ((2 * (long)x) << 16) / number_of_columns - ((long)1 << 16); //x-coordinate in camera space
      _Q16 rayPosX = posX;
      _Q16 rayPosY = posY;
      int ray_angle = camera_angle + ((2 * x) - 40);     
     
      if(ray_angle > 359)
      ray_angle -= 360;
      
      if(ray_angle < -359)
      ray_angle += 360;
     
      //_Q16 rayDirX = dirX + (((planeX >> 8) * cameraX) >> 8);
      //_Q16 rayDirY = dirY + (((planeY >> 8) * cameraX) >> 8);

	  _Q16 rayDirX = (my_cosine(ray_angle) << 15) / (my_cosine(ray_angle - camera_angle) >> 1);
	  _Q16 rayDirY = (my_sine(ray_angle) << 15) / (my_cosine(ray_angle - camera_angle) >> 1);

	  //which box of the map we're in  
      int mapX = (int)(rayPosX >> 16);
      int mapY = (int)(rayPosY >> 16);
       
      //length of ray from current position to next x or y-side
      _Q16 sideDistX;
      _Q16 sideDistY;
       //length of ray from one x or y-side to next x or y-side
      //float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
      //float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
      //float deg = ray_angle * .017;
      //float deltaDistX = fabs(1 / cos(deg));
      //float deltaDistY = fabs(1 / sin(deg));
      //_Q16 deltaDistX = _Q16ftoi(fabs(1 / _itofQ16(my_cosine(ray_angle))));
      _Q16 deltaDistX = labs(((long)1 << 31) / (my_cosine(ray_angle) >> 1));
      //_Q16 deltaDistY = _Q16ftoi(fabs(1 / _itofQ16(my_sine(ray_angle))));
      _Q16 deltaDistY = labs(((long)1 << 31) / (my_sine(ray_angle) >> 1));
      
      _Q16 perpWallDist;
       
      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit? 

       //calculate step and initial sideDist
      if (rayDirX < 0)
      {
        stepX = -1;
        sideDistX = ((rayPosX - ((long)mapX << 16)) >> 8) * (deltaDistX >> 8);
      }
      else
      {
        stepX = 1;
        sideDistX = ((((long)mapX << 16) + 65536 - rayPosX) >> 8) * (deltaDistX >> 8);
      }
      if (rayDirY < 0)
      {
        stepY = -1;
        sideDistY = ((rayPosY - ((long)mapY << 16)) >> 8) * (deltaDistY >> 8);
      }
      else
      {
        stepY = 1;
        sideDistY = ((((long)mapY << 16)+ 65536 - rayPosY) >> 8) * (deltaDistY >> 8);
      } 

		 //perform DDA
      while (hit == 0)
      {
        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if (worldMap[mapX][mapY] > 0) hit = 1;
      }  


      //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)

	  if (side == 0)
	  perpWallDist = ((((long)mapX << 16) - rayPosX + ((long)(1 - stepX) << 16) / 2) << 8) / (rayDirX >> 8);
      //perpWallDist = _Q16ftoi(fabs((mapX - _itofQ16(rayPosX) + (1 - stepX) / 2) / _itofQ16(rayDirX)));
      //perpWallDist =  _Q16ftoi(_itofQ16(fabs(rayPosX - posX)) * _itofQ16(deltaDistX));
      else
      perpWallDist = ((((long)mapY << 16) - rayPosY + ((long)(1 - stepY) << 16) / 2) << 8) / (rayDirY >> 8);
      //perpWallDist = _Q16ftoi(fabs((mapY - _itofQ16(rayPosY) + (1 - stepY) / 2) / _itofQ16(rayDirY)));
      //perpWallDist =  _Q16ftoi(_itofQ16(fabs(rayPosY - posY)) * _itofQ16(deltaDistY));
      
      //perpWallDist = ((perpWallDist >> 1) * (my_cosine(ray_angle - camera_angle))) >> 15;   

	  int h = 192;
      //Calculate height of line to draw on screen
      int lineHeight = abs((int)(h / _itofQ16(perpWallDist)));
       
      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + h / 2;
      if(drawStart < 0)drawStart = 0;
      int drawEnd = lineHeight / 2 + h / 2;
      if(drawEnd >= h)drawEnd = h - 1; 
      if(ray_index_selector == 0){
      ray_index[x] = drawStart;
      ray_colors[x]   = worldMap[mapX][mapY];
						}
      else{
	      	ray_index[x + number_of_columns] = drawStart;
      ray_colors[x + number_of_columns]   = worldMap[mapX][mapY];
				}
}
}	

void render_view(){
int i;
int old_line, new_line;
int color;

for(i = 0; i < number_of_columns; i++){

if(ray_index_selector == 0) {
	old_line = ray_index[i + number_of_columns];
	new_line = ray_index[i];
	color = ray_colors[i];
}
else {
	old_line = ray_index[i];
	new_line = ray_index[i + number_of_columns];
	color = ray_colors[i + number_of_columns];
}

if(old_line < new_line){
	draw_Vline(i, old_line, new_line, 0);
	draw_Vline(i, SCREEN_HEIGHT - new_line, SCREEN_HEIGHT - old_line, 0);	
	
	}

   		 draw_Vline(i, new_line,SCREEN_HEIGHT - new_line, color);
}


}

void rotate_player(int deg){
	camera_angle += deg;
      
     if(camera_angle > 359)
     camera_angle -= 360;
      
     if(camera_angle < -359)
     camera_angle += 360;
       
    /*  deg = camera_angle * .017;
						
      dirX = cos(deg);
      dirY = sin(deg);
      planeX = -.88 * sin(deg);
      planeY = .88 * cos(deg);
*/
      dirX = my_cosine(camera_angle);
      dirY = my_sine(camera_angle);
      planeX =  - .88 * my_sine(camera_angle);
      planeY = .88 * my_cosine(camera_angle);

}

void build_trig_tables(){
	int i;
	for(i = 0; i < 90;	i++){
		sint[i] = _Q16ftoi(sin((float)i * .017));
	}
			
	for(i = 0; i < 90; i++){
		cost[i] = _Q16ftoi(cos((float)i * .017));
	}
}

_Q16 my_sine(int deg){
_Q16 sine;	

if(deg < 0){
	deg +=360;
}

  if((deg > 89) & (deg < 180)){
	  deg = 180 - deg;
	  sine = sint[deg];
		}
	else if((deg > 179) & (deg < 270)){
		deg = deg - 180;
		sine = -sint[deg];
	} 
	else if ((deg > 269) & (deg < 360)){
		deg = 360 - deg;
		sine = -sint[deg];
	}
	else{
		sine = sint[deg];
	}
  return sine;
  }
  
_Q16 my_cosine(int deg){
_Q16 cosine;	

if(deg < 0){
	deg +=360;
}
	
  if((deg > 89) & (deg < 180)){
	  deg = 180 - deg;
	  cosine = -cost[deg];
		}
	else if((deg > 179) & (deg < 270)){
		deg = deg - 180;
		cosine = -cost[deg];
	} 
	else if ((deg > 269) & (deg < 360)){
		deg = 360 - deg;
		cosine = cost[deg];
	}
	else{
		cosine = cost[deg];
	}
	return cosine;
}

void draw_Vline(int x, int y_top, int y_bottom, int color){
char color_lookup[4] = {0x00, 0x55, 0xAA, 0xFF};
color = color_lookup[color];	
	while(y_top < y_bottom){
	g_VRAMBuffer[y_top * 40 + x] = color;
	y_top++;
	}
	//GFX_VLine_2BPP(x, y_top, y_bottom, (char)color, g_VRAMBuffer);
}	



