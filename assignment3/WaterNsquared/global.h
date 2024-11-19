









































/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

/*  This file contains the declaration of the GlobalMemory 
structure and the maximum number of molecules allowed 
by the program. */

#define LOCK_PER_PROC
// #define MAXLCKS	4096
#define MAXLCKS 2048

struct GlobalMemory {
    lock_t IOLock;
    lock_t IndexLock;
    lock_t IntrafVirLock;
    lock_t InterfVirLock;
    lock_t FXLock;
    lock_t FYLock;
    lock_t FZLock;
    lock_t KinetiSumLock;
    lock_t PotengSumLock;
    lock_t (MolLock[MAXLCKS]);
    barrier_t start;
    barrier_t InterfBar;
    barrier_t PotengBar;
    int Index;
    double VIR;
    double SUM[3];
    double POTA, POTR, POTRF;
    unsigned long createstart,createend,computestart,computeend;
    unsigned long trackstart, trackend, tracktime;
    unsigned long intrastart, intraend, intratime;
    unsigned long interstart, interend, intertime;
};

extern struct GlobalMemory *gl;

