#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <X11/Xlib.h>
#include <string.h>

#define PRINT_TIME  1
#define IF_PRINT    1
#define CENTER_NODE 0
#define TAG_TASK    0
#define TAG_POINT   1

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x>y?y:x )

typedef struct complextype
{
	double real, imag;
} Compl;


/**start struct meta**/
    int size, rank, actual_size;
/**end struct**/
/* set window size */
		int width = 800;
		int height = 800;
		int max_loop = 10000;

		int x_location =0 ;
		int y_location =0 ;



		Display *display;
		Window window;      /*initialization for a window*/
		int screen;         /*which screen*/
		/* create graph */
		GC gc;
		/* set window size */
    /* border width in pixels */
    int border_width = 0;



int main(int argc,char *argv[])
{
    /**init mpi**/
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);


    if(rank==0)
    {

      /* open connection with the server */
      display = XOpenDisplay(NULL);
      if(display == NULL) {
        fprintf(stderr, "cannot open display\n");
        return 0;
      }

      screen = DefaultScreen(display);




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

      /* draw points */
      Compl z, c;
      int repeats;
      double temp, lengthsq;
      int i, j;
      for(i=0; i<width; i++) {
        for(j=0; j<height; j++) {
          z.real = 0.0;
          z.imag = 0.0;
          c.real = ((double)i - 400.0)/200.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
          c.imag = ((double)j - 400.0)/200.0; /* So needs to scale the window */
          repeats = 0;
          lengthsq = 0.0;

          while(repeats < max_loop && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
            temp = z.real*z.real - z.imag*z.imag + c.real;
            z.imag = 2*z.real*z.imag + c.imag;
            z.real = temp;
            lengthsq = z.real*z.real + z.imag*z.imag;
            repeats++;
          }

          XSetForeground (display, gc,  1024 * 1024 * (repeats % 256));
          XDrawPoint (display, window, gc, i, j);
        }
      }
      XFlush(display);
      sleep(5);
    }





    MPI_Finalize();
}
