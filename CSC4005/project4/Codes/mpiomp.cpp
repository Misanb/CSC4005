#include <omp.h>
#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
using namespace std;

int iter_num  = 5000;

int main (int argc,char *argv[]){
    
    int size = atoi(argv[1]);
    int thread_num = atoi(argv[2]);
    
    MPI_Comm comm;
    int my_rank;
    int comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
//
    int* local_sizes = new int[comm_size];
    int* offsets = new int[comm_size];
    for (int i = 0; i < comm_size; ++i){
        if (i<= (size % comm_size -1)){
            local_sizes[i] = size / comm_size + 1;
        }else{
            local_sizes[i] = size / comm_size;
        }
    }
    int* local_sizes_ = new int[comm_size];
    for (int i = 0; i < comm_size; ++i){
        local_sizes_[i] = local_sizes[i]*size;
    }
    offsets[0] = 0;
    for (int i = 0; i < comm_size-1; ++i){
        offsets[i+1] = offsets[i] + local_sizes[i]*size;
    }

    double * local_arr = (double *) malloc (size * local_sizes[my_rank]*sizeof(double));
    double * local_tmp = (double *) malloc (size * local_sizes[my_rank]*sizeof(double));
    double * up_row = (double *) malloc (size * sizeof(double));
    double * down_row = (double *) malloc (size * sizeof(double));
    double * global_array = (double *) malloc (size*size*sizeof(double));
    //initialize
    for(int i = 0; i< local_sizes[my_rank];i++){
        for(int j = 0; j<size;j++){
            local_arr[i*size+j] = 20;
        }
    }
    
    Window          win;
    Display         *display;
    GC              gc;   //this is a graphic content, it could be a pixel color
    GC*             gcs;
    struct timeval timeStart, timeEnd, timeSystemStart;
    int runTime = 0;
    //fireplace
    if(my_rank==0){
        //fireplace
        
        for (int i = -size/12; i < size/12; i++ ){
            local_arr[size/2+i] = 100;
        }
    //    prepare the background
    //    Draw the graphic, copy from exanple codes
        
        char            window_name[] = "test", *display_name = NULL;                     /* initialization for a window */
        
        
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
        
        double systemRunTime;
        gettimeofday(&timeStart, NULL );
    }
    clock_t time_begin = clock();
    omp_set_num_threads(thread_num);
    
    for(int i = 0; i<iter_num;i++){
        //transfer
        
        if(my_rank > 0){
            MPI_Send(&local_arr[0], size, MPI_DOUBLE, my_rank-1,0,MPI_COMM_WORLD);
        }
        if(my_rank < comm_size-1){
            MPI_Recv(&down_row[0], size, MPI_DOUBLE,my_rank+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Send(&local_arr[(local_sizes[my_rank])*size-size], size, MPI_DOUBLE, my_rank+1,0,MPI_COMM_WORLD);
        }
        if(my_rank > 0){
            MPI_Recv(&up_row[0], size, MPI_DOUBLE, my_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        
        //calculate
        #pragma omp parallel for
        for (int j = size; j <size*local_sizes[my_rank]-size;j++){
            if((j%size) != 0 && ((j+1)%size) !=0){
                local_tmp[j] = 0.25*(local_arr[j-1]+local_arr[j+1]+local_arr[j-size]+local_arr[j+size]);
            }
        }

        if(my_rank != 0){
            #pragma omp parallel for
            for (int j = 0; j < size;j++){
                if((j%size) != 0 && ((j+1)%size) !=0){
                    local_tmp[j] = 0.25*(local_arr[j-1]+local_arr[j+1]+up_row[j]+local_arr[j+size]);
                }
            }
        }

        if(my_rank != comm_size-1){
            #pragma omp parallel for
            for (int j = size*local_sizes[my_rank]-size; j < size*local_sizes[my_rank];j++){
                if((j%size) != 0 && ((j+1)%size) !=0){
                    local_tmp[j] = 0.25*(local_arr[j-1] + local_arr[j+1] + local_arr[j-size] + down_row[j-size*local_sizes[my_rank]+size]);
                }
            }
        }
        
        
        //update
        #pragma omp parallel for
        for (int j = size; j <size*local_sizes[my_rank]-size;j++){
            if((j%size) != 0 && ((j+1)%size) !=0){
                local_arr[j] = local_tmp[j];
            }
        }
        
        
        
        if(my_rank != 0){
            #pragma omp parallel for
            for (int j = 0; j < size;j++){
                if((j%size) != 0 && ((j+1)%size) !=0){
                    local_arr[j] = local_tmp[j];
                }
            }
        }
        
        
        
        if(my_rank != comm_size-1){
            #pragma omp parallel for
            for (int j = size*local_sizes[my_rank]-size; j < size*local_sizes[my_rank];j++){
                if((j%size) != 0 && ((j+1)%size) !=0){
                    local_arr[j] = local_tmp[j];
                }
            }
        }
        
        
        
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(local_arr, local_sizes[my_rank]*size, MPI_DOUBLE,global_array, local_sizes_, offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        if(my_rank == 0){
            //display
//            printf("global,%f\n",global_array[300]);
            if(i%5 == 0){
                int level;
                for(int j = 0; j< size*size; j++){
                    level = (global_array[j]-20)/5;
                    XDrawPoint (display, win, gcs[level], j%(size)*200/size, floor(j/(size))*200/size);
                }
                XFlush (display);
            }
            
        }
    }
    clock_t time_end = clock();
    
    if(my_rank==0){
        gettimeofday( &timeEnd, NULL );
        runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
        printf("Name: Yiru Mao\n");
        printf("Student ID: 118010223\n");
        printf("Assignment 4, Heat Distribution,MPI + OpenMP implementation.\n");
        cout<< "runTime is "<<runTime<<"s"<<endl;
    }
    MPI_Finalize();
    return 0;
}
