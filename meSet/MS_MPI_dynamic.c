#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <X11/Xlib.h>
#include <string.h>
#include <unistd.h>

#define PRINT_TIME 1
#define IF_PRINT   1
#define CENTER_NODE 0
#define TAG_TASK    0
#define TAG_POINT   1

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x>y?y:x )

typedef struct complextype
{
	double real, imag;
} Compl;

typedef struct Task{
  int process_handle_start_x;
  int process_handle_count_x;
}Task;

typedef struct DrawPoint
{
		int repeats;
    short x,y;
}DrawPoint;


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


/* set window size */
int width = 800;
int height = 800;



/* set window position */
int x_position = 0;
int y_position = 0;

/**start struct meta**/
    int size, rank, actual_size;
/**end struct**/


/**start struct time**/
  double communication_time = .0;
  double compution_time = .0;
  double total_time = .0;
/**end struct time**/


int max_loop = 100000;
int transfer_size = 10;

Task *task;
int task_dispacher_pointer = 0;
int task_complete_pointer = 0;
int *number_of_task_per_thread ;
DrawPoint* processes_points;
MPI_Datatype mpi_point_type;
MPI_Datatype mpi_task_type;

int ask_for_task_message = 1;

/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

/**/
void my_init(int argc,char *argv[]);
void my_excute();
void my_summatize();
void my_excute_as_single_node();

void my_draw();


int getTask();
void handleTask();
void dispatchTask();
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
		if(actual_size>0)
		{
			my_excute();
			my_summatize();
		}
		else
		{
			my_excute_as_single_node();
		}



    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

    if(PRINT_TIME)
    {
        printf( "total_time: %f\n communication_time: %f\n compution_time: %f\n",
            										total_time, communication_time, compution_time );
    }

		my_draw();

    MPI_Finalize();
}



void my_excute()
{
	if(rank==0)
	{
		 dispatchTask();
	}
	else
	{
		while(getTask())
		{
			handleTask();
		}
	}
}


void my_excute_as_single_node()
{
	task->process_handle_count_x = parameters.number_of_points_x;
	task->process_handle_start_x = 0;
	handleTask();
}

void handleTask()
{
	/* draw points */
	Compl z, c;
	int repeats;
	double temp, lengthsq;
	int i, j, k;

	for( k=0; k<task->process_handle_count_x; k++)
  {

    i = k + task->process_handle_start_x;
		for(j=0; j<parameters.number_of_points_y; j++) {
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

      processes_points[task_complete_pointer].x = i;
      processes_points[task_complete_pointer].y = j;
      processes_points[task_complete_pointer++].repeats = repeats;

		}
	}
}


void my_summatize()
{
	int i;

  if (rank==0)
  {
		task_complete_pointer = 0;
    for ( i = 1; i < actual_size; i++)
    {
      myRecv(
        &processes_points[ task_complete_pointer ],
        number_of_task_per_thread[i],
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
            task_complete_pointer,
            mpi_point_type,
            CENTER_NODE,
            TAG_POINT,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
          );
  }
}

int getTask()
{
	mySendrecv(&ask_for_task_message, 1, MPI_INT, CENTER_NODE, TAG_TASK,
							task, 1, mpi_task_type, CENTER_NODE, TAG_TASK,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	return task->process_handle_count_x;
}


void dispatchTask()
{
	MPI_Status status;
	int i;


	while (task_dispacher_pointer<parameters.number_of_points_x) {

		task->process_handle_start_x = task_dispacher_pointer;
		task->process_handle_count_x = min(parameters.number_of_points_x - task_dispacher_pointer, transfer_size);
		task_dispacher_pointer = task->process_handle_count_x;


		myRecv(&ask_for_task_message, 1, MPI_INT, MPI_ANY_SOURCE, TAG_TASK, MPI_COMM_WORLD, &status);
		mySend(task, 1, mpi_task_type, status.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE );


		number_of_task_per_thread[status.MPI_SOURCE] += task->process_handle_count_x;
		//merge.
	}

	task->process_handle_count_x = 0;
	for ( i=0; i < actual_size; i++)
	{
		myRecv(&ask_for_task_message, 1, MPI_INT, MPI_ANY_SOURCE, TAG_TASK, MPI_COMM_WORLD, &status);
		mySend(task, 1, mpi_task_type, status.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
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

void my_init(int argc,char *argv[])
{
  /**init excute parameters.**/

	int index;
      if(argc<3)
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

			my_create_type();
			task = (Task*)malloc(sizeof(Task));
			if (rank == 0)
      {
				number_of_task_per_thread = (int*)malloc( sizeof(int)*size );
				for ( index = 0; index < size; index++) {
					number_of_task_per_thread[index] = 0;
				}

        processes_points = (DrawPoint*) malloc( sizeof(DrawPoint) *
              parameters.number_of_points_x * parameters.number_of_points_y );
      }
      else
      {
        processes_points = (DrawPoint*) malloc( sizeof(DrawPoint) *
              parameters.number_of_points_x * parameters.number_of_points_y );
      }

}


void my_create_type()
{
	const int points_nitems = 3;
	const int task_nitems = 2;
	MPI_Datatype points_types[3] = {MPI_INT, MPI_SHORT, MPI_SHORT};
  MPI_Aint     points_offsets[3];
  int          points_blocklengths[3] = {1,1,1};
  points_offsets[0] = offsetof(DrawPoint, repeats);
  points_offsets[1] = offsetof(DrawPoint, x);
  points_offsets[2] = offsetof(DrawPoint, y);

  MPI_Type_create_struct(points_nitems, points_blocklengths, points_offsets, points_types, &mpi_point_type);
  MPI_Type_commit(&mpi_point_type);



	MPI_Datatype task_types[2] = {MPI_INT, MPI_INT};
  MPI_Aint     task_offsets[2];
  int          task_blocklengths[2] = {1,1};
  task_offsets[0] = offsetof(Task, process_handle_start_x);
  task_offsets[1] = offsetof(Task, process_handle_count_x);
	MPI_Type_create_struct(task_nitems, task_blocklengths, task_offsets, task_types, &mpi_task_type);
  MPI_Type_commit(&mpi_task_type);
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
