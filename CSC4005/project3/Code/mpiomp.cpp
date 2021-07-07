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

int const body_num = 200;
const double G = 0.667; //G=6.672x10^-11
const int max_mass = 500;
const int min_mass = 250;
const int Upper_bound = 150;
const int Lower_bound = 50;
const int Left_bound = 50;
const int Right_bound = 150;

int iter_num  = 500;
double time_interval = 0.001;

struct body{
    double vx, vy, x, y, ay, ax, m;
};


int main (int argc,char *argv[]){
    iter_num = atoi(argv[1]);
    int thread_num = atoi(argv[2]);
    MPI_Comm comm;
    int my_rank;
    int comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    comm = MPI_COMM_WORLD;
    int* local_sizes = new int[comm_size];
    int* offsets = new int[comm_size];
    for (int i = 0; i < comm_size; ++i){
        if (i<= (body_num % comm_size -1)){
            local_sizes[i] = body_num / comm_size + 1;
        }else{
            local_sizes[i] = body_num / comm_size;
        }
    }
    offsets[0] = 0;
    for (int i = 0; i < comm_size-1; ++i){
        offsets[i+1] = offsets[i] + local_sizes[i];
    }
    MPI_Datatype MPIBody;
    MPI_Type_contiguous(7, MPI_DOUBLE, &MPIBody);
    MPI_Type_commit(&MPIBody);
    
    struct body* Bodys = (struct body*)malloc(body_num * sizeof(struct body));
    struct body *local_bodys;
    local_bodys = (struct body*)malloc(local_sizes[my_rank] * sizeof(struct body));
    if(my_rank == 0){
        srand((unsigned) time(NULL));
        for(int i=0; i<body_num; i++){
            Bodys[i].vx = 0.0;
            Bodys[i].vy = 0.0;
            Bodys[i].ax = 0.0;
            Bodys[i].ay = 0.0;
            Bodys[i].x = rand()%(Right_bound-Left_bound)+Left_bound;
            Bodys[i].y = rand()%(Upper_bound-Lower_bound)+Lower_bound;
            Bodys[i].m = rand()%(max_mass-min_mass)+min_mass;
        }
        
        Window          win;
        char            window_name[] = "test", *display_name = NULL;                     /* initialization for a window */
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

        width = 200;
        height = 200;

        /* set window position */

        x = 0;
        y = 0;

        /* create opaque window */

        border_width = 4;
        win = XCreateSimpleWindow (display, RootWindow (display, screen),
                              x, y, width, height, border_width,
                              BlackPixel (display, screen), WhitePixel (display, screen)); //Change to WhitePixel (display, screen) if you want a white background

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
        
        struct timeval timeStart, timeEnd, timeSystemStart;
        double runTime=0, systemRunTime;
        gettimeofday(&timeStart, NULL );
        
        int offset = offsets[my_rank];
        
        for(int k =0; k<iter_num;k++){
            MPI_Bcast(Bodys, body_num, MPIBody, 0, MPI_COMM_WORLD);
            for(int i = 0; i<local_sizes[my_rank]; i++){
                local_bodys[i].vx = Bodys[offset+i].vx;
                local_bodys[i].vy = Bodys[offset+i].vy;
                local_bodys[i].x = Bodys[offset+i].x;
                local_bodys[i].y = Bodys[offset+i].y;
                local_bodys[i].ax = Bodys[offset+i].ax;
                local_bodys[i].ay = Bodys[offset+i].ay;
                local_bodys[i].m = Bodys[offset+i].m;
            }
            double r_sqr = 0.0;

            int j = 0;
            bool collide = false;
            #pragma omp parallel for private(j, r_sqr, collide)
            for(int i = offset; i<offset+local_sizes[my_rank]; i++){
                collide = false;
                Bodys[i].ax = 0;
                Bodys[i].ay = 0;
                for(j = 0; j<body_num;j++){
                    if(i!=j){
        //                r_sqr = (Bodys[i].x-Bodys[j].x)**2+(Bodys[i].y-Bodys[j].y)**2；
                        r_sqr = pow((Bodys[i].x-Bodys[j].x),2)+pow((Bodys[i].y-Bodys[j].y),2);
                        if(r_sqr == 0){
                            //collision
                            collide = true;
                            if(Bodys[i].vx*Bodys[j].vx<0){
                                Bodys[i].vx *= -1;
                            }
                            if(Bodys[i].vy*Bodys[j].vy<0){
                                Bodys[i].vy *= -1;
                            }
                        }
                        else{
                            Bodys[i].ax += (Bodys[j].x-Bodys[i].x)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                            Bodys[i].ay += (Bodys[j].y-Bodys[i].y)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                        }
                    }
                }
                if(collide == true){
                    Bodys[i].vx *= 0.9;
                    Bodys[i].vy *= 0.9;
                }
            }
            #pragma omp parallel for
            for(int i = offset; i<offset+local_sizes[my_rank]; i++){
                Bodys[i].x = Bodys[i].x +Bodys[i].vx*time_interval + Bodys[i].ax*pow(time_interval,2)/2;
                Bodys[i].y = Bodys[i].y +Bodys[i].vy*time_interval + Bodys[i].ay*pow(time_interval,2)/2;
                Bodys[i].vx += Bodys[i].ax * time_interval;
                Bodys[i].vy += Bodys[i].ay * time_interval;
                if(Bodys[i].x>Right_bound+10){
                    Bodys[i].x =Right_bound+10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].x<=Left_bound-10){
                    Bodys[i].x=Left_bound-10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].y>=Upper_bound+10){
                    Bodys[i].y =Upper_bound+10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].y<=Lower_bound-10){
                    Bodys[i].y =Lower_bound-10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }
            }
            for(int i = 0; i<local_sizes[my_rank]; i++){
                local_bodys[i].vx = Bodys[offset+i].vx;
                local_bodys[i].vy = Bodys[offset+i].vy;
                local_bodys[i].x = Bodys[offset+i].x;
                local_bodys[i].y = Bodys[offset+i].y;
                local_bodys[i].ax = Bodys[offset+i].ax;
                local_bodys[i].ay = Bodys[offset+i].ay;
                local_bodys[i].m = Bodys[offset+i].m;
            }
            MPI_Gatherv(local_bodys, local_sizes[my_rank], MPIBody, Bodys, local_sizes, offsets, MPIBody, 0, MPI_COMM_WORLD);

            XClearWindow(display, win);
            XSetForeground (display, gc, color.pixel);

            for(int i=0; i<body_num;i++){
    //            printf("%.2f\t",Bodys[i].ax);
    //            printf("%.2f\t",Bodys[i].ax);
                XDrawPoint(display, win, gc, Bodys[i].y, Bodys[i].x);
    //                usleep(1);
            }

    //        printf("\n");
            XFlush(display);
            
        }
        
        gettimeofday( &timeEnd, NULL );
        runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
        printf("Name: Yiru Mao\n");
        printf("Student ID: 118010223\n");
        printf("Assignment 3, Nbody, mpi + openmp implementation.\n");
        cout<< "runTime is "<<runTime<<"s"<<endl;
        
    }else{
        int offset = offsets[my_rank];

        for(int k =0; k<iter_num;k++){
            MPI_Bcast(Bodys, body_num, MPIBody, 0, MPI_COMM_WORLD);
//            printf("place0 %f\n",Bodys[130].vx);
            for(int i = 0; i<local_sizes[my_rank]; i++){
                local_bodys[i].vx = Bodys[offset+i].vx;
                local_bodys[i].vy = Bodys[offset+i].vy;
                local_bodys[i].x = Bodys[offset+i].x;
                local_bodys[i].y = Bodys[offset+i].y;
                local_bodys[i].ax = Bodys[offset+i].ax;
                local_bodys[i].ay = Bodys[offset+i].ay;
                local_bodys[i].m = Bodys[offset+i].m;
            }
            double r_sqr = 0.0;
            int j = 0;
            bool collide = false;
            #pragma omp parallel for private(j, r_sqr, collide)
            for(int i = offset; i<offset+local_sizes[my_rank]; i++){
                collide = false;
                Bodys[i].ax = 0;
                Bodys[i].ay = 0;
                for(j = 0; j<body_num;j++){
                    if(i!=j){
        //                r_sqr = (Bodys[i].x-Bodys[j].x)**2+(Bodys[i].y-Bodys[j].y)**2；
                        r_sqr = pow((Bodys[i].x-Bodys[j].x),2)+pow((Bodys[i].y-Bodys[j].y),2);
                        if(r_sqr == 0){
                            //collision
                            collide = true;
                            if(Bodys[i].vx*Bodys[j].vx<0){
                                Bodys[i].vx *= -1;
                            }
                            if(Bodys[i].vy*Bodys[j].vy<0){
                                Bodys[i].vy *= -1;
                            }
                        }
                        else{
                            Bodys[i].ax += (Bodys[j].x-Bodys[i].x)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                            Bodys[i].ay += (Bodys[j].y-Bodys[i].y)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                        }
                    }
                }
                if(collide == true){
                    Bodys[i].vx *= 0.9;
                    Bodys[i].vy *= 0.9;
                }
            }
            
            #pragma omp parallel for
            for(int i = offset; i<offset+local_sizes[my_rank]; i++){
                Bodys[i].x = Bodys[i].x +Bodys[i].vx*time_interval + Bodys[i].ax*pow(time_interval,2)/2;
                Bodys[i].y = Bodys[i].y +Bodys[i].vy*time_interval + Bodys[i].ay*pow(time_interval,2)/2;
                Bodys[i].vx += Bodys[i].ax * time_interval;
                Bodys[i].vy += Bodys[i].ay * time_interval;
                if(Bodys[i].x>Right_bound+10){
                    Bodys[i].x =Right_bound+10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].x<=Left_bound-10){
                    Bodys[i].x=Left_bound-10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].y>=Upper_bound+10){
                    Bodys[i].y =Upper_bound+10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }else if(Bodys[i].y<=Lower_bound-10){
                    Bodys[i].y =Lower_bound-10;
                    Bodys[i].vx *= -1;
                    Bodys[i].vy *= -1;
                }
            }
            for(int i = 0; i<local_sizes[my_rank]; i++){
                local_bodys[i].vx = Bodys[offset+i].vx;
                local_bodys[i].vy = Bodys[offset+i].vy;
                local_bodys[i].x = Bodys[offset+i].x;
                local_bodys[i].y = Bodys[offset+i].y;
                local_bodys[i].ax = Bodys[offset+i].ax;
                local_bodys[i].ay = Bodys[offset+i].ay;
                local_bodys[i].m = Bodys[offset+i].m;
            }
//            printf("place1 %d\n",offset);
//            printf("place1 %f\n",Bodys[130].vx);
            MPI_Gatherv(local_bodys, local_sizes[my_rank], MPIBody, Bodys, local_sizes, offsets, MPIBody, 0, MPI_COMM_WORLD);
        }
    }
    
    

//    Draw the graphic, copy from exanple codes
    delete[] local_sizes;
    delete[] offsets;
    MPI_Finalize();
    return 0;
}


