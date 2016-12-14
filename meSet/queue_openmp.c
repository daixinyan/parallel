/*
	openmp Mandelbort sort
*/
#include <X11/Xlib.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAXSIZE 640000
#define ADDGETSIZE 40
#define IF_PRINT 1

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x>y?y:x )



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
typedef struct Queue
{
	DrawPoint* data;
	int front;
	int rear;
	int size;
}Queue;
Queue* CreateQueue(int size);
int AddQ(Queue* q, int repeats, int x, int y);
int DeleteQ(Queue* q,Queue* deleteQueue) ;
int GetQ(Queue* q, DrawPoint* point);

void my_excute_calculate();
void my_excute_draw();
void my_init_x11();
void my_init(int argc,char *argv[]);
void my_main_excute();

int max_loop = 100000;

Display *display;
Window window;      /*initialization for a window*/
int screen;         /*which screen*/
/* create graph */
GC gc;
/* set window size */
int width = 800;
int height = 800;
/* set window position */
int x = 0;
int y = 0;
/* border width in pixels */
int border_width = 0;

Queue* queue;
Queue* deleteQueue;
DrawPoint temp;

int main(int argc,char *argv[])
{

	clock_t start_clock = clock();
	time_t  start_time = time(NULL);


	my_init(argc,argv);
	my_init_x11();



	if(display == NULL) {
		return 0;
	}

	my_main_excute();


	XFlush(display);
	clock_t end_clock = clock();
	time_t  end_time = time(NULL);
	printf("CLOCK:  %ld\n",(end_clock-start_clock)/CLOCKS_PER_SEC);
	printf("TIME:  %ld\n",(end_time-start_time) );


	sleep(5);
	return 0;
}

void my_main_excute()
{
	queue = CreateQueue(MAXSIZE);
	deleteQueue = CreateQueue(ADDGETSIZE);
	omp_set_nested(1);

	Compl z, c;

	double temp, lengthsq;
	int repeats;
	int i=0, j=0;
#pragma omp parallel for  private(i,j,z,c,temp,lengthsq,repeats) schedule(dynamic,10)
	for(i=-10; i<parameters.number_of_points_x; i++)
	{
		if (i==-10)
		{
			my_excute_draw();
		}
		else if (i>=0)
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
				int added = 0;
				while(!added)
				{
#pragma omp critical
					{
						added = AddQ(queue,repeats,i*width/parameters.number_of_points_x,j*height/parameters.number_of_points_y);
					}
				}
			}
		if(IF_PRINT&&(i==width-1))
		{
			printf("done with calculating.\n");
		}
	}

}


void my_excute_calculate()
{
	Compl z, c;

	double temp, lengthsq;
	int repeats;
	int i=0, j=0;
#pragma omp parallel for  private(i,j,z,c,temp,lengthsq,repeats) schedule(dynamic,1)
	for(i=0; i<width; i++)
	{
		for(j=0; j<height; j++)
		{
			repeats = 0;
			z.real = 0.0;
			z.imag = 0.0;
			c.real = (double)i/(double)width*4.0 - 2.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
			c.imag = (double)j/(double)height*4.0 - 2.0; /* So needs to scale the window */
			lengthsq = 0.0;

			while(repeats < 1000000 && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
				temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real + z.imag*z.imag;
				repeats++;
			}
			int added = NULL;
			while(!added)
			{
#pragma omp critical
				{
					added = AddQ(queue,repeats,i,j);
				}
			}
		}

	}
}


void my_excute_draw()
{
	int i,j,result;
	DrawPoint* point;
	for(i=0; i<parameters.number_of_points_x*parameters.number_of_points_y;)
	{
		result = 0;
		while(result==0)
		{
#pragma omp critical
			{
				result = DeleteQ(queue,deleteQueue);
				// result = GetQ(queue, &temp);
			}
		}
		for (j = 0; j < result; j++)
		{

			point = &(deleteQueue->data[j]);
			XSetForeground (display, gc,  1024 * 1024 * (point->repeats % 256));
			XDrawPoint (display, window, gc, point->x, point->y);
		}

		i+=result;
	}
}


void my_init_x11()
{

	/* open connection with the server */
	display = XOpenDisplay(NULL);
	if(display == NULL) {
		fprintf(stderr, "cannot open display\n");
		return;
	}

	screen = DefaultScreen(display);
	/* create window */
	window = XCreateSimpleWindow(
			display, RootWindow(display, screen),
			x, y, width, height, border_width,
			BlackPixel(display, screen),
			WhitePixel(display, screen)
	);

	/* create graph */
	XGCValues values;
	long valuemask = 0;

	gc = XCreateGC(display, window, valuemask, &values);
	/*XSetBackground (display, gc, WhitePixel (display, screen));*/
	XSetForeground (display, gc, BlackPixel (display, screen));
	XSetBackground(display, gc, 0X0000FF00);
	XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

	/* map(show) the window */
	XMapWindow(display, window);
	XSync(display, 0);
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
	parameters.real_range = parameters.right_range_of_real - parameters.left_range_of_real;
	parameters.imag_range = parameters.upper_range_of_imag - parameters.lower_range_of_imag;
	border_width = parameters.number_of_points_x/width;
}


Queue* CreateQueue(int size) {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->data = (DrawPoint*)malloc(sizeof(DrawPoint)*size);
	if ( (!q) || (!q->data) ) {
		printf("no enough space.\n");
		return NULL;
	}
	q->front = -1;
	q->rear = -1;
	q->size = 0;
	return q;
}


int AddQ(Queue* q, int repeats, int x, int y) {
	if((q->size == MAXSIZE))
	{
		return 0;
	}
	q->rear++;
	if(q->rear==MAXSIZE)
	{
		q->rear = 0;
	}
	q->size++;

	q->data[q->rear].repeats = repeats;
	q->data[q->rear].x = x;
	q->data[q->rear].y = y;

	return 1;
}


int GetQ(Queue* q,DrawPoint* point) {
	if(q->size == 0)
	{
		return 0;
	}
	q->front++;
	if(q->front == MAXSIZE)
	{
		q->front = 0;
	}
	q->size--;
	point->repeats = q->data[q->front].repeats;
	point->x = q->data[q->front].x;
	point->y = q->data[q->front].y;
	return 1;
}

int DeleteQ(Queue* q, Queue* deleteQueue) {

	int count = min ( ADDGETSIZE , q->size );
	int l = min( count, MAXSIZE-q->front -1);
	//int i;
	//for ( i = 0; i < count; i++) {
	//	q->front++;
	//	if(q->front == MAXSIZE)
	//	{
	//		q->front = 0;
	//	}
	//	memcpy( &(deleteQueue->data[i]), &(q->data[q->front]), sizeof(DrawPoint));
	//	deleteQueue->data[i].repeats = q->data[q->front].repeats;
	//	deleteQueue->data[i].x = q->data[q->front].x;
	//	deleteQueue->data[i].y = q->data[q->front].y;
	//}
	memcpy( &(deleteQueue->data[0]), &(q->data[q->front+1]), l*sizeof(DrawPoint));
	memcpy( &(deleteQueue->data[l]), &(q->data[0]), (count-l)*sizeof(DrawPoint));
	q->size -= count;
	q->front += count;
	if(q->front >= MAXSIZE) q->front -= MAXSIZE;
	deleteQueue->size = count;
	return count;
}
