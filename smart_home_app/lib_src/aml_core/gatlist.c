/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatlist.h"

void gatListHeadInit(struct gatListHead *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */

void __gatListAdd(struct gatListHead *newnode,
                  struct gatListHead *prev,
                  struct gatListHead *next)
{
    next->prev = newnode;
    newnode->next = next;
    newnode->prev = prev;
    prev->next = newnode;
}

/**
 * gat_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void gatListAdd(struct gatListHead *newnode, struct gatListHead *head)
{
    __gatListAdd(newnode, head, head->next);
}

/**
 * gatListAddTail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void gatListAddTail(struct gatListHead *newnode, struct gatListHead *head)
{
    __gatListAdd(newnode, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __gatListDel(struct gatListHead * prev, struct gatListHead * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * gat_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: gatListEmpty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
void gatListDel(struct gatListHead *entry)
{
    __gatListDel(entry->prev, entry->next);
    entry->next = entry;
    entry->prev = entry;
}

/**
 * gatListEmpty - tests whether a list is empty
 * @head: the list to test.
 */
int gatListEmpty(const struct gatListHead *head)
{
    return head->next == head;
}

/**
 * gatListEmptyCareful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using gatListEmptyCareful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is gat_list_del_init(). Eg. it cannot be used
 * if another CPU could re-gat_list_add() it.
 */
int gatListEmptyCareful(const struct gatListHead *head)
{
    struct gatListHead *next = head->next;
    return (next == head) && (next == head->prev);
}
