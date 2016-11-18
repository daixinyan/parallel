/*
	openmp Mandelbort sort
*/
#include <X11/Xlib.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define MAXSIZE 640000
#define ADDGETSIZE 40
#define IF_PRINT 1

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x>y?y:x )


 int max_loop = 100000;
typedef struct complextype
{
 double real, imag;
} Compl;

struct
{
 int number_of_threads;
 double left_range_of_real;
 double right_range_of_real;
 double real_range;
 double lower_range_of_imag;
 double upper_range_of_imag;
 double imag_range;
 int number_of_points_x;
 int number_of_points_y;
 int is_enable;
}parameters;

typedef struct DrawPoint
{
	 int repeats;
	 short x,y;
}DrawPoint;


void my_init(int argc,char *argv[]);
void my_excute();
void my_draw();

 /* set window size */
 int width = 800;
 int height = 800;


 DrawPoint* processes_points;

int main(int argc,char *argv[])
{

	 clock_t start_clock = clock();
	 time_t  start_time = time(NULL);
   struct timeval start,end;
   gettimeofday(&start, NULL );


	 my_init(argc,argv);
	 my_excute();
   clock_t end_clock = clock();
   time_t  end_time = time(NULL);
   printf("CLOCK:  %ld\n",(end_clock-start_clock)/CLOCKS_PER_SEC);
   printf("TIME:  %f\n",difftime(end_time, start_time) );
   gettimeofday(&end, NULL );
   long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
   printf("time=%f\n",timeuse /1000000.0);

   my_draw();

   return 0;
}

	 void my_excute()
	 {

			 Compl z, c;

							 double temp, lengthsq;
							 int repeats;
							 int i=0, j=0;
							 #pragma omp parallel for  private(i,j,z,c,temp,lengthsq,repeats) schedule(static,10)
							 for(i=0; i<parameters.number_of_points_x; i++)
							 {
                   int aaa = omp_get_num_threads();
                   printf("%d\n",aaa );
									 for(j=0; j<parameters.number_of_points_y; j++)
									 {
											 repeats = 0;
											 z.real = 0.0;
											 z.imag = 0.0;

											 c.real = (double)i/(double)width*parameters.real_range- parameters.real_range/2; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
											 c.imag = (double)j/(double)height*parameters.imag_range - parameters.imag_range/2; /* So needs to scale the window */

											 lengthsq = 0.0;

											 while(repeats < max_loop && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
													 temp = z.real*z.real - z.imag*z.imag + c.real;
													 z.imag = 2*z.real*z.imag + c.imag;
													 z.real = temp;
													 lengthsq = z.real*z.real + z.imag*z.imag;
													 repeats++;
											 }
											 processes_points[ i*parameters.number_of_points_y + j].x = i ;
											 processes_points[ i*parameters.number_of_points_y + j].y = j ;
											 processes_points[ i*parameters.number_of_points_y + j].repeats = repeats ;
									 }
							 }

	 }


   void my_draw()
   {
   	if( parameters.is_enable)
   	{
   		Display *display;
   		Window window;      //initialization for a window
   		int screen;         //which screen

   		/* open connection with the server */
   		display = XOpenDisplay(NULL);
   		if(display == NULL) {
   			fprintf(stderr, "cannot open display\n");
   			return;
   		}

   		screen = DefaultScreen(display);

   		/* set window position */
   		int x = 0;
   		int y = 0;

   		/* border width in pixels */
   		int border_width = 0;

   		/* create window */
   		window = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, width, height, border_width,
   						BlackPixel(display, screen), WhitePixel(display, screen));

   		/* create graph */
   		GC gc;
   		XGCValues values;
   		long valuemask = 0;

   		gc = XCreateGC(display, window, valuemask, &values);
   		//XSetBackground (display, gc, WhitePixel (display, screen));
   		XSetForeground (display, gc, BlackPixel (display, screen));
   		XSetBackground(display, gc, 0X0000FF00);
   		XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

   		/* map(show) the window */
   		XMapWindow(display, window);
   		XSync(display, 0);

   		int i=0;
   		DrawPoint* temp ;
   		for(i=0; i<parameters.number_of_points_x * parameters.number_of_points_y; i++)
   		{
   			temp = &processes_points[i];
   			XSetForeground (display, gc,  1024 * 1024 * (temp->repeats % 256));
   			XDrawPoint (display, window, gc, temp->x, temp->y);

   		}

   		XFlush(display);
   		sleep(5);

   	}
   }


	 void my_init(int argc,char *argv[])
	 {
		 /**init excute parameters.**/
				 if(argc<3)
				 {
					 parameters.number_of_threads = 8;
					 parameters.left_range_of_real = -2;
					 parameters.right_range_of_real = 2;
					 parameters.lower_range_of_imag = -2;
					 parameters.upper_range_of_imag = 2;
					 parameters.number_of_points_x = 400;
					 parameters.number_of_points_y = 400;
					 parameters.is_enable = 1;
				 }
				 else
				 {
					 parameters.number_of_threads = atoi(argv[1]);
					 parameters.left_range_of_real = atof(argv[2]);
					 parameters.right_range_of_real = atof(argv[3]);
					 parameters.lower_range_of_imag = atof(argv[4]);
					 parameters.upper_range_of_imag = atof(argv[5]);
					 parameters.number_of_points_x = atoi(argv[6]);
					 parameters.number_of_points_y = atoi(argv[7]);
					 parameters.is_enable = strlen("enable")==strlen(argv[8]);
				 }
         width = parameters.number_of_points_x;
         height = parameters.number_of_points_y;

				 parameters.real_range = parameters.right_range_of_real - parameters.left_range_of_real;
				 parameters.imag_range = parameters.upper_range_of_imag - parameters.lower_range_of_imag;

         processes_points = (DrawPoint*) malloc( sizeof(DrawPoint) *
               parameters.number_of_points_x * parameters.number_of_points_y );
	 }
