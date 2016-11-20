#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <X11/Xlib.h>
#include <string.h>
#include <unistd.h>

#define PRINT_TIME  1
#define IF_PRINT    1
#define CENTER_NODE 0
#define TAG_TASK    0
#define TAG_POINT   1

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x>y?y:x )

int max_loop = 100000;

typedef struct complextype
{
	double real, imag;
} Compl;


typedef struct Task{
  int process_handle_start_x;
  int process_handle_count_x;
}Task;


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


typedef struct RecordDrawPoint
{
	 int repeats;
	 short x,y;
}DrawPoint;



/**start struct meta**/
    int size, rank, actual_size;
/**end struct**/
/* set window size */
/* set window size */
int width ;
int height ;

    Task* processes_task;
    DrawPoint* processes_points;

		MPI_Datatype mpi_point_type;

/**start struct time**/
    double communication_time = .0;
    double compution_time = .0;
    double total_time = .0;
/**end struct time**/




/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv( const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

/**/
void my_init(int argc,char *argv[]);

/**/
void my_excute();


void my_draw();

/**/
void my_summatize();

/**/
void my_create_type();


int main(int argc,char *argv[])
{

    /**record time**/
    double total_start_time;
    double total_end_time;




    /**init mpi**/
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

    total_start_time = MPI_Wtime();



        my_init(argc, argv);
        my_excute();
				my_summatize();



    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

		if(PRINT_TIME)
    {
        printf( "rank: %d\n total_time: %f\n communication_time: %f\n compution_time: %f\n",
            										rank, total_time, communication_time, compution_time );
    }
			my_draw();

    MPI_Finalize();
}


void my_excute()
{
  /* draw points */
	Compl z, c;
	int repeats;
	double temp, lengthsq;
	int i=rank, j=0, k=0;
	for( k=0; k<processes_task[rank].process_handle_count_x; k++)
  	{

		for(j=0; j<parameters.number_of_points_y; j++)
		{
			z.real = 0.0;
			z.imag = 0.0;
      			c.real = (double)i / (double)width * parameters.real_range- parameters.real_range/2; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
      			c.imag = (double)j / (double)height * parameters.imag_range - parameters.imag_range/2; /* So needs to scale the window */
			repeats = 0;
			lengthsq = 0.0;

			while(repeats < max_loop && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
				temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real + z.imag*z.imag;
				repeats++;
			}

			processes_points[k*parameters.number_of_points_y+j].x = i;
      			processes_points[k*parameters.number_of_points_y+j].y = j;
      			processes_points[k*parameters.number_of_points_y+j].repeats = repeats;

		}
		i += actual_size;
	}
}


void my_summatize()
{
  int i;

  if (rank==0)
  {
    for ( i = 1; i < actual_size; i++)
    {
      myRecv(
        &processes_points[ processes_task[i].process_handle_start_x * parameters.number_of_points_y ],
        processes_task[i].process_handle_count_x * parameters.number_of_points_y,
        mpi_point_type,
        i,
        TAG_POINT,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE
      );
    }
  }
  else if(actual_size>rank)
  {
    mySend( processes_points,
            processes_task[rank].process_handle_count_x * parameters.number_of_points_y,
            mpi_point_type,
            CENTER_NODE,
            TAG_POINT,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
          );
  }
}

void my_draw()
{
	if(rank==0 && parameters.is_enable)
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

void my_create_type()
{
	const int nitems=3;
  MPI_Datatype types[3] = {MPI_INT, MPI_SHORT, MPI_SHORT};
  MPI_Aint     offsets[3];
  int          blocklengths[3] = {1,1,1};
  offsets[0] = offsetof(DrawPoint, repeats);
  offsets[1] = offsetof(DrawPoint, x);
  offsets[2] = offsetof(DrawPoint, y);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);
}

void my_init(int argc,char *argv[])
{
  /**init excute parameters.**/
      if(argc<8)
      {
        parameters.number_of_threads = 8;
        parameters.left_range_of_real = -2;
        parameters.right_range_of_real = 2;
        parameters.lower_range_of_imag = -2;
        parameters.upper_range_of_imag = 2;
        parameters.number_of_points_x = 800;
        parameters.number_of_points_y = 800;
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

      parameters.real_range = parameters.right_range_of_real - parameters.left_range_of_real;
      parameters.imag_range = parameters.upper_range_of_imag - parameters.lower_range_of_imag;

			width = parameters.number_of_points_x;
			height = parameters.number_of_points_y;

      int reminder,quotient,index;
      reminder = parameters.number_of_points_x % size;
      quotient = parameters.number_of_points_x / size;
      actual_size = quotient==0?reminder:size;
      processes_task = (Task*) malloc( sizeof(Task) * size );
      for ( index = 0; index <size; index++)
      {
        if (index>=reminder)
        {
          processes_task[index].process_handle_start_x = quotient * index + reminder;
          processes_task[index].process_handle_count_x = quotient;
        }else
        {
          processes_task[index].process_handle_start_x = ( quotient + 1 ) * index;
          processes_task[index].process_handle_count_x = quotient + 1 ;
        }
      }


      if (rank == 0)
      {
        processes_points = (DrawPoint*) malloc( sizeof(DrawPoint) *
              parameters.number_of_points_x * parameters.number_of_points_y );
      }
      else
      {
        processes_points = (DrawPoint*) malloc( sizeof(DrawPoint) *
              processes_task[rank].process_handle_count_x * parameters.number_of_points_y );
      }

			my_create_type();


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



void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}




void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}
