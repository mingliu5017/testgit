/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include "semaph.h"

/****************************************************************************
 * Create a semaphore with a specified initial value.
 * If the semaphore already exists, we don't initialize it (of course).
 * We return the semaphore ID if all OK, else -1.
 */
int semCreate(key_t key, int initval)
{
    register int        id, semval;
    union semun {
        int     val;
        struct semid_ds *buf;
        ushort      *array;
    } semctl_arg;
    struct sembuf opLock = {0, 0, 0};

    if (key == IPC_PRIVATE)
        return(-1); /* not intended for private semaphores */

    else if (key == (key_t) -1)
        return(-1); /* probably an ftok() error by caller */
again:
    if ((id = semget(key, 1, 0666 | IPC_CREAT)) < 0)
    {
        printf("semget fail\r\n");
        return(-1); /* permission problem or tables full */
    }

    /*
    if (semop(id, &opLock, 1) < 0) {
        if (errno == EINVAL)
            goto again;
        printf("can't lock\r\n");
    }
    */

    /*
     * Get the value of the process counter.  If it equals 0,
     * then no one has initialized the semaphore yet.
     */

    if ((semval = semctl(id, 0, GETVAL, 0)) < 0)
        printf("can't GETVAL\r\n");

    if (semval != initval) {
        /*
         * We could initialize by doing a SETALL, but that
         * would clear the adjust value that we set when we
         * locked the semaphore above.  Instead, we'll do 2
         * system calls to initialize [0] and [1].
         */
        semctl_arg.val = initval;
        if (semctl(id, 0, SETVAL, semctl_arg) < 0)
            printf("can SETVAL[0]\r\n");
    }

    return(id);
}

/****************************************************************************
 * Open a semaphore that must already exist.
 * This function should be used, instead of semCreate(), if the caller
 * knows that the semaphore must already exist.  For example a client
 * from a client-server pair would use this, if its the server's
 * responsibility to create the semaphore.
 * We return the semaphore ID if all OK, else -1.
 */
int semOpen(key_t key)
{
    register int    id;

    if (key == IPC_PRIVATE)
        return(-1); /* not intended for private semaphores */

    else if (key == (key_t) -1)
        return(-1); /* probably an ftok() error by caller */

    if ((id = semget(key, 1, 0)) < 0)
        return(-1); /* doesn't exist, or tables full */

    return(id);
}

/****************************************************************************
 * Remove a semaphore.
 * This call is intended to be called by a server, for example,
 * when it is being shut down, as we do an IPC_RMID on the semaphore,
 * regardless whether other processes may be using it or not.
 * Most other processes should use semClose() below.
 */
void semRm(int id)
{
    if (semctl(id, 0, IPC_RMID, 0) < 0)
        printf("can't IPC_RMID");
}

/****************************************************************************
 * Close a semaphore.
 * Unlike the remove function above, this function is for a process
 * to call before it exits, when it is done with the semaphore.
 * We "decrement" the counter of processes using the semaphore, and
 * if this was the last one, we can remove the semaphore.
 */
void semClose(int id)
{
    semRm(id);
}

/****************************************************************************
 * Wait until a semaphore's value is greater than 0, then decrement
 * it by 1 and return.
 * Dijkstra's P operation.  Tanenbaum's DOWN operation.
 */
void semP(int id)
{
    semOp(id, -1);
}

/****************************************************************************
 * Increment a semaphore by 1.
 * Dijkstra's V operation.  Tanenbaum's UP operation.
 */
void semV(int id)
{
    semOp(id, 1);
}

/****************************************************************************
 * General semaphore operation.  Increment or decrement by a user-specified
 * amount (positive or negative; amount can't be zero).
 */
void semOp(int id, int value)
{
    struct sembuf op_op = {
                0, 99, SEM_UNDO /* decrement or increment [0] with undo on exit */
                };

    if ((op_op.sem_op = value) == 0)
        printf("can't have value == 0\r\n");

    while (semop(id, &op_op, 1) < 0)
    {
        if(EINTR == errno)
        {
            continue;
        }
        printf("semOp error.id:%d\r\n", id);
        perror("semOp error");
    }
}
