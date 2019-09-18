#include <stdio.h>     /* printf()                 */
#include <stdlib.h>    /* exit(), malloc(), free() */
#include <sys/types.h> /* key_t, sem_t, pid_t      */
#include <sys/shm.h>   /* shmat(), IPC_RMID        */
#include <errno.h>     /* errno, ECHILD            */
#include <semaphore.h> /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>     /* O_CREAT, O_EXEC          */

#include <unistd.h>
#include <sys/wait.h>

int rand_lim(int limit);
struct timespec ts; /* Struct used to determine the amount of seconds a process will wait for a goal */

int main(int argc, char **argv)
{
    int i;     /*      loop variables          */
    int n;     /*      fork count              */
    pid_t pid; /*      fork pid                */

    int timedOut_GoalA;
    int timedOut_GoalB;
    int randomNumber;

    int ball;
    int segundos = 0;

    ts.tv_sec = 3; /* Wait for 3 seconds, then let the goal go. */

    // Structs
    struct player
    {
        char team;
        int player_id;
    };

    struct goal
    {
        int isUsed;
        char team;
        int goals;
    };

    struct goal goalA;
    goalA.team = 'A';
    goalA.goals = 0;
    goalA.isUsed = 1;

    struct goal goalB;
    goalB.team = 'B';
    goalB.goals = 0;
    goalB.isUsed = 1;

    struct player newPlayer;

    /* Block ONE of Shared memory  */
    key_t shmkey;                                        /*      shared memory key       */
    int shmid;                                           /*      shared memory id        */
    int *ballControl; /*      shared variable         */ /*shared */

    sem_t *sem; /*      synch semaphore         */ /*shared */
    int value = 1;                                 /*      semaphore value         */
    /* End of Block ONE */

    /* Block TWO of Shared memory  */
    key_t shmkey_Two;                                 /*      shared memory key       */
    int shmid_Two;                                    /*      shared memory id        */
    int *goalsOnA; /*      shared variable         */ /*shared */

    sem_t *semaphoreGoalA; /*      synch semaphore         */ /*shared */
    int value_Two = goalA.isUsed;                             /*      semaphore value         */
    /* End of Block TWO */

    /* Block THREE of Shared memory  */
    key_t shmkey_Three;                               /*      shared memory key       */
    int shmid_Three;                                  /*      shared memory id        */
    int *goalsOnB; /*      shared variable         */ /*shared */

    sem_t *semaphoreGoalB; /*      synch semaphore         */ /*shared */
    int value_Three = goalB.isUsed;                           /*      semaphore value         */
    /* End of Block THREE */

    /* Block minuto of Shared memory  */
    key_t shmkey_minuto;                              /*      shared memory key       */
    int shmid_minuto;                                 /*      shared memory id        */
    int *p_minuto; /*      shared variable         */ /*shared */
    /* End of Block minuto */

    /* initialize a shared variable in shared memory: ONE */
    shmkey = ftok("/dev/null", 5); /* valid directory name and a number */
    shmid = shmget(shmkey, sizeof(int), 0666 | IPC_CREAT);
    if (shmid < 0)
    { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    ballControl = (int *)shmat(shmid, NULL, 0); /* attach p to shared memory */
    *ballControl = 0;

    /********************************************************/

    /* initialize a shared variable in shared memory: TWO */
    shmkey_Two = ftok("/dev/null", 6); /* valid directory name and a number */
    shmid_Two = shmget(shmkey_Two, sizeof(int), 0666 | IPC_CREAT);
    if (shmid_Two < 0)
    { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    goalsOnA = (int *)shmat(shmid_Two, NULL, 0); /* attach p to shared memory */
    *goalsOnA = goalA.goals;

    /********************************************************/

    /* initialize a shared variable in shared memory: THREE */
    shmkey_Three = ftok("/dev/null", 7); /* valid directory name and a number */
    shmid_Three = shmget(shmkey_Three, sizeof(int), 0666 | IPC_CREAT);
    if (shmid_Three < 0)
    { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    goalsOnB = (int *)shmat(shmid_Three, NULL, 0); /* attach p to shared memory */
    *goalsOnB = goalB.goals;

    /********************************************************/

    /* initialize a shared variable in shared memory: minuto */
    shmkey_minuto = ftok("/dev/null", 8); /* valid directory name and a number */
    shmid_minuto = shmget(shmkey_minuto, sizeof(int), 0666 | IPC_CREAT);
    if (shmid_minuto < 0)
    { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    p_minuto = (int *)shmat(shmid_minuto, NULL, 0); /* attach p to shared memory */
    *p_minuto = 0;

    /********************************************************/
    /* initialize semaphores for shared processes */
    sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, value);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    /* initialize semaphores for shared processes */
    semaphoreGoalA = sem_open("psemaphoreGoalA", O_CREAT | O_EXCL, 0644, value_Two);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    /* initialize semaphores for shared processes */
    semaphoreGoalB = sem_open("psemaphoreGoalB", O_CREAT | O_EXCL, 0644, value_Three);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    printf(" Recursos compartidos y semaforos inicializados .\n\n");

    /* fork child processes */
    for (i = 0; i < 10; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            /* check for error      */
            sem_unlink("pSem");
            sem_close(sem);

            sem_unlink("psemaphoreGoalA");
            sem_close(semaphoreGoalA);

            sem_unlink("psemaphoreGoalB");
            sem_close(semaphoreGoalB);
            /* unlink prevents the semaphore existing forever */
            /* if a crash occurs during the execution         */
            printf("Fork error.\n");
        }
        else if (pid == 0)
            if (i < 5)
            {
                //printf(" Del equipo A\n");
                newPlayer.team = 'A';
                newPlayer.player_id = getpid();
                printf("Hijo numero [%d] con padre de ID: [%d] y de equipo [%c] \n", i, getppid(), newPlayer.team);
                break; /* child processes */
            }
            else
            {
                //printf(" Del equipo B\n");
                newPlayer.team = 'B';
                newPlayer.player_id = getpid();
                printf("Hijo numero [%d] con padre de ID: [%d] y de equipo [%c] \n", i, getppid(), newPlayer.team);
                //printf("  Child(%d) is created. Team(%c)\n", i,newPlayer.team);
                break; /* child processes */
            }
    }

    sleep(2);
    /******************************************************/
    /******************   PARENT PROCESS   ****************/
    /******************************************************/
    if (pid != 0)
    {
        while (*p_minuto < 5)
        {
            if (segundos == 60)
            {
                *p_minuto += 1;
                printf("========================Vamos por el minuto [%d]========================\n", *p_minuto);
                segundos = 0;
            }
            segundos += 1;
            sleep(1);
        }

        /* wait for all children to exit */
        while (pid = waitpid(-1, NULL, 0))
        {
            if (errno == ECHILD)
                break;
        }

        printf("\nParent: All children have exited.\n");
        printf("------------------------------------  Resultado final  ------------------------------------\n");
        printf("====================================TeamA(%d) : TeamB(%d)==================================\n", *goalsOnB, *goalsOnA);
        printf("------------------------------------------- o ---------------------------------------------\n");

        /* shared memory detach */
        shmdt(ballControl);
        shmctl(shmid, IPC_RMID, 0);

        shmdt(goalsOnA);
        shmctl(shmid_Two, IPC_RMID, 0);

        shmdt(goalsOnB);
        shmctl(shmid_Three, IPC_RMID, 0);
        /* cleanup semaphores */
        sem_unlink("pSem");
        sem_close(sem);

        sem_unlink("psemaphoreGoalA");
        sem_close(semaphoreGoalA);

        sem_unlink("psemaphoreGoalB");
        sem_close(semaphoreGoalB);
        /* unlink prevents the semaphore existing forever */
        /* if a crash occurs during the execution         */

        exit(0);
    }

    /******************************************************/
    /******************   CHILD PROCESS   *****************/
    /******************************************************/
    else
    {
        srand(getpid()); /* Sleep before going for goals*/
        while (*p_minuto < 5)
        {
            randomNumber = rand_lim(20);
            printf("El jugador(%d) del equipo(%c) espera por (%d) antes de pedir un recurso\n", i, newPlayer.team,randomNumber);
            sleep(randomNumber);
            printf("El jugador(%d) del equipo(%c) ya puede pedir recursos\n", i, newPlayer.team);
            sem_wait(sem);
            printf("El jugador(%d) del equipo(%c) intenta conseguir la bola\n", i, newPlayer.team);
            //sleep(1);
            if (newPlayer.team == 'A')
            {
                timedOut_GoalB = sem_timedwait(semaphoreGoalB, &ts); 

                if (timedOut_GoalB == -1)
                {
                    if (errno == ETIMEDOUT)
                    {
                        sem_post(sem); 
                        printf("El jugador(%d) del equipo(%c) devolvio la bola despues de intentar conseguir la cancha\n", i, newPlayer.team);
                        randomNumber = rand_lim(20);
                        sleep(randomNumber);
                    }
                    else
                    {
                        perror("sem_timedwait");
                    }
                }

                else
                {
                    printf("El jugador(%d) del equipo(%c) consiguio la bola y la cancha y va a meter un gol\n", i, newPlayer.team);
                    //sleep(1);
                    *goalsOnB += 1; 
                    printf("El jugador(%d) del equipo(%c) metio un golazo\n", i, newPlayer.team);
                    printf("====================================TeamA(%d) : TeamB(%d)====================================\n", *goalsOnB, *goalsOnA);
                    sem_post(semaphoreGoalB); 
                }
            }

            else if (newPlayer.team == 'B')
            {
                timedOut_GoalA = sem_timedwait(semaphoreGoalA, &ts); 

                if (timedOut_GoalA == -1)
                {
                    if (errno == ETIMEDOUT)
                    {
                        sem_post(sem); 
                        printf("El jugador(%d) del equipo(%c) devolvio la bola despues de intentar conseguir la cancha", i, newPlayer.team);
                        randomNumber = rand_lim(20);
                        sleep(randomNumber);
                    }
                    else
                    {
                        perror("sem_timedwait");
                    }
                }

                else
                {
                    printf("El jugador(%d) del equipo(%c) consiguio la bola y la cancha y va a meter un gol\n", i, newPlayer.team);
                    //sleep(1);
                    *goalsOnA += 1; 
                    printf("El jugador(%d) del equipo(%c) metio un golazo\n", i, newPlayer.team);
                    printf("====================================TeamA(%d) : TeamB(%d)====================================\n", *goalsOnB, *goalsOnA);
                    sem_post(semaphoreGoalA); 
                }
            }
            sem_post(sem); 
        }
        exit(0);
    }
}

int rand_lim(int limit)
{
    /* return a random number between 0 and limit inclusive.
 */

    int divisor = RAND_MAX / (limit + 1);
    int retval;

    do
    {
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}