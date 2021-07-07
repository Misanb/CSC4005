#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
#include <sys/time.h>
using namespace std;



int X, Y, NUM_THREAD, local_row_num;
int * result_arr;

int MAG_THRE = 2;
int ITER_THRE = 100;
void printArray(int arr[], int arr_size);
int cal_M_set(int idx);
void printArray(int arr[], int arr_size){
    for(int i = 0; i < arr_size; i++){
        cout<< arr[i] << " ";
    }
    cout<<endl;
    
}
//todo turn row number to index


int cal_M_set(int idx){
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
    return(k == ITER_THRE);
}



int main (int argc, char* argv[]){
    X = Y = atoi(argv[1]);
    result_arr = (int *) malloc(sizeof(int) * X * Y);
    struct timeval timeStart, timeEnd, timeSystemStart;
    double runTime=0, systemRunTime;
    gettimeofday(&timeStart, NULL );
    for(int i = 0; i<X*X; i++){
        result_arr[i] = cal_M_set(i);
    }
    
    gettimeofday( &timeEnd, NULL );
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    printf("Name: Yiru Mao\n");
    printf("Student ID: 118010223\n");
    printf("Assignment 2, Mandelbrot Set, sequential implementation.\n");
    cout<< "runTime is "<<runTime<<"s"<<endl;
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

    return 0;
}
