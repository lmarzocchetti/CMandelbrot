#include <SFML/Graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

#define NUM_THREADS 12 // 15
#define THREAD_FACTOR (SCREEN_WIDTH / NUM_THREADS)

typedef struct {
    double position;
    sfColor color;
} colorPos;

double xPos = (double) -0.5;
double yPos = (double) -0.5;

double zoom = (double) 0.0037;

sfImage* image;
int maxiter;
double inc_pos;
sfText* text;
char result[50];

colorPos* init_palette() {
    colorPos* ret = (colorPos*) malloc(5*sizeof(colorPos));

    colorPos c1 = {0.0, sfColor_fromRGB(0, 7, 100)};
    colorPos c2 = {0.16, sfColor_fromRGB(32, 107, 203)};
    colorPos c3 = {0.42, sfColor_fromRGB(237, 255, 255)};
    colorPos c4 = {0.6425, sfColor_fromRGB(255, 170, 0)};
    colorPos c5 = {0.8575, sfColor_fromRGB(0, 2, 0)};

    ret[0] = c1;
    ret[1] = c2;
    ret[2] = c3;
    ret[3] = c4;
    ret[4] = c5;

    return ret;
}

int iterate(double re_c, double im_c, int limit) {
    int depth = 0;
    double re_z = 0;
    double im_z = 0;
    double re_z_sqr = 0;
    double im_z_sqr = 0;

    while(re_z_sqr + im_z_sqr < 4 && ++depth <= limit) {
        re_z_sqr = re_z * re_z;
        im_z_sqr = im_z * im_z;

        im_z = 2 * re_z * im_z + im_c;    // perform iteration
        re_z = re_z_sqr - im_z_sqr + re_c;
    }
    return depth;
}

int iterate_julia(double re_c, double im_c, int limit, double R) {
    int depth = 0;
    double re_z = 0;
    double im_z = 0;
    double re_z_sqr = 0;
    double im_z_sqr = 0;

    while(re_z_sqr + im_z_sqr < R*R && depth++ < limit) {
        re_z_sqr = re_z * re_z;
        im_z_sqr = im_z * im_z;

        im_z = 2 * re_z * im_z + im_c;    // perform iteration
        re_z = re_z_sqr - im_z_sqr + re_c;
    }
    if(depth != limit) {
        return depth;
    }
    else {
        return 1;
    }

}

void* calculate_mandelbrot_thread(void *threadid) {
    int x, y;

    long id = (long) threadid;

    for(x = (int) id * THREAD_FACTOR; x < id * THREAD_FACTOR + THREAD_FACTOR; ++x) {
        for(y = 0; y < SCREEN_HEIGHT; ++y) {
            //double re_c = xPos/SCREEN_WIDTH + (x - (SCREEN_WIDTH >> 1)) / zoom;
            //double im_c = yPos/SCREEN_HEIGHT + (y - (SCREEN_HEIGHT >> 1)) / zoom;
            double re_c = xPos + (x - (SCREEN_WIDTH >> 1)) * zoom;
            double im_c = yPos + (y - (SCREEN_HEIGHT >> 1)) * zoom;
            //int depth = iterate(re_c , im_c , maxiter);
            int depth = iterate_julia(re_c , im_c , maxiter,  2.5);
            sfImage_setPixel(image, x, y, sfColor_fromRGB(depth % 256, (depth * 3) % 256, (depth * 7 + 39) % 256));
        }
    }
    return NULL;
}

void repaint(sfRenderWindow* window, sfSprite* spr, sfTexture* txt) {
    sfRenderWindow_clear(window, sfBlack);

    sfTexture_updateFromImage(txt, image, 0, 0);
    sfSprite_setTexture(spr,txt, false);
    sfRenderWindow_drawSprite(window, spr, NULL);

    memset(result, 0, 50);
    sprintf(result, "%lf", inc_pos);
    sfText_setString(text, result);
    sfRenderWindow_drawText(window, text, NULL);

    sfRenderWindow_display(window);
}

void init_threads_and_calculate(pthread_t * thrds) {
    int rc;

    for(long i = 0; i < NUM_THREADS; i++) {
        rc = pthread_create(&thrds[i], NULL, calculate_mandelbrot_thread, (void *)i);
        if (rc) {
            printf("ERORR; return code from pthread_create() is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    int ret;
    for (int i = 0; i < NUM_THREADS; i++) {
        void *retval;
        ret = pthread_join(thrds[i], &retval);
        if (retval == PTHREAD_CANCELED)
            printf("The thread was canceled - ");
        /*else
            printf("Returned value %ld - ", (long)retval);*/

        switch (ret) {
            case 0:
                //printf("The thread joined successfully\n");
                break;
            case EDEADLK:
                printf("Deadlock detected\n");
                break;
            case EINVAL:
                printf("The thread is not joinable\n");
                break;
            case ESRCH:
                printf("No thread with given ID is found\n");
                break;
            default:
                printf("Error occurred when joining the thread\n");
        }
    }
    //pthread_exit(NULL);

}

int main(int argc, char* argv[]) {
    sfVideoMode mode = {SCREEN_WIDTH, SCREEN_HEIGHT, 32};
    sfRenderWindow* window;
    sfSprite* sprite = sfSprite_create();
    sfTexture* texture = sfTexture_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    //colorPos* palette;
    sfEvent event;
    pthread_t threads[NUM_THREADS];
    inc_pos = 30.0;

    if(argc != 2) {
        printf("Usage:   %s <maxiter>\n", argv[0]);
        printf("Example: %s 1000\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    maxiter = atoi(argv[1]);

    // Initialize the render window
    window = sfRenderWindow_create(mode, "Mandelbrot set", sfClose, NULL);
    sfRenderWindow_setFramerateLimit(window, 30);

    image = sfImage_create(SCREEN_WIDTH, SCREEN_HEIGHT);

    //palette = init_palette();

    init_threads_and_calculate(threads);

    text = sfText_create();
    sfVector2f tp = {20, 20};
    sfText_setPosition(text, tp);
    sfText_setCharacterSize(text, 25);
    sfText_setColor(text, sfWhite);
    sfText_setFont(text, sfFont_createFromFile("/home/rhohen/CLionProjects/mandelbrot/arial.ttf"));

    //calculate_mandelbrot(image, maxiter);

    while(sfRenderWindow_isOpen(window)) {
        // EVENTS
        while (sfRenderWindow_pollEvent(window, &event)) {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close(window);
            }
            if (event.type == sfEvtKeyReleased) {
                if(event.key.code == sfKeySpace) {
                    //zoom += 1100 * increment;
                    //increment *= 3;
                    zoom *= 0.8;
                    init_threads_and_calculate(threads);
                    //calculate_mandelbrot(image, maxiter);
                }
                else if(event.key.code == sfKeyLeft) {
                    //xPos -= inc_pos;
                    xPos -= 10 * zoom;
                    init_threads_and_calculate(threads);
                    //calculate_mandelbrot(image, maxiter);
                }
                else if(event.key.code == sfKeyRight) {
                    //xPos += inc_pos;
                    xPos += 10 * zoom;
                    init_threads_and_calculate(threads);
                    //calculate_mandelbrot(image, maxiter);
                }
                else if(event.key.code == sfKeyUp) {
                    //yPos -= inc_pos;
                    yPos -= 10 * zoom;
                    init_threads_and_calculate(threads);
                    //calculate_mandelbrot(image, maxiter);
                }
                else if(event.key.code == sfKeyDown) {
                    // += inc_pos;
                    yPos += 10 * zoom;
                    init_threads_and_calculate(threads);
                    //calculate_mandelbrot(image, maxiter);
                }
                else if(event.key.code == sfKeyJ) {
                    if(inc_pos > 1) {
                        inc_pos -= 1;
                    }
                    else if(inc_pos <= 1 && inc_pos > 0) {
                        inc_pos /= 10;
                    }
                    else {
                        inc_pos = 30;
                    }
                }
                else if(event.key.code == sfKeyK) {
                    if(inc_pos < 30 && inc_pos > 1) {
                        inc_pos += 1;
                    }
                    else if(inc_pos <= 1 && inc_pos > 0) {
                        inc_pos *= 10;
                    }
                    else {
                        inc_pos = 30;
                    }
                }
            }
            /*
            else if (event.type == sfEvtMouseButtonReleased) {

                }
            }*/
        }
        // UPDATES
        repaint(window, sprite, texture);
    }

    // DESTROY
    sfImage_destroy(image);
    sfTexture_destroy(texture);
    sfSprite_destroy(sprite);
    //window
    sfRenderWindow_destroy(window);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
