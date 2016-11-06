 /*
   openmp Mandelbort sort
 */
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAXSIZE 6400
#define IF_PRINT 1

int max_loop = 100000;

typedef struct complextype
{
	double real, imag;
} Compl;



typedef struct DrawPoint
{
    short x,y;
    int repeats;
}DrawPoint;
typedef struct Queue
{
    DrawPoint* data;
    int front;
    int rear;
    int size;
}Queue;
Queue* CreateQueue();
DrawPoint* AddQ(Queue* q, int repeats, int x, int y);
DrawPoint* DeleteQ(Queue* q) ;
DrawPoint* AddDeleteQ(Queue* q, int repeats, int x, int y);

void my_excute_calculate();
void my_excute_draw();
void my_init_x11();
void my_main_excute();

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

int main(void)
{

    clock_t start_clock = clock();
    time_t  start_time = time(NULL);


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
        queue = CreateQueue();
        omp_set_nested(1);

        Compl z, c;

                double temp, lengthsq;
                int repeats;
                int i=0, j=0;
                #pragma omp parallel for  private(i,j,z,c,temp,lengthsq,repeats) schedule(dynamic,10)
                for(i=-10; i<width; i++)
                {
                    if (i==-1)
                    {
                        my_excute_draw();
                    }
                    else if (i>=0)
                    for(j=0; j<height; j++)
                    {
                        repeats = 0;
                        z.real = 0.0;
                        z.imag = 0.0;
                        c.real = (double)i/(double)width*4.0 - 2.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
                        c.imag = (double)j/(double)height*4.0 - 2.0; /* So needs to scale the window */
                        lengthsq = 0.0;

                        while(repeats < max_loop && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
                            temp = z.real*z.real - z.imag*z.imag + c.real;
                            z.imag = 2*z.real*z.imag + c.imag;
                            z.real = temp;
                            lengthsq = z.real*z.real + z.imag*z.imag;
                            repeats++;
                        }
                        DrawPoint* added = NULL;
                        while(!added)
                        {

                            printf("add....");
                            {
                                added = AddDeleteQ(queue,repeats,i,j);
                            }
                        }
												printf("add one.");
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
                        DrawPoint* added = NULL;
                        while(!added)
                        {
                            {
                                added = AddDeleteQ(queue,repeats,i,j);
                            }
                        }
                    }

                }
    }

    void my_excute_draw()
    {
        int i;
        for(i=0; i<width*height; i++)
        {
            DrawPoint* point = NULL;
            while(point==NULL)
            {
								printf("drwaing.");

                {
                    point = AddDeleteQ(queue, -1, 0, 0);
                }
            }
	    					printf("drawing one.");

            XSetForeground (display, gc,  1024 * 1024 * (point->repeats % 256));
            XDrawPoint (display, window, gc, point->x, point->y);
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


Queue* CreateQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
		q->data = (DrawPoint*)malloc(sizeof(DrawPoint)*MAXSIZE);
		if ( (!q) || (!q->data) ) {
        printf("no enough space.\n");
        return NULL;
    }
    q->front = -1;
    q->rear = -1;
    q->size = 0;
    return q;
}

DrawPoint* AddDeleteQ(Queue* q, int repeats, int x, int y)
{
		DrawPoint* re = NULL;
		#pragma omp critical
		{
			if(repeats<0)
			{
					re =  DeleteQ(q);
			}else
			{
					re =  AddQ(q, repeats, x, y);
					printf("add deleted: add\n" );
					if(re==NULL) printf("null\n" );
					else printf("is not NULL\n" );
			}
		}
		return re;
}


DrawPoint* AddQ(Queue* q, int repeats, int x, int y) {
    if((q->size == MAXSIZE))
    {
			printf("qsize %d ==MAXSIZE %d \n",q->size, MAXSIZE );
        return NULL;
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

    return &(q->data[q->rear]);
}


DrawPoint* DeleteQ(Queue* q) {
    if(q->size == 0)
    {
        return NULL;
    }
    q->front++;
    if(q->front == MAXSIZE)
    {
        q->front = 0;
    }
    q->size--;
    return &(q->data[q->front]);
}
