#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#define MAX 16
#define MAXZEICHEN 30
#define ALPHASIZE 26
#define DEFAULT 0
void *status;
char alphabet[ALPHASIZE] = "abcdefghijklmnopqrstuvwxyz";
char *p_alpha = alphabet;
int active=1;
int i;
int rc1, rc2, rc3, rc4;
pthread_t threads[4];
#define p_alpha_start (char *)alphabet
#define p_alpha_end (char *)(alphabet + ALPHASIZE-1)
char zeichen;
pthread_mutex_t rb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full_condvar = PTHREAD_COND_INITIALIZER;
int thread_id[4] = {0,1,2,3};
typedef struct {
int buffer[MAX];
int *p_in;
int *p_out;
int count;
}rb;

rb x = { {0}, NULL, NULL, 0};
rb *p_rb = &x;
#define p_start (int *)(p_rb -> buffer)
#define p_end (int *)((p_rb -> buffer) + MAX-1)
void* p_1_w(void *pid);
void* p_2_w(void *pid);
void* consumer(void *pid);
void* control(void *pid);

int main(int argc, char* argv[])
{
    //pthread_t threads[4];
    printf("Start des Beispiels \n");
    //printf("Argumente verfuegbar: ARGC\n", 3*argc);
    p_rb -> p_in = p_start;
    p_rb -> p_out = p_start;
    p_rb -> count = 0;
    printf("Counter value %d\n", p_rb ->count);

    pthread_attr_t my_thread_attr; // Thread Attribut
    struct sched_param my_prio;
    pthread_attr_init(&my_thread_attr);
    pthread_attr_setinheritsched(&my_thread_attr, PTHREAD_EXPLICIT_SCHED); // Freigabe der Parameteränd.
    pthread_attr_setschedpolicy(&my_thread_attr, SCHED_FIFO);
    my_prio.sched_priority = 10; // Priority ändern
    pthread_attr_setschedparam(&my_thread_attr, &my_prio);

    rc3 = pthread_create(&threads[2], NULL, control, (void *)&thread_id[2]);
    rc1 = pthread_create(&threads[0], NULL, p_1_w, (void *)thread_id);
    rc2 = pthread_create(&threads[1], NULL, p_2_w, (void *)&thread_id[1]);
    rc4 = pthread_create(&threads[3], NULL, consumer, (void *)&thread_id[3]);
    for(i = 0; i<4; i++)
        pthread_join(threads[i], NULL);
    printf("Ende nach Join der Threads\n");
    printf("Control Thread returns: %d\n",rc3);
    printf("Producer_1 Thread returns: %d\n",rc1);
    printf("Producer_2 Thread returns: %d\n",rc2);
    printf("Consumer Thread returns: %d\n",rc4);
    return 0;
}

void menu()
{
    printf("***************Menu*************\n");
    printf("TE 1: Start / Stopp Producer_1\n");
    printf("TE 2: Start / Stopp Producer_2\n");
    printf("TE c oder C: Start / Stop Consumer\n");
    printf("TE q oder Q: Terminiert die Threads, sodass der Main_Thread das System beendet.\n");
    printf("TE h: Menu\n");
}

void* control(void *pid)
{
    while (1) {
    char eingabe;
    int rc;
    menu();
    eingabe = getchar();

    switch(eingabe) {
        case '1':
        printf("producer_1 gejoint!\n");
        if (active) {
            pthread_detach(thread_id[0]);
            active=0;
        }
        else {
            pthread_join(thread_id[0], &status);
            active=1;
        }
        break;
        case '2':
        if (active) {
            pthread_detach(thread_id[1]);
            active=0;
        }
        else {
            pthread_join(thread_id[1], &status);
            active=1;
        }
        printf("producer_2 gejoint!\n");
        pthread_join(thread_id[1], &status);
        break;
        case 'c': case 'C':
        if (active) {
            pthread_detach(thread_id[3]);
            active=0;
        }
        else {
            pthread_join(thread_id[3], &status);
            active=1;
        }
        case 'q': case 'Q':
        pthread_cancel(threads[0]);
        pthread_cancel(threads[1]);
        pthread_cancel(threads[2]);
        pthread_cancel(threads[3]);
        break;
        case 'h': case 'H': menu(); break;
        default:
        printf("Kein Eingabe wurde festgestellt!");
        break;
    }
    }
}

void* p_1_w(void *pid)
{
    printf("im Producer_1 Thread\n");
    while (1)
    {
        if(p_rb->count < MAX){
            sleep(3);
            pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
            zeichen = (*p_alpha);
            *p_rb ->p_in = zeichen;
            if (p_rb -> p_in == p_end)
                p_rb -> p_in = p_start;
            else
                p_rb ->p_in++;
            printf("%c wurde im Ring Puffer geschrieben!\n", zeichen);
            i = 0;
            while (i < MAX - 1) {
                printf("%c", p_rb -> buffer[i]);
                i++;
            }
            printf("\n");
            p_rb -> count++;
            if (p_alpha == p_alpha_end)
                p_alpha = p_alpha_start;
            else
                p_alpha++;
            pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        }
        else pthread_cond_signal(&not_empty_condvar);
        //pthread_cond_wait(&not_empty_condvar, &rb_mutex);
    }
}

void* p_2_w(void *pid)
{
    printf("im Producer_2 Thread\n");
    while (1)
    {
        if(p_rb->count < MAX){
            sleep(3);
            pthread_mutex_lock(&rb_mutex); // Reservierung der Ressourcen für den aktuellen Thread
            zeichen = (toupper(*p_alpha));
            *p_rb ->p_in = zeichen;
            if (p_rb -> p_in == p_end) p_rb -> p_in = p_start;
            else p_rb ->p_in++;
            printf("%c : wurde im Ringpuffer geschrieben!\n", zeichen);
            i = DEFAULT;
            while (i < MAX - 1) {
                printf("%c", p_rb -> buffer[i]);
                i++;
            }
            printf("\n");
            p_rb -> count++;
            if (p_alpha == p_alpha_end)
                p_alpha = p_alpha_start;
            else
                p_alpha++;
            pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        }
        else pthread_cond_signal(&not_empty_condvar);
        //pthread_cond_wait(&not_empty_condvar, &rb_mutex);
    }
}
void* consumer(void *pid)
{
    printf("im Consumer Thread\n");
    while (1){
        if(p_rb->count > 0) {
            sleep(2);
            zeichen = *p_rb ->p_out;
            //if (sizeof(zeichen)< MAXZEICHEN)
                //printf("Zeichen %c : wurde aus dem Ringpuffer gelesen!", zeichen);
            //else
                printf("%c : wurde aus dem Ringpuffer gelesen!\n", zeichen);
            *p_rb ->p_out = DEFAULT;
            if (p_rb -> p_out == p_end)
                p_rb -> p_out = p_start;
            else
                p_rb ->p_out++;
            p_rb -> count--;
        }
        else pthread_cond_signal(&not_empty_condvar);
        //pthread_cond_wait(&not_empty_condvar, &rb_mutex);
    }
}
