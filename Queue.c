/* Filename: Queue.c
 * Created by: Jason Chu
 * An implementation of the abstract data type Queue. Contains functionalities of createQ (create a Queue set to null), addQ to add
 * an element to the queue, isEmptyQ that checks to see if the queue has any elements, headQ that gives the value stored in the head of the
 * queue, removeQ that removes the head queue element, and destroyQ that frees any storage used by the queue.
 */

#include "Queue.h"
#include <stdlib.h>

// The internal representation of a Queue is a headless, singly-linked,
// circular list.  The Queue data type is a pointer to the LAST node in
// the list (or NULL if the queue is empty).


// type definition of a node in a queue
// Field char *value is a pointer to the string stored in the node
// Field node *next is the next node in the queue
typedef struct node
{           // List node
    char *value;
    struct node *next;
} Node;


// sets the pointer q to NULL
int createQ (Queue *q)
{
    *q = NULL;
    return true;
}

// adds a node with value s to the tail of the queue q using malloc
int addQ (Queue *q, char *s)
{
    // HOW ABOUT WHEN Q is EMPTY?
    Node *nxt = malloc(sizeof(Node));
    if (nxt == NULL)
        return false;
    (*nxt).value = s;
    if (isEmptyQ(q))
    {
        (*nxt).next = nxt;
    }
    else
    {
        (*nxt).next = (*q)->next;
        (**q).next = nxt;
    }
    // q pointer to last element, which is now Node nxt
    *q = nxt;
    return true;
}

// checks to see if queue q is empty
int isEmptyQ (Queue *q)
{
    if (*q == NULL)
        return true;
    else
        return false;
}


// stores the value of the head of the queue to s if queue is not empty, returns false if queue is empty
int headQ (Queue *q, char **s)
{
    if (isEmptyQ(q))
    {
        return false;
    }
    else
    {
        (*s) = (*q)->next->value;
        return true;
    }
}

// removes the head of the queue if not empty, returns false if empty
int removeQ (Queue *q, char **s)
{
    if (isEmptyQ(q))
    {
        return false;
    }
    else
    {
        headQ(q, s);
        Node *head = (*q)->next;
        //case of removing a single node
        if (head == *q)
        {
            *q = NULL;
        }
        else
        {
            Node *nxt = head->next;
            (**q).next = nxt;
        }
        free(head);
        return true;
    }
}

// removes all elements in the queue and frees storage
int destroyQ (Queue *q)
{
    while (!isEmptyQ(q))
    {
        char *s = "x";
        removeQ(q, &s);
        
        /*Node *temp = (*q)->next;
         *q = temp->next;
         free(temp);*/
    }
    return true;
}
