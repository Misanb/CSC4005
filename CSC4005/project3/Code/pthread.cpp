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

const int body_num = 200;
const double G = 0.667; //G=6.672x10^-11
const int max_mass = 500;
const int min_mass = 250;
const int Upper_bound = 150;
const int Lower_bound = 50;
const int Left_bound = 50;
const int Right_bound = 150;

int iter_num  = 500;
double time_interval = 0.001;
int current_idx = -1;
int NUM_THREAD;
pthread_barrier_t b;
int local_size,offset;

struct body{
    double vx, vy, x, y, ay, ax, m;
};
struct body Bodys[body_num];
void generateNbodys();
void *updateAcceleration(void *t);
void updateInfo();
void generateNbodys(){
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
}

void *updateAcceleration(void *t){
    int *my_id = (int *)t;
    
    double r_sqr;
    bool collide;
    int sub_size = local_size;
    if(*my_id == NUM_THREAD-1) sub_size += offset;
    for(int current_number = local_size*(*my_id); current_number<local_size*(*my_id)+sub_size; current_number++){
//        printf("In thread %d, the current body is %d\n",*my_id,current_body);
        collide = false;
        Bodys[current_number].ax = 0;
        Bodys[current_number].ay = 0;
        for(int j = 0; j<body_num;j++){
            if(current_number!=j){
                r_sqr = pow((Bodys[current_number].x-Bodys[j].x),2)+pow((Bodys[current_number].y-Bodys[j].y),2);
                if(r_sqr == 0){
                    //collision
                    collide = true;
                    Bodys[current_number].vx *= -1;
                    Bodys[current_number].vy *= -1;

                    if(Bodys[current_number].vx*Bodys[j].vx<0){
                        Bodys[current_number].vx *= -1;
                    }
                    if(Bodys[current_number].vy*Bodys[j].vy<0){
                        Bodys[current_number].vy *= -1;
                    }
                }
                else{
                    Bodys[current_number].ax += (Bodys[j].x-Bodys[current_number].x)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                    Bodys[current_number].ay += (Bodys[j].y-Bodys[current_number].y)/pow(r_sqr,1/2)*G*Bodys[j].m/r_sqr;
                }
            }
        }
        if(collide == true){
            Bodys[current_number].vx *= 0.9;
            Bodys[current_number].vy *= 0.9;
        }
        
    }
    
    pthread_barrier_wait(&b);
    for(int i = local_size*(*my_id); i<local_size*(*my_id)+sub_size; i++){
        Bodys[i].x = Bodys[i].x +Bodys[i].vx*time_interval + Bodys[i].ax*pow(time_interval,2)/2;
        Bodys[i].y = Bodys[i].y +Bodys[i].vy*time_interval + Bodys[i].ay*pow(time_interval,2)/2;
        Bodys[i].vx += Bodys[i].ax * time_interval;
        Bodys[i].vy += Bodys[i].ay * time_interval;
//        if(Bodys[i].x>=Right_bound+10 || Bodys[i].x<=Left_bound-10){
//
//            Bodys[i].vx *= -0.5;
////            printf("%f\n",Bodys[i].vx);
//        }
        
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
    
}

void updateInfo(){
    for(int i = 0; i<body_num;i++){
        Bodys[i].x = Bodys[i].x +Bodys[i].vx*time_interval + Bodys[i].ax*pow(time_interval,2)/2;
        Bodys[i].y = Bodys[i].y +Bodys[i].vy*time_interval + Bodys[i].ay*pow(time_interval,2)/2;
        Bodys[i].vx += Bodys[i].ax * time_interval;
        Bodys[i].vy += Bodys[i].ay * time_interval;
//        if(Bodys[i].x>=Right_bound+10 || Bodys[i].x<=Left_bound-10){
//
//            Bodys[i].vx *= -0.5;
////            printf("%f\n",Bodys[i].vx);
//        }
        
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

}

int main (int argc,char *argv[]){
    iter_num = atoi(argv[1]);
    NUM_THREAD = atoi(argv[2]);
    pthread_t threads[NUM_THREAD];
    int *thread_ids = (int *) malloc(sizeof(int) * NUM_THREAD);
    local_size = body_num/NUM_THREAD;
    offset = body_num % NUM_THREAD;
    
    generateNbodys();
//    prepare the background
//    Draw the graphic, copy from exanple codes
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
    
    
    pthread_barrier_init(&b,NULL,NUM_THREAD);
    struct timeval timeStart, timeEnd, timeSystemStart;
    double runTime=0, systemRunTime;
    gettimeofday(&timeStart, NULL );
    int rc;
    int i;
    for(int k =0; k<iter_num;k++){
        for(i = 0; i<NUM_THREAD; i++){
            thread_ids[i] = i;
            rc = pthread_create(&threads[i], NULL, updateAcceleration,(void*)&thread_ids[i]);
            if(rc){
                printf("ERROR: return code from pthread_create() is %d", rc);
                exit(1);
            }
        }
        for(i = 0; i<NUM_THREAD; i++){
            pthread_join(threads[i], NULL);
        }
        
//        updateInfo();
        
        XClearWindow(display, win);
        XSetForeground (display, gc, color.pixel);

//            printf("%.2f\t",Bodys[0].x);
        for(int i=0; i<body_num;i++){
//            printf("%.4f\t",Bodys[i].ax);
            
            
            XDrawPoint(display, win, gc, Bodys[i].y, Bodys[i].x);
//                usleep(1);
        }
//        printf("\n");
        XFlush(display);
    }
    pthread_barrier_destroy(&b);
    gettimeofday( &timeEnd, NULL );
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
    printf("Name: Yiru Mao\n");
    printf("Student ID: 118010223\n");
    printf("Assignment 3, Nbody, pthread implementation.\n");
    cout<< "runTime is "<<runTime<<"s"<<endl;
    pthread_exit(NULL);
    return 0;
}


