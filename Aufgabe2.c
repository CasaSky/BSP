#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#define MAX 16
#define MAXZEICHEN 30
#define ALPHASIZE 26
#define DEFAULT 0
#define CONTROLEXIT
const int p1terminated = 10;
const int p2terminated = 20;
const int cterminated = 30;
void *status; /* thread result */
int result[4];
int controllexit=0;
void* status;
char alphabet[ALPHASIZE] = "abcdefghijklmnopqrstuvwxyz";
char alphabet2[ALPHASIZE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *p_alpha = alphabet;
char *p_alpha2 = alphabet2;
int p1flag=1;
int p2flag=1;
int cflag=1;
int zaehler=0;
int i;
pthread_t threads[4];
#define p_alpha_start (char *)alphabet
#define p_alpha_end (char *)(alphabet + ALPHASIZE-1)
#define p_alpha_start2 (char *)alphabet2
#define p_alpha_end2 (char *)(alphabet2 + ALPHASIZE-1)
char zeichen;

pthread_mutex_t rb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t p1_unlock = PTHREAD_COND_INITIALIZER;
pthread_cond_t p2_unlock = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_unlock = PTHREAD_COND_INITIALIZER;

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

    // Threads erstellen
    pthread_create(&threads[2], NULL, control, (void *)&thread_id[2]);
    pthread_create(&threads[0], NULL, p_1_w, (void *)thread_id);
    pthread_create(&threads[1], NULL, p_2_w, (void *)&thread_id[1]);
    pthread_create(&threads[3], NULL, consumer, (void *)&thread_id[3]);

        // Controller join und status abfangen
        pthread_join(threads[2], &status);
        // Wenn erfolgreich beendet
        if (status == 0) {
            printf("Control join erfolgreich!\n");
            pthread_cond_signal(&p1_unlock); // signal für das Freigeben, wenn p1 gestoppt ist.
            pthread_cond_signal(&p2_unlock); // signal für das Freigeben, wenn p2 gestoppt ist.
            pthread_cond_signal(&c_unlock); // signal für das Freigeben, wenn c gestoppt ist.
            pthread_cancel(threads[0]); // beenden
            pthread_cancel(threads[1]); // beenden
            pthread_cancel(threads[3]); // beenden
        }

    //for(i = 0; i<4; i++) {
        result[0] = pthread_join(threads[0], NULL); // &status
        result[1] = pthread_join(threads[1], NULL); // &status
        result[3] = pthread_join(threads[3], NULL); // &status
    //printf("Exit status: %d\n", *(int *)status); // Speicherzugriffsfehler
        //result[i] = *(int *)status;
    //}

	// vosichthalber vor dem Join ein Signal für den jeweiligen Thread mit condtion schicken.
    printf("Ende nach Join der Threads\n");
    //printf("Control Thread returns: %d\n",result[2]);
    printf("Producer_1 Thread returns: %d\n",result[0]);
    printf("Producer_2 Thread returns: %d\n",result[1]);
    printf("Consumer Thread returns: %d\n",result[3]);
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

// Wenn Flag gesetzt ist, heißt es, der Thread läuft, wenn nicht, ist er gestoppt
//läuft solange bis q oder Q gedrückt wird -> return 0
void *control(void *pid)
{
    while (1) {
    char eingabe;
    //menu();
    eingabe = getchar();

    switch(eingabe) {
        case '1':
        pthread_mutex_lock(&rb_mutex); // Ressourcen Reservieren
        if (p1flag==1)  // Falls flag gesetzt ist -> stoppen
            p1flag=0;
        else {// starten
            p1flag=1;
            pthread_cond_signal(&p1_unlock); // Signal für p1_unlock, da es gestartet werden soll
        }
        pthread_mutex_unlock(&rb_mutex); // Ressourcen Freigeben
        break;

        case '2':
        pthread_mutex_lock(&rb_mutex); // Ressourcen Reservieren
        if (p2flag==1)  // Falls flag gesetzt ist -> stoppen
            p2flag=0;
        else {// starten
            p2flag=1;
            pthread_cond_signal(&p2_unlock); // Signal für p1_unlock, da es gestartet werden soll
        }
        pthread_mutex_unlock(&rb_mutex); // Ressourcen Freigeben
        break;

        case 'c': case 'C':
        pthread_mutex_lock(&rb_mutex); // Ressourcen Reservieren
        if (cflag==1)   // Falls flag gesetzt ist -> stoppen
            cflag=0;
        else {// starten
            cflag=1;
            pthread_cond_signal(&c_unlock); // Signal für p1_unlock, da es gestartet werden soll
        }
        pthread_mutex_unlock(&rb_mutex); // Ressourcen Freigeben
        break;
        case 'q': case 'Q':
        return  (void *)0;
        break;
        case 'h': case 'H': menu();
        break;
        default:
        //printf("Kein Eingabe wurde festgestellt!");
        //return (void *)1;
        break;
    }
    }
}

void* p_1_w(void *pid)
{
    while (1)
    {
        if (p1flag==0) { // Wenn flag nicht gesetzt ist, heißt der Thread soll stoppen -> wait for signal
                printf("p1 waiting...\n");
                pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
                pthread_cond_wait(&p1_unlock, &rb_mutex); // warten auf Signal mit p1_unlock -> wieder starten von p1
                pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        }
        pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
        if(p_rb->count < MAX){ // Wenn counter >= MAX ->  Ringpuffer ist voll -> es kann nichts mehr geschrieben werden, bis platz frei ist
            pthread_testcancel(); // sicheres Cancel
            zeichen = (*p_alpha); // nächstes Zeichen holen
            *p_rb ->p_in = zeichen; // Zeichen im nächsten verfübaren Platz im Puffer, schreiben
            if (p_rb -> p_in == p_end) // Wenn das Ende erreicht ist -> vom Anfang an starten
                p_rb -> p_in = p_start;
            else
                p_rb ->p_in++; // Zeigt auf das nächste verfügbaren Platz im Ringpuffer

            printf("Geschrieben %c \n", zeichen);
            p_rb -> count++; // Count um eins erhören
            if (p_alpha == p_alpha_end) // Fall das Alphabet durch ist -> vorne wieder anfangen
                p_alpha = p_alpha_start;
            else
                p_alpha++; // zeigt auf das nächste Zeichen im Alphabet
            //pthread_cond_signal(&not_empty_condvar); // Signal für consumer, das er starten werden kann, falls er gestoppt ist
        }
        pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        sleep(3); // um ein anderen Thread ran zu lassen
       // else pthread_cond_wait(&not_full_condvar, &rb_mutex);
    }
    return((void *) &p1terminated);
}

void* p_2_w(void *pid)
{
    while (1)
    {
        if (p2flag==0) { // Wenn flag nicht gesetzt ist, heißt der Thread soll stoppen -> wait for signal
                printf("p2 waiting...\n");
                pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
                pthread_cond_wait(&p2_unlock, &rb_mutex);  // warten auf Signal mit p1_unlock -> wieder starten von p2
                pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        }
        pthread_mutex_lock(&rb_mutex); // Reservierung der Ressourcen für den aktuellen Thread
        if(p_rb->count < MAX){  // Wenn counter >= MAX ->  Ringpuffer ist voll -> es kann nichts mehr geschrieben werden, bis platz frei ist
            pthread_testcancel(); // sicheres Cancel
            zeichen = (*p_alpha2); // nächstes Zeichen holen
            *p_rb ->p_in = zeichen; // Zeichen im nächsten verfübaren Platz im Puffer, schreiben
            if (p_rb -> p_in == p_end) // Wenn das Ende erreicht ist -> vom Anfang an starten
                p_rb -> p_in = p_start;
            else
                p_rb ->p_in++; // Zeigt auf das nächste verfügbaren Platz im Ringpuffer, was ein neues Zeichen darauf geschrieben kann

            printf("Geschrieben %c \n", zeichen);
            p_rb -> count++; // Count um eins erhören
            if (p_alpha2 == p_alpha_end2) // Fall das Alphabet durch ist -> vorne wieder anfangen
                p_alpha2 = p_alpha_start2;
            else
                p_alpha2++; // zeigt auf das nächste Zeichen im Alphabet
            //pthread_cond_signal(&not_empty_condvar); // Signal für consumer, das er starten werden kann, falls er gestoppt ist
        }
        pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        sleep(3); // um ein anderen Thread ran zu lassen
        //else pthread_cond_wait(&not_full_condvar, &rb_mutex);
    }
    return((void *) &p2terminated);
}
void* consumer(void *pid)
{
    while (1){
        if (cflag==0) { // Wenn flag nicht gesetzt ist, heißt der Thread soll stoppen -> wait for signal
                printf("consumer waiting...\n");
                pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
                pthread_cond_wait(&c_unlock, &rb_mutex);  // warten auf Signal mit p1_unlock -> wieder starten von consumer
                pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        }
        pthread_mutex_lock(&rb_mutex);  // Reservierung der Ressourcen für den aktuellen Thread
        if(p_rb->count > 0) { // Wenn counter <= 0 -> Ringpuffer ist leer -> es kann nichts mehr gelesen werden,
            pthread_testcancel(); // sicheres Cancel
            zeichen = *p_rb ->p_out; // nächstes Zeichen, das gelesen kann, holen
            if (zaehler < MAXZEICHEN) {  // Max erreicht
                printf("Gelesen %c \n", zeichen);
                zaehler++;
            }
            else {
                printf("Gelesen %c \n", zeichen);
                zaehler=0;
            }
            *p_rb ->p_out = DEFAULT; // löscht das Zeichen, was bereist gelesen wurde.
            if (p_rb -> p_out == p_end) // Wenn das Ende erreicht ist -> vom Anfang an starten
                p_rb -> p_out = p_start;
            else
                p_rb ->p_out++; // Zeigt auf das nächste Platz im Ringpuffer, was gelesen kann
            p_rb -> count--;
            //pthread_cond_signal(&not_full_condvar); // Signal für consumer, das er starten werden kann, falls er gestoppt ist
        }
        pthread_mutex_unlock(&rb_mutex); // Freigabe der Ressourcen
        sleep(2); // um ein anderen Thread ran zu lassen
        //else pthread_cond_wait(&not_empty_condvar, &rb_mutex);
    }
    return((void *) &cterminated);
}
