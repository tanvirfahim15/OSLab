#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "paintshop_driver.h"

#include "paintshop.h"


/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_paint()
 *
 * Takes one argument specifying the can to be filled. The function
 * makes the can available to staff threads and then blocks until the staff
 * have filled the can with the appropriately tinted paint.
 *
 * The can itself contains an array of requested tints.
 */ 

void order_paint(struct paintcan *can)
{
    P(empty_order);
    P(access_orders);
    int i;
    for(i = 0; i < NCUSTOMERS; i++)
    {
        if(order_buffer[i] == NULL)
        {
            order_buffer[i] = can;
            break;
        }
    }
    V(access_orders);
    V(full_order);

    int found = 0;
    while(!found) {
	P(order_ready);
	P(access_done);
	int i;
	for(i = 0; i < NCUSTOMERS; i++) {
	    if(can == (struct paintcan *) done_can[i]) {
		done_can[i] = NULL;
		found = 1;
		break;
	    }
	}
	V(access_done);
	if(!found) {
	    V(order_ready);
	    thread_yield();
	}
    }
}



/*
 * go_home()
 *
 * This function is called by customers when they go home. It could be
 * used to keep track of the number of remaining customers to allow
 * paint shop staff threads to exit when no customers remain.
 */

void go_home()
{

    remaining_customers--;
}


/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY PAINT SHOP STAFF THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it records the details, and returns a
 * pointer to something representing the order.
 *
 * The return pointer type is void * to allow freedom of representation
 * of orders.
 *
 * The function can return NULL to signal the staff thread it can now
 * exit as their are no customers nor orders left. 
 */
 
void * take_order()
{
    
    
    void *order;
    while(1){
    if(remaining_customers == 0)
    {
	return NULL;
    }

    bool flag = false;
    P(full_order);
    P(access_orders);
    int i;
    for(i = 0; i < NCUSTOMERS; i++)
    {
        if(order_buffer[i] != NULL)
        {
	    order = (void *)order_buffer[i];
            order_buffer[i] = NULL;
	    flag = true;
            break;
        }
    }

    V(access_orders);
    V(full_order);
    if(flag){
        V(empty_order);
	break;    
    }

    }
    return order;
}


/*
 * fill_order()
 *
 * This function takes an order generated by take order and fills the
 * order using the mix() function to tint the paint.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE TINTS
 * IT NEEDS TO USE TO FILE THE ORDER.
 */
void wait(struct paintcan *c){
    int i,col;
    for(i = 0; i < PAINT_COMPLEXITY; i++) {
	col = c->requested_colours[i] - 1;
      	if(col > 0)
	    P(tintSem[col]);   
    }
}
void signal(struct paintcan *c){
    int i,col;   
    for(i = 0; i < PAINT_COMPLEXITY; i++) {
	col = c->requested_colours[i] - 1;
	if(col > 0)
	    V(tintSem[col]);   
    }
}
void fill_order(void *v)
{
    wait(v);
    mix(v);
    signal(v);
}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to the waiting customer.
 */

void serve_order(void *v)
{
    int i;
    P(access_done);
    for(i = 0; i < NCUSTOMERS; i++) {
	if(done_can[i] == NULL) {
	    done_can[i] = v;
	    break;
        }
    }
    V(access_done);
    V(order_ready);
}



/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * paintshop_open()
 *
 * Perform any initialisation you need prior to opening the paint shop to
 * staff and customers
 */

void paintshop_open()
{
    remaining_customers = NCUSTOMERS;
    
    access_orders = sem_create("access_orders", 1);
    empty_order = sem_create("empty_order", NCUSTOMERS);
    full_order = sem_create("full_order", 0);
    
    access_done   = sem_create("access_done", 1);
    order_ready   = sem_create("order_ready", 0);
	

    int i;
    for(i = 0 ; i < NCOLOURS ; i++)
        tintSem[i] = sem_create("tint sem", 1);

}

/*
 * paintshop_close()
 *
 * Perform any cleanup after the paint shop has closed and everybody
 * has gone home.
 */

void paintshop_close()
{
sem_destroy(access_orders);
sem_destroy(empty_order);
sem_destroy(full_order);
sem_destroy(access_done);
sem_destroy(order_ready);

    int i;
    for(i = 0 ; i < NCOLOURS ; i++)
        sem_destroy(tintSem[i]);

}
