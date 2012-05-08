/* clean_guppi_shmem.c
 *
 * Mark all GUPPI shmem segs for deletion.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>

#include "guppi_status.h"
#include "guppi_databuf.h"
#include "guppi_error.h"
#include "guppi_thread_main.h"

int main(int argc, char *argv[]) {
    int rv,ex=0;

    /* Status shared mem, force unlock first */
    struct guppi_status s;
    const char *semname = guppi_status_semname();
    sem_unlink(semname);
    rv = guppi_status_attach(&s);
    if (rv!=GUPPI_OK) {
        fprintf(stderr, "Error connecting to status shared mem.\n");
        perror(NULL);
        exit(1);
    }
    // TODO Use getopt
    // "-d" as command line argument deletes status memory and semaphore.
    // Otherwise, it is simply re-initialized.
    if(argc > 1 && argv[0][0] == '-' && argv[0][1] == 'd') {
      rv = shmctl(s.shmid, IPC_RMID, NULL);
      if (rv==-1) {
          fprintf(stderr, "Error deleting status segment.\n");
          perror("shmctl");
          ex=1;
      }
      rv = sem_unlink(semname);
      if (rv==-1) {
          fprintf(stderr, "Error unlinking status semaphore.\n");
          perror("sem_unlink");
          ex=1;
      }
      printf("Deleted status shared memory and semaphore.\n");
    } else {
      guppi_status_clear(&s);
      printf("Cleared status shared memory.\n");
    }

    /* Databuf shared mem */
    struct guppi_databuf *d=NULL;
    int i = 0;
    for (i=1; i<=2; i++) {
        d = guppi_databuf_attach(i); // Repeat for however many needed ..
        if (d==NULL) continue;
        if (d->semid) { 
            rv = semctl(d->semid, 0, IPC_RMID); 
            if (rv==-1) {
                fprintf(stderr, "Error removing databuf semaphore\n");
                perror("semctl");
                ex=1;
            }
        }
        rv = shmctl(d->shmid, IPC_RMID, NULL);
        if (rv==-1) {
            fprintf(stderr, "Error deleting databuf segment.\n");
            perror("shmctl");
            ex=1;
        }
    }

    exit(ex);
}

