#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#define MAX 16
#define ALPHASIZE 26
void *status;
//char alphabet[ALPHASIZE] = {'a','b','c','d','e','f','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
char alphabet2[ALPHASIZE] = "ABCDEFGHIJKLMNOPQRSTUWXYZ";
char *p_alpha = alphabet2;
char zeichen;
pthread_mutex_t rb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full_condvar = PTHREAD_COND_INITIALIZER;
//...
int thread_id[4] = {0,1,2,3};
typedef struct {
char buffer[MAX];
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
    int i;
    pthread_t threads[4];
    printf("Start des Beispiels \n");
    //printf("Argumente verfuegbar: ARGC\n", 3*argc);
    p_rb -> p_in = p_start;
    p_rb -> p_out = p_start;
    p_rb -> count = 0;
    printf("Counter value %d\n", p_rb ->count);

    //pthread_mutex_lock(&rb_mutex);

    pthread_create(&threads[0], NULL, p_1_w, (void *)&thread_id[0]);
    pthread_create(&threads[1], NULL, p_2_w, (void *)&thread_id[1]);
    pthread_create(&threads[2], NULL, control, (void *)&thread_id[2]);
    //pthread_create(&threads[3], NULL, consumer, (void *) &thread_id[3]);
    //…
    // Hier bis i<4!!!!
    for(i = 0; i<3; i++)
        pthread_join(threads[i], NULL);
    printf("Ende nach Join der Threads\n");
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
    char eingabe;
    menu();
    eingabe = getchar(); //scanf("%c", eingabe);

    switch(eingabe) {
        case '1':
        printf("producer_1 gejoint!\n");
        pthread_join(thread_id[0], &status);
        break;
        case '2': pthread_join(thread_id[1], &status); break;
        case 'c': case 'C': pthread_join(thread_id[2], &status); break;
        case 'q': case 'Q':
        pthread_cancel(thread_id[0]); // nicht sicher!
        pthread_cancel(thread_id[1]);
        pthread_cancel(thread_id[2]);
        break;
        case 'h': case 'H': menu(); break;
        default:
        printf("Kein Eingabe wurde festgestellt!");
        break;
    }
}

void* p_1_w(void *pid)
{
    printf("im Producer_1 Thread\n");

    while(p_rb->count < MAX){
        printf("%d in der SChleife\n", indiz);
        zeichen = (*p_alpha);
        *p_rb ->p_in = zeichen;
        p_alpha++;
        printf("Zeichen %c wurde im Ring Puffer geschrieben!\n", zeichen);
        //p_rb -> p_in++;
        p_rb -> count++;
        sleep(3);
    }
    //block hier
    pthread_mutex_lock(&rb_mutex);
}

void* p_2_w(void *pid)
{
    printf("im Producer_2 Thread\n");

    while(p_rb->count < MAX){
        printf("%d in der SChleife\n", indiz);
        zeichen = (toupper(*p_alpha));
        *p_rb ->p_in = zeichen;
        p_alpha++;
        printf("Zeichen %c wurde im Ring Puffer geschrieben!\n", zeichen);
        //p_rb -> p_in++;
        p_rb -> count++;
        sleep(3);
    }
    //block hier
    pthread_mutex_lock(&rb_mutex);
}


//void* p_2_w(void *pid){}

//void* consumer(void *pid){}
