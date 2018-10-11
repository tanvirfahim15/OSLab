/*
 * **********************************************************************
 * You are free to add anything you think you require to this file
 */

#ifndef _PAINTSHOP_H_
#define _PAINTSHOP_H_

#endif
struct queue *orders;

struct semaphore *access_orders;
struct semaphore *access_done;
struct semaphore *order_ready;
struct semaphore *access_tints;

void *done_can[NCUSTOMERS];

int remaining_customers;

struct queue {
	int size;
	int nextwrite;	// next element to write to (was head)
	int nextread;	// next element to read from (was tail)
	void **data;
};
struct queue *
q_create(int size);

int
q_grow(struct queue *q, int targetsize);

int
q_addtail(struct queue *q, void *ptr);

int
q_empty(struct queue *q);

void *
q_remhead(struct queue *q);

void
q_destroy(struct queue *q);
