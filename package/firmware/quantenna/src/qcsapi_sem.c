/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2009 - 2012 Quantenna Communications, Inc.          **
**                                                                           **
**  File        : qcsapi_sem.c						     **
**  Description : Locking mechanism for QCSAPI                               **
**                                                                           **
*******************************************************************************
**                                                                           **
**  Redistribution and use in source and binary forms, with or without       **
**  modification, are permitted provided that the following conditions       **
**  are met:                                                                 **
**  1. Redistributions of source code must retain the above copyright        **
**     notice, this list of conditions and the following disclaimer.         **
**  2. Redistributions in binary form must reproduce the above copyright     **
**     notice, this list of conditions and the following disclaimer in the   **
**     documentation and/or other materials provided with the distribution.  **
**  3. The name of the author may not be used to endorse or promote products **
**     derived from this software without specific prior written permission. **
**                                                                           **
**  Alternatively, this software may be distributed under the terms of the   **
**  GNU General Public License ("GPL") version 2, or (at your option) any    **
**  later version as published by the Free Software Foundation.              **
**                                                                           **
**  In the case this software is distributed under the GPL license,          **
**  you should have received a copy of the GNU General Public License        **
**  along with this software; if not, write to the Free Software             **
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  **
**                                                                           **
**  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR       **
**  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES**
**  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  **
**  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,         **
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT **
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,**
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    **
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      **
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF **
**  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        **
**                                                                           **
*******************************************************************************
EH0*/

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

#include "qcsapi.h"

/* Existing path used to generate key for System V semaphore */
#define	LOCK_PATH	"/lib/libqcsapi.so"

/* The maximum times waiting initialization of semaphore */
#define	MAX_TRIES	10

union semun {
	int val;		/* value for SETVAL			*/
	struct semid_ds *buf;	/* buffer for IPC_STAT & IPC_SET	*/
	unsigned short  *array;	/* array for GETALL & SETALL		*/
};

/*
 * postop is used to perform V operation on semaphore
 * waitop is used to perform P operation on semaphore
 */
static struct	sembuf	postop, waitop;
static int		semid, init_flag;

static int sem_enable = 1;

int
qcsapi_sem_init(void)
{
	struct semid_ds	seminfo;
	union semun	arg;
	int oflag, i;

	if (sem_enable == 0)
		return 0;

	oflag = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR;

	/* Init semop() structures: postop and waitop			*/
	postop.sem_num = 0;
	postop.sem_op  = 1;
	postop.sem_flg = SEM_UNDO;

	waitop.sem_num = 0;
	waitop.sem_op  = -1;
	waitop.sem_flg = SEM_UNDO;

	/* Create a new System V semaphore or open a existing semaphore */
	if ((semid = semget(ftok(LOCK_PATH, 0), 1, oflag)) >= 0) {
		arg.val = 1;
		semctl(semid, 0, SETVAL, arg);

		/*
		 * A pair of semops make sem_otime non-zero so that other processes
		 * can judge that the semaphore has been initialized
		 */
		semop(semid, &waitop, 1);
		semop(semid, &postop, 1);

	} else if (errno == EEXIST) {

		/*
		 * The semaphore has been created; wait until it's initialized.
		 * Once initialized, field sem_otime is non-zero.
		 */
		oflag = S_IRUSR | S_IWUSR;
		semid = semget(ftok(LOCK_PATH, 0), 1, oflag);

		arg.buf = &seminfo;
		for (i = 0; i < MAX_TRIES; i++) {
			semctl(semid, 0, IPC_STAT, arg);
			if (arg.buf->sem_otime != 0)
				break;

			sleep(1);
		}

		if (i == MAX_TRIES)
			return -qcsapi_sem_error;

	} else {
		return -errno;
	}

	init_flag = 1;

	return 0;
}

void qcsapi_sem_disable(void)
{
	sem_enable = 0;
}

void
qcsapi_sem_lock(void)
{
	if (sem_enable == 1) {
		if (init_flag == 0)
			qcsapi_sem_init();

		if (init_flag == 1)
			semop(semid, &waitop, 1);
	}
}

void
qcsapi_sem_unlock(void)
{
	if (sem_enable == 1 && init_flag == 1)
		semop(semid, &postop, 1);
}
