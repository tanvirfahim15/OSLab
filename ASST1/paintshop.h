/*
 * **********************************************************************
 * You are free to add anything you think you require to this file
 */

#ifndef _PAINTSHOP_H_
#define _PAINTSHOP_H_

#endif

typedef struct paintcan paint_can;
typedef struct semaphore _semaphore;
struct Order {
  struct paintcan *can;
  struct semaphore *ready;
};


int remaining_customers;

struct Order *done_can[10000];

struct Order *order_buffer[10000];

_semaphore *access_orders;
_semaphore *full_order;
_semaphore *empty_order;

_semaphore *access_done;
_semaphore *order_ready;

_semaphore *remaining_customers_sem;

_semaphore *access_tints[10000];

