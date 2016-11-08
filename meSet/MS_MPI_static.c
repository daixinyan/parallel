#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <X11/Xlib.h>

#define PRINT_TIME 1
#define IF_PRINT   1

/**start struct meta**/
    int size,rank;
/**end struct**/


/**start struct time**/
  double communication_time = .0;
  double compution_time = .0;
  double total_time = .0;
/**end struct time**/

typedef struct complextype
{
	double real, imag;
} Compl;

typedef struct DrawPoint
{
		int repeats;
    short x,y;
}DrawPoint;
/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
int myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
int mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

void my_test();

int main(int argc,char *argv[])
{

    /**record time**/
    double total_start_time;
    double total_end_time;

    double start_time;
    double end_time;


    /**init mpi**/
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

    total_start_time = MPI_Wtime();


    /**init excute parameters.**/
        if(argc<3)
        {
        }
        else
        {
        }

        my_test();

    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

    if(PRINT_TIME)
    {
        printf("total_time: %f\n communication_time: %f\n compution_time: %f\n",
            total_time, communication_time, compution_time );
    }

    MPI_Finalize();
}


void my_test()
{
  Display *display;
	Window window;      //initialization for a window
	int screen;         //which screen

	/* open connection with the server */
	display = XOpenDisplay(NULL);
	if(display == NULL) {
		fprintf(stderr, "cannot open display\n");
		return 0;
	}

	screen = DefaultScreen(display);

	/* set window size */
	int width = 800;
	int height = 800;

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

	/* draw points */
	Compl z, c;
	int repeats;
	double temp, lengthsq;
	int i, j;


  if(rank!=1)
  {
    return;
  }
	for(i=0; i<width; i++) {
		for(j=0; j<height; j++) {
			z.real = 0.0;
			z.imag = 0.0;
			c.real = ((double)i - 400.0)/200.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
			c.imag = ((double)j - 400.0)/200.0; /* So needs to scale the window */
			repeats = 0;
			lengthsq = 0.0;

			while(repeats < 100000 && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
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

void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status)
{
  double start_time;
  double end_time;
  start_time = MPI_Wtime();
  MPI_Sendrecv(sendbuf, sendcount, sendtype,dest, sendtag,
    recvbuf, recvcount, recvtype,source, recvtag, comm,  status);
  end_time = MPI_Wtime();
  communication_time += end_time-start_time;
}



int myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}




int mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}
