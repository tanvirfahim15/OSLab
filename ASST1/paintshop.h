/*
 * **********************************************************************
 * You are free to add anything you think you require to this file
 */

#ifndef _PAINTSHOP_H_
#define _PAINTSHOP_H_

#endif

typedef struct paintcan paint_can;
typedef struct semaphore _semaphore;

int remaining_customers;

void *done_can[10000];

paint_can *order_buffer[10000];

_semaphore *access_orders;
_semaphore *full_order;
_semaphore *empty_order;

_semaphore *access_done;
_semaphore *order_ready;


_semaphore *access_tints[10000];

