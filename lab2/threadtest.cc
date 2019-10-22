// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(_int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times *** vip %d vips\n", (int) which, num,(int)currentThread->getvip());
        currentThread->Yield();
    }
}

void
p0(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times *** vip %d vips\n", (int) which, num,(int)currentThread->getvip());
        currentThread->Yield();

    }
}

void
p1(int which1)
{
   int num1;
    
    for (num1 = 0; num1 < 5; num1++) {
	printf("*** thread %d looped %d times *** vip %d vips\n", (int) which1, num1,(int)currentThread->getvip());
        //currentThread->Yield();
    }
}

void
p3(int which13)
{
   int num13;
    
    for (num13 = 0; num13 < 5; num13++) {
	printf("*** thread %d looped %d times *** vip %d vips\n", (int) which13, num13,(int)currentThread->getvip());
        //currentThread->Yield();
    }
}

void
p2(int which2)
{
   int num2;
    
    for (num2 = 0; num2 < 5; num2++) {
	printf("*** thread %d looped %d times *** vip %d vips\n", (int) which2, num2,(int)currentThread->getvip());
        
if(num2 == 0) {Thread *t3 = new Thread("fofcdrked thread",1);
		t3->Fork(p3,4);}currentThread->Yield();
    }
	
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//---------------------a voi-------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");
    Thread *t = new Thread("forked thread",3);
    t->Fork(p0, 1);
    Thread *t1 = new Thread("fofrked thread",5);
    t1->Fork(p1,2);
    Thread *t2 = new Thread("fofcrked thread",4);
    t2->Fork(p2,3);
}

