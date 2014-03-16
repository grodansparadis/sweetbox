//******************************************************************************************
//*                                                                                        *                                                   
//*           Name:  graphutil.h                                                           *                              
//* Project Number:  MT2192                                                                *                                                   
//*           Date:  10/15/2007                                                            *                                                  
//*                                                                                        *
//*   Description:                                                                         * 
//*     this unit contains useful graphics and scaling transformation utilities and other  *
//* miscellaneous graphics procedures to scale a sensor or display a cursor, grid, etc.    *
//*                                                                                        *
//******************************************************************************************


//#include <graphics.h>

#ifndef GRAPHUTIL_LOADED  /* prevent multiple loading */
#define GRAPHUTIL_LOADED

/* declare general purpose constants here */

//extern int TRUE;                             /* True value */
//extern int FALSE;                            /* False value */
extern float eps;                            /* small number */

/* declare global graphics window mapping variables here */

extern float xw_min;                                   
extern float yw_min;
extern float xw_max;
extern float yw_max;

/* declare global graphics viewport mapping variables here */

extern int xv_min;
extern int yv_min;
extern int xv_max; 
extern int yv_max; 

/*  declare current keyboard mouse coordinates */

extern int x_key_mouse, y_key_mouse;   
extern int vmin_mouse, vmax_mouse, hmin_mouse, hmax_mouse; 

/* declare important graphics variables here */

extern struct viewporttype current_viewport;           /*  contains the viewport
                                                    information for current 
                                                    graphics mode  */ 
extern int graph_driver;                               /*  graph driver number  */ 
extern int graph_mode;                                 /*  current graph mode  */ 
extern int error_code;                                 /*  error return code  */ 
extern int xasp, yasp; 

extern float xw1, yw1, xw2, yw2;                       /*  world coordinates  */ 

/* declare usefull graphics transformation functions here */

extern int set_window (float x1, float y1, float x2, float y2);
extern int set_view (int x1, int y1, int x2, int y2);
extern int map_window_to_viewport (float xw, float yw, int *xv, int *yv);
extern int map_viewport_to_window (int xv, int yv, float *xw, float *yw);

#ifdef NEW_CODE

/* declare usefull graphics drawing functions here */

extern int cursor(int x, int y, int old_x, int old_y);
extern int draw_grid(int x, int y, int x_space, int y_space);
#endif
 
#endif
