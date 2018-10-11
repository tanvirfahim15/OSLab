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

/* buffers */

void *done_can[NCUSTOMERS];

paint_can *order_buffer[NCUSTOMERS];

_semaphore *access_orders;
_semaphore *full_order;
_semaphore *empty_order;

_semaphore *access_tints;

_semaphore *access_done;
_semaphore *order_ready;

