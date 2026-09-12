/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include "gc_hal_kernel_precomp.h"

#define _GC_OBJ_ZONE gcvZONE_DATABASE

/*******************************************************************************
 ***** Private functions *******************************************************/

#define _GetSlot(database, x) \
    ((gctUINT32)(gcmPTR_TO_UINT64(x) % gcmCOUNTOF((database)->list)))

/*******************************************************************************
 **  gckKERNEL_FindDatabase
 **
 **  Find a database identified by a process ID and move it to the head of the
 **  hash list.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          ProcessID that identifies the database.
 **
 **      gctBOOL LastProcessID
 **          gcvTRUE if searching for the last known process ID.  gcvFALSE if
 **          we need to search for the process ID specified by the ProcessID
 **          argument.
 **
 **  OUTPUT:
 **
 **      gcsDATABASE_PTR *Database
 **          Pointer to a variable receiving the database structure pointer on
 **          success.
 */
gceSTATUS
gckKERNEL_FindDatabase(gckKERNEL Kernel, gctUINT32 ProcessID,
                       gctBOOL LastProcessID, gcsDATABASE_PTR *Database)
{
    gceSTATUS status;
    gcsDATABASE_PTR database, previous;
    gctSIZE_T slot;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d LastProcessID=%d",
                   Kernel, ProcessID, LastProcessID);

    /* Compute the hash for the database. */
    slot = ProcessID % gcmCOUNTOF(Kernel->db->db);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Check whether we are getting the last known database. */
    if (LastProcessID) {
        /* Use last database. */
        database = Kernel->db->lastDatabase;

        if (database == gcvNULL) {
            /* Database not found. */
            gcmkONERROR(gcvSTATUS_INVALID_DATA);
        }
    } else {
        /* Walk the hash list. */
        for (previous = gcvNULL, database = Kernel->db->db[slot];
             database != gcvNULL;
             database = database->next) {
            if (database->processID == ProcessID) {
                /* Found it! */
                break;
            }

            previous = database;
        }

        if (database == gcvNULL) {
            /* Database not found. */
            gcmkONERROR(gcvSTATUS_INVALID_DATA);
        }

        if (previous != gcvNULL) {
            /* Move database to the head of the hash list. */
            previous->next = database->next;
            database->next = Kernel->db->db[slot];
            Kernel->db->db[slot] = database;
        }
    }

    /* Release the database mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));

    /* Return the database. */
    *Database = database;

    /* Success. */
    gcmkFOOTER_ARG("*Database=%p", *Database);
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_DeinitDatabase
 **
 **  De-init a database structure.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gcsDATABASE_PTR Database
 **          Pointer to the database structure to deinit.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
static gceSTATUS
gckKERNEL_DeinitDatabase(gckKERNEL Kernel, gcsDATABASE_PTR Database)
{
    gcmkHEADER_ARG("Kernel=%p Database=%p", Kernel, Database);

    if (Database) {
        Database->deleted = gcvFALSE;

        if (Kernel->processPageTable)
            Database->mmu = gcvNULL;

        /* Destroy handle db. */
        if (Database->refs) {
            gcmkVERIFY_OK(gckOS_AtomDestroy(Kernel->os, Database->refs));
            Database->refs = gcvNULL;
        }

        if (Database->handleDatabase) {
            gcmkVERIFY_OK(gckKERNEL_DestroyIntegerDatabase(Kernel,
                                                           Database->handleDatabase));
            Database->handleDatabase = gcvNULL;
        }

        if (Database->handleDatabaseMutex) {
            gcmkVERIFY_OK(gckOS_DeleteMutex(Kernel->os,
                                            Database->handleDatabaseMutex));
            Database->handleDatabaseMutex = gcvNULL;
        }
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **  gckKERNEL_NewRecord
 **
 **  Create a new database record structure and insert it to the head of the
 **  database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gcsDATABASE_PTR Database
 **          Pointer to a database structure.
 **
 **  OUTPUT:
 **
 **      gcsDATABASE_RECORD_PTR * Record
 **          Pointer to a variable receiving the database record structure
 **          pointer on success.
 */
static gceSTATUS
gckKERNEL_NewRecord(gckKERNEL Kernel, gcsDATABASE_PTR Database,
                    gctUINT32 Slot, gcsDATABASE_RECORD_PTR *Record)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gcsDATABASE_RECORD_PTR record = gcvNULL;

    gcmkHEADER_ARG("Kernel=%p Database=%p", Kernel, Database);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    if (Kernel->db->freeRecord != gcvNULL) {
        /* Allocate the record from the free list. */
        record = Kernel->db->freeRecord;
        Kernel->db->freeRecord = record->next;
    } else {
        gctPOINTER pointer = gcvNULL;

        /* Allocate the record from the heap. */
        gcmkONERROR(gckOS_Allocate(Kernel->os, gcmSIZEOF(gcsDATABASE_RECORD), &pointer));

        record = pointer;
    }

    /* Insert the record in the database. */
    record->next = Database->list[Slot];
    Database->list[Slot] = record;

    /* Release the database mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));

    /* Return the record. */
    *Record = record;

    /* Success. */
    gcmkFOOTER_ARG("*Record=%p", *Record);
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }
    if (record != gcvNULL)
        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Kernel->os, record));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_DeleteRecord
 **
 **  Remove a database record from the database and delete its structure.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gcsDATABASE_PTR Database
 **          Pointer to a database structure.
 **
 **      gceDATABASE_TYPE Type
 **          Type of the record to remove.
 **
 **      gctPOINTER Data
 **          Data of the record to remove.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T_PTR Bytes
 **          Pointer to a variable that receives the size of the record deleted.
 **          Can be gcvNULL if the size is not required.
 */
static gceSTATUS
gckKERNEL_DeleteRecord(gckKERNEL Kernel, gcsDATABASE_PTR Database,
                       gceDATABASE_TYPE Type, gctPOINTER Data,
                       gctSIZE_T_PTR Bytes OPTIONAL)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gcsDATABASE_RECORD_PTR record, previous;
    gctUINT32 slot = _GetSlot(Database, Data);

    gcmkHEADER_ARG("Kernel=%p Database=%p Type=%d Data=%p",
                   Kernel, Database, Type, Data);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Scan the database for this record. */
    for (record = Database->list[slot], previous = gcvNULL; record != gcvNULL;
         record = record->next) {
        if (record->type == Type && record->data == Data) {
            /* Found it! */
            break;
        }

        previous = record;
    }

    if (record == gcvNULL) {
        /* Ouch!  This record is not found? */
        gcmkONERROR(gcvSTATUS_INVALID_DATA);
    }

    if (Bytes != gcvNULL) {
        /* Return size of record. */
        *Bytes = record->bytes;
    }

    /* Remove record from database. */
    if (previous == gcvNULL)
        Database->list[slot] = record->next;
    else
        previous->next = record->next;

    /* Insert record in free list. */
    record->next = Kernel->db->freeRecord;
    Kernel->db->freeRecord = record;

    /* Release the database mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=%zu", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_FindRecord
 **
 **  Find a database record from the database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gcsDATABASE_PTR Database
 **          Pointer to a database structure.
 **
 **      gceDATABASE_TYPE Type
 **          Type of the record to remove.
 **
 **      gctPOINTER Data
 **          Data of the record to remove.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T_PTR Bytes
 **          Pointer to a variable that receives the size of the record deleted.
 **          Can be gcvNULL if the size is not required.
 */
static gceSTATUS
gckKERNEL_FindRecord(gckKERNEL Kernel, gcsDATABASE_PTR Database,
                     gceDATABASE_TYPE Type, gctPOINTER Data,
                     gcsDATABASE_RECORD_PTR Record)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gcsDATABASE_RECORD_PTR record;
    gctUINT32 slot = _GetSlot(Database, Data);

    gcmkHEADER_ARG("Kernel=%p Database=%p Type=%d Data=%p",
                   Kernel, Database, Type, Data);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Scan the database for this record. */
    for (record = Database->list[slot]; record != gcvNULL; record = record->next) {
        if (record->type == Type && record->data == Data) {
            /* Found it! */
            break;
        }
    }

    if (record == gcvNULL) {
        /* Ouch!  This record is not found? */
        gcmkONERROR(gcvSTATUS_INVALID_DATA);
    }

    if (Record != gcvNULL) {
        /* Return information of record. */
        gcmkONERROR(gckOS_MemCopy(Record, record, sizeof(gcsDATABASE_RECORD)));
    }

    /* Release the database mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));

    /* Success. */
    gcmkFOOTER_ARG("Record=%p", Record);
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 ***** Public API **************************************************************/

/*******************************************************************************
 **  gckKERNEL_CreateProcessDB
 **
 **  Create a new process database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckKERNEL_CreateProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsDATABASE_PTR database = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctBOOL acquired = gcvFALSE;
    gctSIZE_T slot;
    gctUINT32 i;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d", Kernel, ProcessID);

    /* Compute the hash for the database. */
    slot = ProcessID % gcmCOUNTOF(Kernel->db->db);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Walk the hash list. */
    for (database = Kernel->db->db[slot]; database != gcvNULL; database = database->next) {
        if (database->processID == ProcessID) {
            gctINT32 oldVal = 0;

            if (database->deleted) {
                gcmkFATAL("%s(%d): DB of Process=0x%x database=%llx cannot be reentered since it was in deletion\n",
                          __FUNCTION__, __LINE__, ProcessID, database);

                gcmkONERROR(gcvSTATUS_INVALID_REQUEST);
            }

            if (database->refs == gcvNULL)
                gcmkONERROR(gcvSTATUS_INVALID_ADDRESS);

            gcmkVERIFY_OK(gckOS_AtomIncrement(Kernel->os, database->refs, &oldVal));
            goto OnExit;
        }
    }

    if (Kernel->db->freeDatabase) {
        /* Allocate a database from the free list. */
        database = Kernel->db->freeDatabase;
        Kernel->db->freeDatabase = database->next;
    } else {
        /* Allocate a new database from the heap. */
        gcmkONERROR(gckOS_Allocate(Kernel->os, gcmSIZEOF(gcsDATABASE), &pointer));

        gckOS_ZeroMemory(pointer, gcmSIZEOF(gcsDATABASE));

        database = pointer;

        gcmkONERROR(gckOS_CreateMutex(Kernel->os, &database->counterMutex));
    }

    /* Initialize the database. */
    /* Save the hash slot. */
    database->slot = slot;
    database->processID = ProcessID;
    database->vidMem.bytes = 0;
    database->vidMem.maxBytes = 0;
    database->vidMem.totalBytes = 0;
    database->vidMem.freeCount = 0;
    database->vidMem.allocCount = 0;

    database->nonPaged.bytes = 0;
    database->nonPaged.maxBytes = 0;
    database->nonPaged.totalBytes = 0;
    database->nonPaged.freeCount = 0;
    database->nonPaged.allocCount = 0;

    database->mapMemory.bytes = 0;
    database->mapMemory.maxBytes = 0;
    database->mapMemory.totalBytes = 0;
    database->mapMemory.freeCount = 0;
    database->mapMemory.allocCount = 0;

    for (i = 0; i < gcmCOUNTOF(database->list); i++)
        database->list[i] = gcvNULL;

    for (i = 0; i < gcvVIDMEM_TYPE_COUNT; i++) {
        database->vidMemType[i].bytes = 0;
        database->vidMemType[i].maxBytes = 0;
        database->vidMemType[i].totalBytes = 0;
        database->vidMemType[i].freeCount = 0;
        database->vidMemType[i].allocCount = 0;
    }

    for (i = 0; i < gcvPOOL_NUMBER_OF_POOLS; i++) {
        database->vidMemPool[i].bytes = 0;
        database->vidMemPool[i].maxBytes = 0;
        database->vidMemPool[i].totalBytes = 0;
        database->vidMemPool[i].freeCount = 0;
        database->vidMemPool[i].allocCount = 0;
    }

    if (Kernel->processPageTable)
        gcmkONERROR(gckMMU_ConstructProcessMMU(Kernel, ProcessID, &database->mmu));

    gcmkASSERT(database->refs == gcvNULL);
    gcmkONERROR(gckOS_AtomConstruct(Kernel->os, &database->refs));
    gcmkONERROR(gckOS_AtomSet(Kernel->os, database->refs, 1));

    gcmkASSERT(database->handleDatabase == gcvNULL);
    gcmkONERROR(gckKERNEL_CreateIntegerDatabase(Kernel, 64, &database->handleDatabase));

    gcmkASSERT(database->handleDatabaseMutex == gcvNULL);
    gcmkONERROR(gckOS_CreateMutex(Kernel->os, &database->handleDatabaseMutex));

    /* Insert the database into the hash. */
    database->next = Kernel->db->db[slot];
    Kernel->db->db[slot] = database;

    /* Reset idle timer. */
    Kernel->db->lastIdle = 0;

#if gcdENABLE_PERF_DISPATCH
    for (i = 0; i < gcvHAL_NUM_COMMAND_CODES; i++)
        gcmkONERROR(gckOS_CreateMutex(Kernel->os, &database->dispatchPerfRecords[i].mutex));
#endif

OnError:
    if (gcmIS_ERROR(status)) {
        if (database->mmu) {
            gckMMU_DestroyProcessMMU(database->mmu);
            database->mmu = gcvNULL;
        }
        gcmkVERIFY_OK(gckKERNEL_DeinitDatabase(Kernel, database));

        if (pointer) {
            if (database->counterMutex) {
                gcmkVERIFY_OK(gckOS_DeleteMutex(Kernel->os, database->counterMutex));
                database->counterMutex = gcvNULL;
            }
            /* Safe free memory to avid dangling pointer */
            gcmkOS_SAFE_FREE(Kernel->os, pointer);
        } else {
            /* Return back to free list */
            database->next = Kernel->db->freeDatabase;
            Kernel->db->freeDatabase = database;
        }

        if (database->handleDatabaseMutex) {
            gcmkVERIFY_OK(gckOS_DeleteMutex(Kernel->os, database->handleDatabaseMutex));
            database->handleDatabaseMutex = gcvNULL;
        }

#if gcdENABLE_PERF_DISPATCH
        for (i = 0; i < gcvHAL_NUM_COMMAND_CODES; i++) {
            if (database->dispatchPerfRecords[i].mutex) {
                gcmkVERIFY_OK(gckOS_DeleteMutex(Kernel->os, database->dispatchPerfRecords[i].mutex));
                database->dispatchPerfRecords[i].mutex = gcvNULL;
            }
        }
#endif
    }

OnExit:

    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_AddProcessDB
 **
 **  Add a record to a process database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **      gceDATABASE_TYPE TYPE
 **          Type of the record to add.
 **
 **      gctPOINTER Pointer
 **          Data of the record to add.
 **
 **      gctPHYS_ADDR Physical
 **          Physical address of the record to add.
 **
 **      gctSIZE_T Size
 **          Size of the record to add.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckKERNEL_AddProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID,
                       gceDATABASE_TYPE Type, gctPOINTER Pointer,
                       gctPHYS_ADDR Physical, gctSIZE_T Size)
{
    gceSTATUS status;
    gcsDATABASE_PTR database = gcvNULL;
    gcsDATABASE_RECORD_PTR record = gcvNULL;
    gcsDATABASE_COUNTERS  *count = gcvNULL;
    gctUINT32 vidMemType;
    gcePOOL vidMemPool;
    gceDATABASE_TYPE vidMemDbType;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d Type=%d Pointer=%p Physical=%p Size=%lu",
                   Kernel, ProcessID, Type, Pointer, Physical, Size);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    /* Decode type. */
    vidMemType = (Type & gcdDB_VIDEO_MEMORY_TYPE_MASK) >> gcdDB_VIDEO_MEMORY_TYPE_SHIFT;
    vidMemPool = (Type & gcdDB_VIDEO_MEMORY_POOL_MASK) >> gcdDB_VIDEO_MEMORY_POOL_SHIFT;
    vidMemDbType = (Type & gcdDB_VIDEO_MEMORY_DBTYPE_MASK) >> gcdDB_VIDEO_MEMORY_DBTYPE_SHIFT;

    Type &= gcdDATABASE_TYPE_MASK;

    /* Special case the idle record. */
    if (Type == gcvDB_IDLE) {
        gctUINT64 time;

        /* Get the current profile time. */
        gcmkONERROR(gckOS_GetProfileTick(&time));

        if (ProcessID == 0 && Kernel->db->lastIdle != 0) {
            /* Out of idle, adjust time it was idle. */
            Kernel->db->idleTime += time - Kernel->db->lastIdle;
            Kernel->db->lastIdle = 0;
        } else if (ProcessID == 1) {
            /* Save current idle time. */
            Kernel->db->lastIdle = time;
        }

#if gcdDYNAMIC_SPEED
        {
            /* Test for first call. */
            if (Kernel->db->lastSlowdown == 0) {
                /* Save milliseconds. */
                Kernel->db->lastSlowdown = time;
                Kernel->db->lastSlowdownIdle = Kernel->db->idleTime;
            } else {
                /* Compute ellapsed time in milliseconds. */
                gctUINT delta = gckOS_ProfileToMS(time - Kernel->db->lastSlowdown);

                /* Test for end of period. */
                if (delta >= gcdDYNAMIC_SPEED) {
                    /* Compute number of idle milliseconds. */
                    gctUINT idle =
                        gckOS_ProfileToMS(Kernel->db->idleTime - Kernel->db->lastSlowdownIdle);

                    /* Broadcast to slow down the GPU. */
                    gcmkONERROR(gckOS_BroadcastCalibrateSpeed(Kernel->os,
                                                              Kernel->hardware,
                                                              idle, delta));

                    /* Save current time. */
                    Kernel->db->lastSlowdown = time;
                    Kernel->db->lastSlowdownIdle = Kernel->db->idleTime;
                }
            }
        }
#endif

        /* Success. */
        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

#if gcdENABLE_SW_PREEMPTION
    if (Type == gcvDB_PRIORITY) {
        gctUINT32 id = gcmPTR2INT32(Pointer);

        Kernel->priorityDBCreated[id] = gcvTRUE;
    }
#endif

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    /* Create a new record in the database. */
    gcmkONERROR(gckKERNEL_NewRecord(Kernel, database, _GetSlot(database, Pointer), &record));

    /* Initialize the record. */
    record->kernel = Kernel;
    record->type = Type;
    record->data = Pointer;
    record->physical = Physical;
    record->bytes = Size;

    /* Get pointer to counters. */
    switch (Type) {
    case gcvDB_VIDEO_MEMORY:
        count = &database->vidMem;
        break;

    case gcvDB_NON_PAGED:
        count = &database->nonPaged;
        break;

    case gcvDB_CONTIGUOUS:
        count = &database->contiguous;
        break;

    case gcvDB_MAP_MEMORY:
        count = &database->mapMemory;
        break;

    default:
        count = gcvNULL;
        break;
    }

    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, database->counterMutex, gcvINFINITE));
    acquired = gcvTRUE;

    if (count != gcvNULL) {
        /* Adjust counters. */
        count->totalBytes += Size;
        count->bytes += Size;
        count->allocCount++;

        if (count->bytes > count->maxBytes)
            count->maxBytes = count->bytes;
    }

    if (Type == gcvDB_VIDEO_MEMORY) {
        count = &database->vidMemType[vidMemType];

        /* Adjust counters. */
        count->totalBytes += Size;
        count->bytes += Size;
        count->allocCount++;

        if (count->bytes > count->maxBytes)
            count->maxBytes = count->bytes;

        count = &database->vidMemPool[vidMemPool];

        /* Adjust counters. */
        count->totalBytes += Size;
        count->bytes += Size;
        count->allocCount++;

        if (count->bytes > count->maxBytes)
            count->maxBytes = count->bytes;

        if (vidMemDbType == gcvDB_CONTIGUOUS) {
            count = &database->contiguous;

            /* Adjust counters. */
            count->totalBytes += Size;
            count->bytes += Size;
            count->allocCount++;

            if (count->bytes > count->maxBytes)
                count->maxBytes = count->bytes;
        }
    }

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, database->counterMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, database->counterMutex));

    if (record)
        gcmkVERIFY_OK(gckKERNEL_DeleteRecord(Kernel, database, Type, Pointer, gcvNULL));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_RemoveProcessDB
 **
 **  Remove a record from a process database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **      gceDATABASE_TYPE TYPE
 **          Type of the record to remove.
 **
 **      gctPOINTER Pointer
 **          Data of the record to remove.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckKERNEL_RemoveProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID,
                          gceDATABASE_TYPE Type, gctPOINTER Pointer)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gctSIZE_T bytes = 0;
    gctUINT32 vidMemType;
    gcePOOL vidMemPool;
    gceDATABASE_TYPE vidMemDbType;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d Type=%d Pointer=%p",
                   Kernel, ProcessID, Type, Pointer);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    /* Decode type. */
    vidMemType = (Type & gcdDB_VIDEO_MEMORY_TYPE_MASK) >> gcdDB_VIDEO_MEMORY_TYPE_SHIFT;
    vidMemPool = (Type & gcdDB_VIDEO_MEMORY_POOL_MASK) >> gcdDB_VIDEO_MEMORY_POOL_SHIFT;
    vidMemDbType = (Type & gcdDB_VIDEO_MEMORY_DBTYPE_MASK) >> gcdDB_VIDEO_MEMORY_DBTYPE_SHIFT;

    Type &= gcdDATABASE_TYPE_MASK;

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    /* Delete the record. */
    gcmkONERROR(gckKERNEL_DeleteRecord(Kernel, database, Type, Pointer, &bytes));

    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, database->counterMutex, gcvINFINITE));

    /* Update counters. */
    switch (Type) {
    case gcvDB_VIDEO_MEMORY:
        database->vidMem.bytes -= bytes;
        database->vidMem.freeCount++;
        database->vidMemType[vidMemType].bytes -= bytes;
        database->vidMemType[vidMemType].freeCount++;
        database->vidMemPool[vidMemPool].bytes -= bytes;
        database->vidMemPool[vidMemPool].freeCount++;

        if (vidMemDbType == gcvDB_CONTIGUOUS) {
            database->contiguous.bytes -= bytes;
            database->contiguous.freeCount++;
        }

        break;

    case gcvDB_NON_PAGED:
        database->nonPaged.bytes -= bytes;
        database->nonPaged.freeCount++;
        break;

    case gcvDB_CONTIGUOUS:
        database->contiguous.bytes -= bytes;
        database->contiguous.freeCount++;
        break;

    case gcvDB_MAP_MEMORY:
        database->mapMemory.bytes -= bytes;
        database->mapMemory.freeCount++;
        break;

    default:
        break;
    }

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, database->counterMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_FindProcessDB
 **
 **  Find a record from a process database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **      gceDATABASE_TYPE TYPE
 **          Type of the record to remove.
 **
 **      gctPOINTER Pointer
 **          Data of the record to remove.
 **
 **  OUTPUT:
 **
 **      gcsDATABASE_RECORD_PTR Record
 **          Copy of record.
 */
gceSTATUS
gckKERNEL_FindProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID,
                        gctUINT32 ThreadID, gceDATABASE_TYPE Type,
                        gctPOINTER Pointer, gcsDATABASE_RECORD_PTR Record)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d ThreadID=%d Type=%d Pointer=%p",
                   Kernel, ProcessID, ThreadID, Type, Pointer);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);
    gcmkVERIFY_ARGUMENT(Pointer != gcvNULL);

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    /* Find the record. */
    gcmkONERROR(gckKERNEL_FindRecord(Kernel, database, Type, Pointer, Record));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_FindProcessMMU
 **
 **  Find the mmu page table from a process database.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **  OUTPUT:
 **
 **      gckMMU *Mmu
 */
gceSTATUS
gckKERNEL_FindProcessMMU(gckKERNEL Kernel, gctUINT32 ProcessID, gckMMU *Mmu)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsDATABASE_PTR database;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d", Kernel, ProcessID);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);
    gcmkVERIFY_ARGUMENT(Mmu != gcvNULL);

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    *Mmu = database->mmu;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_GetCurrentMMU(gckKERNEL Kernel, gctBOOL FromUser, gctUINT32 ProcessID, gckMMU *Mmu)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Kernel=%p", Kernel);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    if (Kernel->processPageTable && FromUser) {
        gctUINT32 processID = ProcessID;

        /* If ProcessID is not passed. */
        if (!processID)
            gcmkONERROR(gckOS_GetProcessID(&processID));

        gcmkONERROR(gckKERNEL_FindProcessMMU(Kernel, processID, Mmu));

        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

    *Mmu = Kernel->mmu;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_RemoveDatabaseFromList(gckKERNEL Kernel, gcsDATABASE_PTR Database, gctUINT32 ProcessID)
{
    gcsDATABASE_PTR db = gcvNULL;
    gcsDATABASE_PTR previous = gcvNULL;
    gcsDATABASE_PTR database = Database;
    gctBOOL acquired = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T slot;

    gcmkHEADER_ARG("Kernel=%p Database=%d", Kernel, Database);

    /* Compute the hash for the database. */
    slot = ProcessID % gcmCOUNTOF(Kernel->db->db);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Walk the hash list. */
    for (db = Kernel->db->db[slot]; db != gcvNULL; db = db->next) {
        if (db->processID == ProcessID)
            break;

        previous = db;
    }

    if (db != database || !db->deleted) {
        gcmkFATAL("%s(%d): DB of Process=0x%x corrupted after found in deletion\n",
                  __FUNCTION__, __LINE__, ProcessID);
        gcmkONERROR(gcvSTATUS_NOT_FOUND);
    }

    /* Remove the database from the hash list. */
    if (previous)
        previous->next = database->next;
    else
        Kernel->db->db[slot] = database->next;

    /* Deinit current database. */
    gcmkVERIFY_OK(gckKERNEL_DeinitDatabase(Kernel, database));

    if (Kernel->db->lastDatabase) {
        /* Insert last database to the free list. */
        Kernel->db->lastDatabase->next = Kernel->db->freeDatabase;
        Kernel->db->freeDatabase = Kernel->db->lastDatabase;
    }

    /* Update last database to current one. */
    Kernel->db->lastDatabase = database;

OnError:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_DestroyProcessDB
 **
 **  Destroy a process database.  If the database contains any records, the data
 **  inside those records will be deleted as well.  This aids in the cleanup if
 **  a process has died unexpectedly or has memory leaks.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckKERNEL_DestroyProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckKERNEL kernel = Kernel;
    gcsDATABASE_PTR database = gcvNULL;
    gctBOOL acquired = gcvFALSE;
    gctSIZE_T slot;
    gctUINT32 i;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d", Kernel, ProcessID);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    /* Compute the hash for the database. */
    slot = ProcessID % gcmCOUNTOF(Kernel->db->db);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Walk the hash list. */
    for (database = Kernel->db->db[slot]; database != gcvNULL; database = database->next) {
        if (database->processID == ProcessID)
            break;
    }

    if (database) {
        gctINT32 oldVal = 0;

        if (database->refs == gcvNULL)
            gcmkONERROR(gcvSTATUS_INVALID_ADDRESS);

        gcmkONERROR(gckOS_AtomDecrement(Kernel->os, database->refs, &oldVal));
        if (oldVal != 1)
            goto OnExit;

        /* Mark it for delete so disallow reenter until really delete it */
        gcmkASSERT(!database->deleted);

        database->deleted = gcvTRUE;
    } else {
        gcmkFATAL("%s(%d): DB destroy of Process=0x%x cannot match with creation\n",
                  __FUNCTION__, __LINE__, ProcessID);
        gcmkONERROR(gcvSTATUS_NOT_FOUND);
    }

#if gcdCAPTURE_ONLY_MODE
    gcmkPRINT("Capture only mode: The max allocation from System Pool is %llu bytes",
              database->vidMemPool[gcvPOOL_SYSTEM].maxBytes);
#endif

    /* Cannot remove the database from the hash list
     * since later records deinit need to access from the hash
     */

    gcmkONERROR(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    acquired = gcvFALSE;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DATABASE,
                   "DB(%d): VidMem: total=%llu max=%llu",
                   ProcessID, database->vidMem.totalBytes, database->vidMem.maxBytes);
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DATABASE,
                   "DB(%d): NonPaged: total=%llu max=%llu",
                   ProcessID, database->nonPaged.totalBytes, database->nonPaged.maxBytes);
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DATABASE,
                   "DB(%d): Idle time=%llu",
                   ProcessID, Kernel->db->idleTime);
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DATABASE,
                   "DB(%d): Map: total=%llu max=%llu",
                   ProcessID, database->mapMemory.totalBytes, database->mapMemory.maxBytes);

    {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                       "Process %d has entries in its database:", ProcessID);
    }

    for (i = 0; i < gcmCOUNTOF(database->list); i++) {
        gcsDATABASE_RECORD_PTR record, next;

        /* Walk all records. */
        for (record = database->list[i]; record != gcvNULL; record = next) {
            gctBOOL asynchronous = gcvTRUE;
            gckVIDMEM_NODE nodeObject;
            gctPHYS_ADDR physical;
            gctUINT32 handle;
            gckMMU mmu = gcvNULL;

            /* Next record. */
            next = record->next;

            /* Dispatch on record type. */
            switch (record->type) {
            case gcvDB_VIDEO_MEMORY:
                gcmkERR_BREAK(gckVIDMEM_HANDLE_Lookup(record->kernel, ProcessID,
                                                      gcmPTR2INT32(record->data), &nodeObject));

                /* Free the video memory. */
                gcmkVERIFY_OK(gckVIDMEM_HANDLE_Dereference(record->kernel, ProcessID,
                                                           gcmPTR2INT32(record->data)));

                gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(record->kernel, nodeObject));

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: VIDEO_MEMORY %p (status=%d)",
                               record->data, status);
                break;

            case gcvDB_NON_PAGED:
                physical = gcmNAME_TO_PTR(record->physical);

                /* Free the non paged memory. */
                status = gckOS_FreeNonPagedMemory(Kernel->os, physical,
                                                  record->data, record->bytes);

                gcmRELEASE_NAME(record->physical);

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: NON_PAGED %p, bytes=%lu (status=%d)",
                               record->data, record->bytes, status);
                break;

            case gcvDB_SIGNAL:
#if USE_NEW_LINUX_SIGNAL
                status = gcvSTATUS_NOT_SUPPORTED;
#else
                /* Free the user signal. */
                status = gckOS_DestroyUserSignal(Kernel->os,
                                                 gcmPTR2INT32(record->data));
#endif /* USE_NEW_LINUX_SIGNAL */

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: SIGNAL %d (status=%d)",
                               (gctINT)(gctUINTPTR_T)record->data, status);
                break;

            case gcvDB_VIDEO_MEMORY_LOCKED:
                handle = gcmPTR2INT32(record->data);

                gcmkERR_BREAK(gckVIDMEM_HANDLE_Lookup(record->kernel, ProcessID,
                                                      handle, &nodeObject));

                /* Unlock CPU. */
                gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(record->kernel, nodeObject,
                                                       ProcessID, gcvTRUE, gcvFALSE));

                gcmkVERIFY_OK(gckKERNEL_GetCurrentMMU(record->kernel, gcvTRUE, ProcessID, &mmu));

                /* Unlock what we still locked */
                status = gckVIDMEM_NODE_Unlock(record->kernel, nodeObject,
                                               mmu, &asynchronous);

                /* Deref handle. */
                gcmkVERIFY_OK(gckVIDMEM_HANDLE_Dereference(record->kernel, ProcessID, handle));

                if (gcmIS_SUCCESS(status) && gcvTRUE == asynchronous
                    && !record->kernel->processPageTable) {

                    /* Schedule unlock: will unlock and deref node later. */
                    status = gckEVENT_Unlock(record->kernel->eventObj,
                                                gcvKERNEL_PIXEL, mmu, nodeObject);
                } else {
                    /* Deref node */
                    gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(record->kernel, nodeObject));
                }

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: VIDEO_MEMORY_LOCKED %p (status=%d)",
                               record->data, status);
                break;

            case gcvDB_CONTEXT:
                status = gckCOMMAND_Detach(record->kernel->command,
                                           gcmNAME_TO_PTR(record->data));
                gcmRELEASE_NAME(record->data);

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: CONTEXT %p (status=%d)", record->data, status);
                break;

            case gcvDB_MAP_MEMORY:
                /* Unmap memory. */
                status = gckKERNEL_UnmapMemory(record->kernel, record->physical,
                                               record->bytes, record->data, ProcessID);

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: MAP MEMORY %p (status=%d)", record->data, status);
                break;

            case gcvDB_SHBUF:
                /* Free shared buffer. */
                status = gckKERNEL_DestroyShBuffer(record->kernel, (gctSHBUF)record->data);

                gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DATABASE,
                               "DB: SHBUF %u (status=%d)",
                               (gctUINT32)(gctUINTPTR_T)record->data, status);
                break;

#if gcdENABLE_SW_PREEMPTION
            case gcvDB_PRIORITY:
                /* Commit done and trigger the lower priority queue. */
                {
                    gctUINT32 priorityID = gcmPTR2INT32(record->data);
                    gceHARDWARE_TYPE type = Kernel->hardware->type;
                    gctINT32 id = 0;

                    if (priorityID >= gcdMAX_PRIORITY_QUEUE_NUM) {
                        gcmkPRINT("ftg340 Info: get an error priority.");
                        break;
                    }

                    Kernel->priorityDBCreated[priorityID] = gcvFALSE;

                    gcmkVERIFY_OK(gckOS_AtomGet(Kernel->os, Kernel->device->atomPriorityID, &id));

                    if (id > 0 && priorityID == (gctUINT32)id) {
                        gcmkVERIFY_OK(gckOS_AtomDecrement(Kernel->os,
                                                          Kernel->device->atomPriorityID,
                                                          &id));
                        while (--id) {
                            gcmkVERIFY_OK(gckOS_AcquireMutex(Kernel->os,
                                                             Kernel->priorityQueueMutex[id],
                                                             gcvINFINITE));
                            if (!Kernel->priorityQueues[id] || !Kernel->priorityQueues[id]->head) {
                                gcmkVERIFY_OK(gckOS_AtomDecrement(Kernel->os,
                                                                  kernel->device->atomPriorityID,
                                                                  &id));
                            }

                            gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os,
                                                             Kernel->priorityQueueMutex[id]));
                        }
                    }

                    if (type == gcvHARDWARE_3D ||
                        type == gcvHARDWARE_3D2D ||
                        type == gcvHARDWARE_VIP) {
                        gckKERNEL kernel = gcvNULL;
                        gctUINT i;

                        for (i = 0; i < Kernel->device->coreNum; i++) {
                            kernel = Kernel->device->coreInfoArray[i].kernel;

                            gcmkVERIFY_OK(gckOS_ReleaseSemaphoreEx(kernel->os,
                                                                   kernel->preemptSema));
                        }
                    }
                }

                break;
#endif

#if gcdENABLE_CLEAR_FENCE
            case gcvDB_USER_FENCE:
                {
                    gcsUSER_FENCE_INFO_PTR fence_info = gcvNULL;
                    gckDEVICE device = Kernel->device;

                    fence_info = gckOS_ReleaseFenceRecordId(device, gcmPTR_TO_UINT64(record->data));

                    gcmkERR_BREAK(gckOS_AcquireMutex(device->os,
                                                     device->fenceListMutex,
                                                     gcvINFINITE));

                    if (fence_info)
                        gcsLIST_Del(&fence_info->fenceLink);
                    else
                        gcmkPRINT("Failed to find the Fence record!\n");

                    gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os,
                                                     device->fenceListMutex));

                    gckOS_Free(device->os, (gctPOINTER)fence_info);
                }
                break;
#endif

            default:
                gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DATABASE,
                               "DB: Correcupted record=0x%08x type=%d",
                               record, record->type);
                break;
            }

            /* Delete the record. */
            gcmkONERROR(gckKERNEL_DeleteRecord(Kernel, database,
                                               record->type, record->data, gcvNULL));
        }
    }

    gcmkONERROR(gckKERNEL_DestroyProcessReservedUserMap(Kernel, ProcessID));

    /* Destroy this process MMU. */
    if (Kernel->processPageTable && database->mmu) {
        gcsHAL_INTERFACE iface = {0};

        iface.command = gcvHAL_DESTROY_MMU;
        iface.u.DestroyMmu.mmu = gcmPTR_TO_UINT64(database->mmu);
        iface.u.DestroyMmu.pid = ProcessID;
        iface.u.DestroyMmu.database = gcmPTR_TO_UINT64(database);

        gcmkONERROR(gckEVENT_AddList(Kernel->eventObj, &iface,
                                     gcvKERNEL_PIXEL, gcvFALSE, gcvTRUE));
    }

    gcmkONERROR(gckKERNEL_RemoveDatabaseFromList(Kernel, database, ProcessID));

#if gcdENABLE_PERF_DISPATCH
    {
        gctUINT i;

        gcmkPRINT("DISPATCH PERF for [%u]:", ProcessID);
        gcmkPRINT("  id\ttotal\tfailed\taverage\tmaximum");
        for (i = 0; i < gcvHAL_NUM_COMMAND_CODES; i++) {
            gcsDISPATCH_PERF_RECORD *record = &database->dispatchPerfRecords[i];

            if (record->mutex) {
                gcmkVERIFY_OK(gckOS_DeleteMutex(Kernel->os, record->mutex));
                record->mutex = gcvNULL;
            }

            if (record->count != 0)
                gcmkPRINT("  %u\t%llu\t%llu\t%llu\t%u", i,
                          record->count, record->failed, record->cost / record->count, record->maximum);
        }
    }
#endif

OnError:
OnExit:
    if (acquired) {
        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
    }
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **  gckKERNEL_QueryProcessDB
 **
 **  Query a process database for the current usage of a particular record type.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to a gckKERNEL object.
 **
 **      gctUINT32 ProcessID
 **          Process ID used to identify the database.
 **
 **      gctBOOL LastProcessID
 **          gcvTRUE if searching for the last known process ID.  gcvFALSE if
 **          we need to search for the process ID specified by the ProcessID
 **          argument.
 **
 **      gceDATABASE_TYPE Type
 **          Type of the record to query.
 **
 **  OUTPUT:
 **
 **      gcuDATABASE_INFO *Info
 **          Pointer to a variable that receives the requested information.
 */
gceSTATUS
gckKERNEL_QueryProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID,
                         gctBOOL LastProcessID, gceDATABASE_TYPE Type, gcuDATABASE_INFO *Info)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gcePOOL vidMemPool;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d Type=%d Info=%p",
                   Kernel, ProcessID, Type, Info);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);
    gcmkVERIFY_ARGUMENT(Info != gcvNULL);

    /* Deocde pool. */
    vidMemPool = (Type & gcdDB_VIDEO_MEMORY_POOL_MASK) >> gcdDB_VIDEO_MEMORY_POOL_SHIFT;

    Type &= gcdDATABASE_TYPE_MASK;

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, LastProcessID, &database));

    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, database->counterMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Get pointer to counters. */
    switch (Type) {
    case gcvDB_VIDEO_MEMORY:
        if (vidMemPool != gcvPOOL_UNKNOWN) {
            gckOS_MemCopy(&Info->counters,
                          &database->vidMemPool[vidMemPool],
                          gcmSIZEOF(database->vidMemPool[vidMemPool]));
        } else {
            gckOS_MemCopy(&Info->counters,
                          &database->vidMem,
                          gcmSIZEOF(database->vidMem));
        }
        break;

    case gcvDB_NON_PAGED:
        gckOS_MemCopy(&Info->counters, &database->nonPaged, gcmSIZEOF(database->vidMem));
        break;

    case gcvDB_CONTIGUOUS:
        gckOS_MemCopy(&Info->counters, &database->contiguous, gcmSIZEOF(database->vidMem));
        break;

    case gcvDB_IDLE:
        Info->time = Kernel->db->idleTime;
        Kernel->db->idleTime = 0;
        break;

    case gcvDB_MAP_MEMORY:
        gckOS_MemCopy(&Info->counters, &database->mapMemory, gcmSIZEOF(database->mapMemory));
        break;

    default:
        break;
    }

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, database->counterMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, database->counterMutex));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_FindHandleDatabase(gckKERNEL Kernel, gctUINT32 ProcessID,
                             gctPOINTER *HandleDatabase,
                             gctPOINTER *HandleDatabaseMutex)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d", Kernel, ProcessID);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    *HandleDatabase = database->handleDatabase;
    *HandleDatabaseMutex = database->handleDatabaseMutex;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_DumpProcessDB(gckKERNEL Kernel)
{
    gcsDATABASE_PTR database;
    gctINT i, pid;
    gctUINT8 name[24];
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Kernel=%p", Kernel);

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));
    acquired = gcvTRUE;

    gcmkPRINT("**************************\n");
    gcmkPRINT("***  PROCESS DB DUMP   ***\n");
    gcmkPRINT("**************************\n");

    gcmkPRINT_N(8, "%-8s%s\n", "PID", "NAME");
    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(Kernel->db->db); ++i) {
        for (database = Kernel->db->db[i]; database != gcvNULL; database = database->next) {
            pid = database->processID;

            gcmkVERIFY_OK(gckOS_ZeroMemory(name, gcmSIZEOF(name)));

            gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

            gcmkPRINT_N(8, "%-8d%s\n", pid, name);
        }
    }

    status = gcvSTATUS_OK;

OnError:
    /* Release the database mutex. */
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));

    gcmkFOOTER();
    return status;
}

static void
_DumpCounter(gcsDATABASE_COUNTERS *Counter, gctCONST_STRING Name)
{
    gcmkPRINT("%s:", Name);
    gcmkPRINT("  Currently allocated : %10lld", Counter->bytes);
    gcmkPRINT("  Maximum allocated   : %10lld", Counter->maxBytes);
    gcmkPRINT("  Total allocated     : %10lld", Counter->totalBytes);
}

gceSTATUS
gckKERNEL_DumpVidMemUsage(gckKERNEL Kernel, gctINT32 ProcessID)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gcsDATABASE_COUNTERS *counter;
    gctUINT32 i = 0;

    static gctCONST_STRING vidmemTypes[] = {
        "GENERIC",
        "INDEX",
        "VERTEX",
        "TEXTURE",
        "RENDER_TARGET",
        "DEPTH",
        "BITMAP",
        "TILE_STATUS",
        "IMAGE",
        "MASK",
        "SCISSOR",
        "HIERARCHICAL_DEPTH",
        "ICACHE",
        "TXDESC",
        "FENCE",
        "TFBHEADER",
        "COMMAND",
    };

    gcmkHEADER_ARG("Kernel=%p ProcessID=%d", Kernel, ProcessID);

    gcmSTATIC_ASSERT(gcmCOUNTOF(vidmemTypes) == gcvVIDMEM_TYPE_COUNT,
                     "Video memory type mismatch");

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(Kernel, ProcessID, gcvFALSE, &database));

    gcmkPRINT("VidMem Usage (Process %d):", ProcessID);

    /* Get pointer to counters. */
    counter = &database->vidMem;

    _DumpCounter(counter, "Total Video Memory");

    for (i = 0; i < gcvVIDMEM_TYPE_COUNT; i++) {
        counter = &database->vidMemType[i];

        _DumpCounter(counter, vidmemTypes[i]);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}
