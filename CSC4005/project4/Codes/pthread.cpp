#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>


using namespace std;
pthread_barrier_t b;
int iter_num  = 5000;
int local_size;
int offset;
int NUM_THREAD;
int size;
double *data, *tmp_data;
void *calculate(void *t){
    int *my_id = (int *)t;
    int sub_size = local_size;
    if(*my_id == NUM_THREAD-1) sub_size += offset;
    int idx = local_size*(*my_id);

    for (int j=idx;j<idx+sub_size;j++){
        if((j%size) != 0 && ((j+1)%size) !=0 && j >= size && j < size*size-size){
            tmp_data[j] = 0.25*(data[j-1]+data[j+1]+data[j-size]+data[j+size]);
        }
    }
    pthread_barrier_wait(&b);
    
    //update
    for (int j=idx;j<idx+sub_size;j++){
        if((j%size) != 0 && ((j+1)%size) != 0 && j >= size && j < size*size-size){
            data[j] = tmp_data[j];
        }
    }
}
int main (int argc,char *argv[]){
    
    size = atoi(argv[1]);
    NUM_THREAD = atoi(argv[2]);
    pthread_t threads[NUM_THREAD];
    int *thread_ids = (int *) malloc(sizeof(int) * NUM_THREAD);
    local_size = size*size/NUM_THREAD;
    offset = (size*size) % NUM_THREAD;
    
    
//    prepare the background
//    Draw the graphic, copy from exanple codes
    Window          win;
    char            window_name[] = "test", *display_name = NULL;                     /* initialization for a window */
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    GC*             gcs;
    unsigned long            valuemask = 0;
    XGCValues       values; //value of the graphics content
    XSizeHints      size_hints;
    Pixmap          bitmap;
    XSetWindowAttributes attr[1];
    int             width, height,                  /* window size */
                    x, y,                           /* window position */
                    border_width,                   /*border width in pixels */
                    display_width, display_height,  /* size of screen */
                    screen;                         /* which screen */

    if (  (display = XOpenDisplay (display_name)) == NULL ) {
       fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
      exit (-1);
      }

    /* get screen size */
    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* set window size */
    width = 200;
    height = 200;

    /* set window position */
    x = 0;
    y = 0;

    /* create opaque window */

    border_width = 4;
    win = XCreateSimpleWindow (display, RootWindow (display, screen),
                          x, y, width, height, border_width,
                          WhitePixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

    size_hints.flags = USPosition|USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;

    XSetNormalHints (display, win, &size_hints);
    XStoreName(display, win, window_name);

    /* create graphics context */
    gcs = (GC *) malloc (((100-20)/5+1)*sizeof(GC));
    for (int i = 0; i < ((100-20)/5+1); i++) {
        gc = XCreateGC (display, win, valuemask, &values);
        gcs[i] = gc;
    }

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel (display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);

    XFlush (display);

    XColor color;

    for (int i = 0; i < ((100-20)/5+1); i++) {
        color.red = 4095*i;
        color.green = 0;
        if(i!=0 && i!=16){
            color.green = i*3000;
        }
        color.blue = 65535-4095*i;
        Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color); //allocate the color using the color map.
        //set the color and attribute of the graphics content
        XSetForeground (display, gcs[i], color.pixel);
        XSetBackground (display, gcs[i], BlackPixel (display, screen));
        XSetLineAttributes (display, gcs[i], 1, LineSolid, CapRound, JoinRound);
    }
//
    pthread_barrier_init(&b,NULL,NUM_THREAD);
    data = (double *) malloc ((size * size)*sizeof(double));
    tmp_data = (double *) malloc ((size * size)*sizeof(double));
    // set the original temperature in the room
    for (int i = 0; i < size*size; i++) {
        data[i] = 20;
    }
    for (int i = -size/12; i < size/12; i++ ){
        data[size/2+i] = 100;
    }
    
    struct timeval timeStart, timeEnd, timeSystemStart;
    double runTime=0, systemRunTime;
    gettimeofday(&timeStart, NULL );
    
    int rc;
    int i;
    for(int k =0; k<iter_num;k++){
        for(i = 0; i<NUM_THREAD; i++){
            thread_ids[i] = i;
            rc = pthread_create(&threads[i], NULL, calculate,(void*)&thread_ids[i]);
            if(rc){
                printf("ERROR: return code from pthread_create() is %d", rc);
                exit(1);
            }
        }
        for(i = 0; i<NUM_THREAD; i++){
            pthread_join(threads[i], NULL);
        }
        
        if(k%5 == 0){
            int level;
            for(int j = 0; j< size*size; j++){
                level = (data[j]-20)/5;
                XDrawPoint (display, win, gcs[level], j%(size)*200/size, floor(j/(size))*200/size);
            }
            XFlush (display);
        }
    }
//        printf("300 %f\n",data[300]);
                //display
        
        
    gettimeofday( &timeEnd, NULL );
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    printf("Name: Yiru Mao\n");
    printf("Student ID: 118010223\n");
    printf("Assignment 4, Heat Distribution, pthread implementation.\n");
    cout<<"thread: "<<NUM_THREAD<< "runTime is "<<runTime<<"s"<<endl;
    return 0;
}
