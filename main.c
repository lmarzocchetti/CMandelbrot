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

#define NUM_THREADS 12
#define THREAD_FACTOR (SCREEN_WIDTH / NUM_THREADS)

//// GLOBAL VARIABLES FOR THREADS  ////
double xPos = (double) -0.5;
double yPos = (double) -0.5;
double zoom = (double) 0.0037;
sfImage* image;
int maxiter;
//////////////////////////////////////

/**
 * Iterate function for mandelbrot set
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
            sfImage_setPixel(image, x, y, sfColor_fromRGB(depth % 256, (depth * 3) % 256, (depth * 7 + 39) % 256));
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

    /// Take the argument passed by command line
    maxiter = (int) strtol(argv[1], NULL, 10);

    /// Initialize the render window and the image
    window = sfRenderWindow_create(mode, "Mandelbrot set", sfClose, NULL);
    sfRenderWindow_setFramerateLimit(window, 30);
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
                    zoom *= 0.8;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyLeft) {
                    xPos -= 10 * zoom;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyRight) {
                    xPos += 10 * zoom;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyUp) {
                    yPos -= 10 * zoom;
                    init_threads_and_calculate(threads);
                }
                else if(event.key.code == sfKeyDown) {
                    yPos += 10 * zoom;
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
    return EXIT_SUCCESS;
}
