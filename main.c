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

#define NUM_THREADS 10
#define THREAD_FACTOR (SCREEN_WIDTH / NUM_THREADS)

//// GLOBAL VARIABLES FOR THREADS  ////
double xPos = (double) -0.5;
double yPos = (double) 0;
double zoom = (double) 0.0037;
sfImage* image;
int maxiter;
sfColor* palette;
//////////////////////////////////////

/**
 * Function which precompute all the 16 colors.
 *
 * @return A pointer to the array of sfColor (Palette)
 */
sfColor* set_color_palette() {
    sfColor* ret = (sfColor*) calloc(16, sizeof(sfColor));

    ret[0] = (sfColor) {66, 30, 15, 255};
    ret[1] = (sfColor) {25, 7, 26, 255};
    ret[2] = (sfColor){9, 1, 47, 255};
    ret[3] = (sfColor){4, 4, 73, 255};
    ret[4] = (sfColor){0, 7, 100, 255};
    ret[5] = (sfColor){12, 44, 138, 255};
    ret[6] = (sfColor){24, 82, 177, 255};
    ret[7] = (sfColor){57, 125, 209, 255};
    ret[8] = (sfColor){134, 181, 229, 255};
    ret[9] = (sfColor){211, 236, 248, 255};
    ret[10] = (sfColor){241, 233, 191, 255};
    ret[11] = (sfColor){248, 201, 95, 255};
    ret[12] = (sfColor){255, 170, 0, 255};
    ret[13] = (sfColor){204, 128, 0, 255};
    ret[14] = (sfColor){153, 87, 0, 255};
    ret[15] = (sfColor){106, 52, 3, 255};

    return ret;
}

/**
 * Iterate function for mandelbrot set
 *
 * @param re_c Define the real part of the mandelbrot function
 * @param im_c Define the imaginary part of the mandelbrot function
 * @param limit Maximum iteration
 * @return the number of iteration explored (depth)
 */
int iterate(double re_c, double im_c, int limit) {
    int depth = 0;
    double re_z = 0;
    double im_z = 0;
    double re_z_sqr = 0;
    double im_z_sqr = 0;

    while(re_z_sqr + im_z_sqr < 4 && ++depth <= limit) {
        re_z_sqr = re_z * re_z;
        im_z_sqr = im_z * im_z;

        im_z = 2 * re_z * im_z + im_c;
        re_z = re_z_sqr - im_z_sqr + re_c;
    }
    return depth;
}

/**
 * Calculate a part of the image of the mandelbrot set,
 * based of the number of threads and the SCREEN_WIDTH (THREAD_FACTOR) @see macros
 *
 * @param threadid the Id of the thread
 * @return is the return value of the thread (NULL)
 */
void* calculate_mandelbrot_thread(void *threadid) {
    int x, y;

    double re_c;
    double im_c;

    long id = (long) threadid;
    int depth;

    for(x = (int) id * THREAD_FACTOR; x < id * THREAD_FACTOR + THREAD_FACTOR; ++x) {
        for(y = 0; y < SCREEN_HEIGHT; ++y) {
            re_c = xPos + (x - (SCREEN_WIDTH >> 1)) * zoom;
            im_c = yPos + (y - (SCREEN_HEIGHT >> 1)) * zoom;
            depth = iterate(re_c , im_c , maxiter);

            if (depth < maxiter && depth > 0) {
                sfImage_setPixel(image, x, y, palette[depth % 16]);
            }
            else {
                sfImage_setPixel(image, x, y, sfBlack);
            }

        }
    }
    pthread_exit(NULL);
}

/**
 * Clear the screen, set the image on the sprite, and display the window
 *
 * @param window the window on the program paint the calculated mandelbrot photo
 * @param spr a sprite for printing the image (@see global variable: sfImage image)
 * @param txt texture for setting the sprite and print on the window
 */
void repaint(sfRenderWindow* window, sfSprite* spr, sfTexture* txt) {
    sfRenderWindow_clear(window, sfBlack);

    sfTexture_updateFromImage(txt, image, 0, 0);
    sfSprite_setTexture(spr,txt, false);
    sfRenderWindow_drawSprite(window, spr, NULL);

    sfRenderWindow_display(window);
}

/**
 * Init all the threads and start for the function calculate_mandelbrot_thread.
 * Next join all threads and control their return values
 *
 * @param thrds Array of threads based of macro NUM_THREADS
 */
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

        switch (ret) {
            case 0 :
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
}

/**
 * Do some stuff, see the function definition
 *
 * @param argc num of argument + 1
 * @param argv only 1 argument, that is the value of maximum iterations
 * @return
 */
int main(int argc, char* argv[]) {
    sfVideoMode mode = {SCREEN_WIDTH, SCREEN_HEIGHT, 32};
    sfRenderWindow* window;
    sfSprite* sprite = sfSprite_create();
    sfTexture* texture = sfTexture_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    sfEvent event;
    pthread_t threads[NUM_THREADS];

    /// Control if the arguments is different from 1
    if(argc != 2) {
        printf("Usage:   %s <maxiter>\n", argv[0]);
        printf("Example: %s 1000\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /// Initialize the color Palette
    palette = set_color_palette();

    /// Take the argument passed by command line
    maxiter = (int) strtol(argv[1], NULL, 10);

    /// Initialize the render window and the image
    window = sfRenderWindow_create(mode, "CMandelbrot", sfClose, NULL);
    sfRenderWindow_setFramerateLimit(window, 60);
    image = sfImage_create(SCREEN_WIDTH, SCREEN_HEIGHT);

    /// Calculate the first iteration
    init_threads_and_calculate(threads);

    /// Events
    while(sfRenderWindow_isOpen(window)) {
        /// EVENTS
        while (sfRenderWindow_pollEvent(window, &event)) {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close(window);
            }
            if (event.type == sfEvtKeyReleased) {
                if(event.key.code == sfKeySpace) {
                    zoom *= 0.7;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyLeft) {
                    xPos -= 10 * zoom * 1.3;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyRight) {
                    xPos += 10 * zoom * 1.3;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyUp) {
                    yPos -= 10 * zoom * 1.3;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyDown) {
                    yPos += 10 * zoom * 1.3;
                    init_threads_and_calculate(threads);
                }
            }
        }
        /// UPDATES
        repaint(window, sprite, texture);
    }

    /// DESTROY
    sfImage_destroy(image);
    sfTexture_destroy(texture);
    sfSprite_destroy(sprite);
    sfRenderWindow_destroy(window);
    free(palette);
    return EXIT_SUCCESS;
}
