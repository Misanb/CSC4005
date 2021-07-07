#include <iostream>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <queue>
#include <sys/time.h>
using namespace std;
int MAG_THRE = 2;
int ITER_THRE = 100;
void printArray(int arr[], int arr_size){
    for(int i = 0; i < arr_size; ++i){
        cout<< arr[i] << " ";
    }
    cout<<endl;
}
void cal_M_set(int X,int start_index,int sub_devide,int send_array[]){
    for(int idx = start_index;idx<start_index+sub_devide;idx++){
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
            send_array[idx-start_index+2] = 1;
        }else{
            send_array[idx-start_index+2] = 0;
        }
    }
}
int main(int argc,  char * argv[]){
    MPI_Comm comm;
    int my_rank;
    int comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    comm = MPI_COMM_WORLD;
//    printf("my rank%d\n",my_rank);

    int X,Y;
    X = Y = atoi(argv[1]);
    int array_size = X*X;
    int num_task = 100;
    int sub_devide = array_size/num_task;
    if(array_size%num_task != 0){
        sub_devide+=1;
    }
    array_size = num_task*sub_devide;
    int tag=0;
    if (my_rank == 0){
        int *array = new int[array_size];
        int *recv_array = new int[sub_devide+2];
        int *result_array = new int[array_size];
        
        queue<int> avail_core;
        for(int p=1; p<comm_size;p++){
            avail_core.push(p);
        }
        
        int current_task = 0;
        int next_core;
        int start_index;
        int recv_rank;
        int recv_start_index;
        int finished_task = 0;
        struct timeval timeStart, timeEnd, timeSystemStart;
        double runTime=0, systemRunTime;
        gettimeofday(&timeStart, NULL );
//        clock_t time_begin = clock();
        while(finished_task<num_task){
            while(avail_core.empty()==0 && current_task<num_task){
                next_core = avail_core.front();
                avail_core.pop();
                start_index = sub_devide*current_task;
//                printf("%d",start_index);
                //todo 考虑无法整除法的情况，可以吧resultarray放大至能够整除的程度，最后再截。
                MPI_Send(&start_index,1,MPI_INT,next_core,tag,MPI_COMM_WORLD);
                //给子进程send这个subtask的起始index，已知sub_divide 的情况下，子进程可以开始计算，
                current_task ++;
            }
            MPI_Status status;
            MPI_Recv(recv_array,sub_devide+2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&status);
            recv_rank = recv_array[0];
            recv_start_index = recv_array[1];
            for(int index=0; index<sub_devide;index++){
                result_array[index+recv_start_index] = recv_array[index+2];
            }
            avail_core.push(recv_rank);
            finished_task++;
        }
        gettimeofday( &timeEnd, NULL );
        runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
//        clock_t time_end = clock();
        printf("Name: Yiru Mao\n");
        printf("Student ID: 118010223\n");
        printf("Assignment 2, Mandelbrot Set, MPI dynamic implementation.\n");
//        double execution_time = (double) (time_end - time_begin) / CLOCKS_PER_SEC * 1000;
        cout<< "runTime is "<<runTime<<"s"<<endl;
        int stop_signal = -1;
        for(int p = 1;p<comm_size;p++){
            MPI_Send(&stop_signal,1,MPI_INT,p,tag,MPI_COMM_WORLD);
        }

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
                if (result_array[i * X + j] == 1) {
                    XDrawPoint(display, win, gc, j, i);
                    usleep(1);
                }
            }
        }
        XFlush(display);
        sleep(1);
    }
    
    if(my_rank != 0){
        int start_index;
        int *send_array=new int[sub_devide+2];
        MPI_Status status;
        while(1){
            MPI_Recv(&start_index,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
            if (start_index==-1){
              break;
            }
//            printArray(send_array,sub_devide+2);
            cal_M_set(X,start_index,sub_devide,send_array);
            send_array[0]=my_rank;
            send_array[1]=start_index;
            MPI_Send(send_array,sub_devide+2,MPI_INT,0,tag,MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    
    
    return 0;
}
