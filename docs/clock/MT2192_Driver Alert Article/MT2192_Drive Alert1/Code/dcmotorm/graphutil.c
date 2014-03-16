//********************************************************************************
//*                                                                              *      
//*           Name:  graphutil.c                                                 *                          
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     this unit contains usefull graphics transformation utilities and other   *
//* miscellaneous graphics procedures to display a cursor, grid, etc.            *
//*                                                                              *
//********************************************************************************

/* include header files here */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "graphutil.h"

/* declare general purpose constants here */

int TRUE;                             /* True value */
int FALSE;                            /* False value */
float eps = 1.0e-7;                   /* small number */

/* declare global graphics window mapping variables here */

float xw_min;
float yw_min;
float xw_max;
float yw_max;

/* declare global graphics viewport mapping variables here */

int xv_min;
int yv_min;
int xv_max;
int yv_max;

/*  declare current keyboard mouse coordinates */

int x_key_mouse, y_key_mouse;
int vmin_mouse, vmax_mouse, hmin_mouse, hmax_mouse;

/* declare important graphics variables here */

/*  contains the viewport information for current graphics mode  */ 
//struct viewporttype current_viewport;           

int graph_driver;                               /*  graph driver number  */
int graph_mode;                                 /*  current graph mode  */
int error_code;                                 /*  error return code  */
int xasp, yasp;

float xw1, yw1, xw2, yw2;                       /*  world coordinates  */

/* declare usefull graphics transformation functions here */

int set_window (float x1, float y1, float x2, float y2);
int set_view (int x1, int y1, int x2, int y2);
int map_window_to_viewport (float xw, float yw, int *xv, int *yv);
int map_viewport_to_window (int xv, int yv, float *xw, float *yw);

/* declare usefull graphics drawing functions here */

int cursor(int x, int y, int old_x, int old_y);
int draw_grid(int x, int y, int x_space, int y_space);

 
/* 
********************************************************************************
*                                                                              *
* SET_WINDOW - procedure to set the global world coordinate window for scaling *
*              the 2D graphics display.                                        *
*                                                                              *
********************************************************************************
*/ 
int         set_window(float         x1,
                       float         y1,
                       float         x2,
                       float         y2)
{ 
  int status;                       /* declare status return variable */
  status = TRUE;
  xw_min = x1; 
  yw_min = y1; 
  xw_max = x2; 
  yw_max = y2; 
  return (status);
}  



//********************************************************************************
//*                                                                              *
//* SET_VIEW -   procedure to set the global device viewport coordinates.        *
//*                                                                              *
//********************************************************************************
int         set_view(int      x1,
                     int      y1,
                     int      x2,
                     int      y2)
{ 
  int status;                       /* declare status return variable */
  status = TRUE;
  xv_min = x1; 
  yv_min = y1; 
  xv_max = x2; 
  yv_max = y2; 
  return (status);
}  
 
//********************************************************************************
//*                                                                              *
//* MAP_WINDOW_TO_VIEWPORT - procedure to map window coordinates to viewport     *
//*                          coordinates to scale the graphics display.          *
//*                                                                              *
//********************************************************************************
int         map_window_to_viewport(float         xw,
                                   float         yw,
                                   int *         xv,
                                   int *         yv)
{  
  int status;                       /* declare status return variable */

  status = TRUE;

  if (abs(xw_max - xw_min) > eps) 
      //*xv = (int) (ceil((xv_max - xv_min) / (xw_max - xw_min) *
      *xv = (int) ((xv_max - xv_min) / (xw_max - xw_min) *
        (xw - xw_min) + xv_min);
    else 
      *xv = 0; 

  if (abs(yw_max - yw_min) > eps) 
      { 
        //*yv = (int) (ceil((yv_max - yv_min) / (yw_max - yw_min) *
        *yv = (int) ((yv_max - yv_min) / (yw_max - yw_min) *
          (yw - yw_min) + yv_min);

        /*  flip y coordinate axis here  */ 

        //*yv = yv_max - *yv; 
      } 
    else 
     *yv = 0; 

  return (status);
}  

//********************************************************************************
//*                                                                              *
//* MAP_VIEWPORT_TO_WINDOW - procedure to map viewport coordinates to window     *
//*                          coordinates to position objects on graphics display.*
//*                                                                              *
//********************************************************************************
int         map_viewport_to_window(int      xv,
                                   int      yv,
                               float *      xw,
                               float *      yw)
{ 
  float         scale_x, scale_y;   /*  scale factors  */ 
  int status;                       /* declare status return variable */
  status = TRUE;

  /*  calculate scale factors here  */ 

  if (abs(xv_max - xv_min) > eps) 
      scale_x = (float) ((xw_max - xw_min) / (float)(xv_max - xv_min));
    else 
      scale_x = 1.0; 

  if (abs(yv_max - yv_min) > eps) 
      scale_y = (float) ((yw_max - yw_min) / (float)(yv_max - yv_min));
    else 
      scale_y = 1.0; 

  *xw = (float) (scale_x * (float)(xv - xv_min) + xw_min); 
  *yw = (float) (scale_y * (float)(yv - yv_min) + yw_min); 
  return (status);
}  

#ifdef NEW_CODE
//********************************************************************************
//*                                                                              *
//* CURSOR - procedure to display a cursor at position (x,y), while erasing old  *
//*          cursor at position (old_x, old_y).  XOR operations will be used so  *
//*          that the background is not disturbed.                               *
//*                                                                              *
//********************************************************************************
int         cursor(int      x,
                   int      y,
                   int      old_x,
                   int      old_y)
{ 
  const int cursor_color = WHITE;     /* define cursor color here */
  const int x_cursor_size = 5;        /* define cursor size here */
  const int y_cursor_size = 5;        /* define cursor size here */
  int status;                         /* declare status return variable */

  status = TRUE;

  /*  set cursor color here  */ 

  setcolor(cursor_color); 

  /*  erase cursor at old position  */ 

  /*  draw horizontal line for old cursor here  */ 

  line(old_x - x_cursor_size,old_y,old_x + x_cursor_size,old_y); 

  /*  draw vertical line for old cursor here  */ 

  line(old_x,old_y - y_cursor_size,old_x,old_y + y_cursor_size); 

  /*  draw cursor at new position  */ 

  /*  draw horizontal line for new cursor here  */ 

  line(x - x_cursor_size,y,x + x_cursor_size,y); 

  /*  draw vertical line for old cursor here  */ 

  line(x,y - y_cursor_size,x,y + y_cursor_size); 
  return (status);

} 

//********************************************************************************
//*                                                                              *
//* GRID - procedure to display a grid with its major axis centered at the grid  *
//*        origin(x,y), separated by selected horizontal and vertical spacing    *
//*        values.                                                               *
//*                                                                              *
//********************************************************************************
int         draw_grid(int      x,
                      int      y,
                      int      x_space,
                      int      y_space)
{ 
  int      i, j;   /*  loop index variables  */ 
  int      x_lower_limit; 
  int      y_lower_limit; 
  int      x_upper_limit; 
  int      y_upper_limit; 
  int status;                       /* declare status return variable */
  status = TRUE;


  /*  draw_grid  */ 

  x_lower_limit = 0; 
  y_lower_limit = 0; 
  x_upper_limit = (int) getmaxx; 
  y_upper_limit = (int) getmaxy; 

  /*  draw vertical cross lines at grid origin (x,y)  */ 

  line(x - 1,y_lower_limit,x - 1,y_upper_limit); 
  line(x,y_lower_limit,x,y_upper_limit); 

  /*  draw horizontal cross lines at grid origin (x,y)  */ 

  line(x_lower_limit,y - 1,x_upper_limit,y - 1); 
  line(x_lower_limit,y,x_upper_limit,y); 

  /*  draw vertical lines relative to grid origin (x,y) separated by
      specified horizontal spacing   */ 

  i = x - x_space; 
  while ((i >= x_lower_limit)) 
    { 
      i = i - x_space; 
      line(i,y_lower_limit,i,y_upper_limit); 
    } 

  i = x + x_space; 
  while ((i <= x_upper_limit)) 
    { 
      i = i + x_space; 
      line(i,y_lower_limit,i,y_upper_limit); 
    } 

  /*  draw horizontal lines relative to grid origin (x,y) separated by
      specified vertical spacing   */ 

  j = y - y_space; 
  while ((j >= y_lower_limit)) 
    { 
      j = j - y_space; 
      line(x_lower_limit,j,x_upper_limit,j); 
    } 

  j = y + y_space; 
  while ((j <= y_upper_limit)) 
    { 
      j = j + y_space; 
      line(x_lower_limit,j,x_upper_limit,j); 
    } 
  return (status);
} 
#endif

