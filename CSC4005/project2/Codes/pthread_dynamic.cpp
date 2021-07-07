#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <pthread.h>
#include <queue>
using namespace std;

int X, Y, NUM_THREAD,sub_devide,next_core,num_task,finished_task;

int stop = 0;
queue<int> avail_core;
int * result_arr;
int * start_index_arr;
int * start_signal;

int MAG_THRE = 2;
int ITER_THRE = 100;
void printArray(int arr[], int arr_size);
void *cal_M_set(void *t);

void printArray(int arr[], int arr_size){
    for(int i = 0; i < arr_size; i++){
        cout<< arr[i] << " ";
    }
    cout<<endl;
    
}



void *cal_M_set(void *t){
    int *my_id = (int *)t;
    while(!stop){
        if(start_signal[*my_id] == 1){
            for(int idx = start_index_arr[*my_id]; idx<start_index_arr[*my_id]+sub_devide;idx++){
                int i = idx/X;
                int j = idx%X;
                double z_real, z_imag, c_real, c_imag;
                double z_mag, temp;
                z_real = z_imag = 0.0;
                c_real = ((float) j - X/2) / (X/4);
                c_imag = ((float) i - X/2) / (X/4);
                int k = 0;
                do {
                    temp = z_real * z_real - z_imag * z_imag + c_real;
                    z_imag = 2.0 * z_real * z_imag + c_imag;
                    z_real = temp;
                    z_mag = z_real * z_real + z_imag * z_imag;
                    k++;
                } while (z_mag < MAG_THRE*MAG_THRE && k <ITER_THRE);
                if(k == ITER_THRE){
                    result_arr[idx] = 1;
                }
            }
            avail_core.push(*my_id);
            finished_task++;
            start_signal[*my_id]=0;
        }
    }
    pthread_exit(NULL);
}

void *allocate_task(void *t){
    
    for(int p=0; p<NUM_THREAD;p++){
        avail_core.push(p);
    }
    int current_task = 0;
    
    while(finished_task<num_task){
        
        while(avail_core.empty()==0 && current_task<num_task){
            next_core = avail_core.front();
            avail_core.pop();
            start_index_arr[next_core] = sub_devide * current_task;
            start_signal[next_core] = 1;
            current_task++;

        }
    }

    stop = 1;
    pthread_exit(NULL);
}

int main (int argc, char* argv[]){
    X = Y = atoi(argv[1]);
//    printf("%d\n",X);
//    printf("update1");
    int array_size = X*X;
    num_task = 100;
    sub_devide = array_size/num_task;
    if(array_size%num_task != 0){
        sub_devide+=1;
    }
    array_size = num_task*sub_devide;
    NUM_THREAD = atoi(argv[2])-1;
//    printf("%d\n",NUM_THREAD);
    result_arr = (int *) malloc(sizeof(int) * array_size);
    start_index_arr = (int *) malloc(sizeof(int) * NUM_THREAD);
    start_signal = (int *) malloc(sizeof(int) * NUM_THREAD);
    pthread_t threads[NUM_THREAD+1];

    int *thread_ids = (int *) malloc(sizeof(int) * NUM_THREAD);
    int rc;
    int i;
    struct timeval timeStart, timeEnd, timeSystemStart;
    double runTime=0, systemRunTime;
    gettimeofday(&timeStart, NULL );
    if(NUM_THREAD == 0){
        for(int idx = 0; idx<X*X;idx++){
            int i = idx/X;
            int j = idx%X;
            double z_real, z_imag, c_real, c_imag;
            double z_mag, temp;
            z_real = z_imag = 0.0;
            c_real = ((float) j - X/2) / (X/4);
            c_imag = ((float) i - X/2) / (X/4);
            int k = 0;
            do {
                temp = z_real * z_real - z_imag * z_imag + c_real;
                z_imag = 2.0 * z_real * z_imag + c_imag;
                z_real = temp;
                z_mag = z_real * z_real + z_imag * z_imag;
                k++;
            } while (z_mag < MAG_THRE*MAG_THRE && k <ITER_THRE);
            if(k == ITER_THRE){
                result_arr[idx] = 1;
            }
        }
    }else{
        for(i = 0; i<NUM_THREAD; i++){
            thread_ids[i] = i;
            rc = pthread_create(&threads[i], NULL, cal_M_set,(void*)&thread_ids[i]);
            if(rc){
                printf("ERROR: return code from pthread_create() is %d", rc);
                exit(1);
            }
        }
        pthread_create(&threads[NUM_THREAD], NULL, allocate_task,(void*)&thread_ids[NUM_THREAD]);
        for(i = 0; i<NUM_THREAD; i++){
            pthread_join(threads[i], NULL);
        }
    }
    gettimeofday( &timeEnd, NULL );
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    printf("Name: Yiru Mao\n");
    printf("Student ID: 118010223\n");
    printf("Assignment 2, Mandelbrot Set, Pthread dynamic implementation.\n");
    cout<<"thread:"<<NUM_THREAD+1<< ",runTime is "<<runTime<<"s"<<endl;
//    printArray(result_arr,X*Y);
    
    
    
	//Draw the graphic, copy from exanple codes
    Window          win;
    char            *window_name = "test", *display_name = NULL;                     /* initialization for a window */
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
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

    width = 300;
    height = 300;

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

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel (display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);

    XFlush (display);

    XColor color;
    color.red=10000; //range from 0~65535
    color.green=10000;
    color.blue=65000;

    gc = XCreateGC (display, win, valuemask, &values);
    Status rc1=XAllocColor(display,DefaultColormap(display, screen),&color); //allocate the color using the color map.
    //set the color and attribute of the graphics content
    XSetForeground (display, gc, color.pixel);
    XSetBackground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);
//    printArray(result_arr,X*Y);
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            if (result_arr[i * X + j] == 1) {
                XDrawPoint(display, win, gc, j, i);
                usleep(1);
            }
        }
    }
    XFlush(display);
    sleep(1);
    pthread_exit(NULL);
    return 0;
}
