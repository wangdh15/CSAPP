/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  (queue_t *)malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if(q){   // 如果内存申请成功，则初始化q的各个参数
      q -> head = NULL;
      q -> size = 0;
      q -> tail = NULL;
    }
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    if(q){
        while(q -> head){
          list_ele_t *t = q -> head -> next;
          free(q -> head -> value);
          free(q -> head);
          q -> head = t;
        }
    }
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if(!q) return false;
    list_ele_t *newh;
    /* What should you do if the q is NULL? */
    newh = (list_ele_t *)malloc(sizeof(list_ele_t));
    if(!newh){
      return false;
    }
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    newh -> value = (char *)malloc(strlen(s) + 1);  // 需要多申请一个字节存放\0
    if(! (newh -> value)){
      // 字符串申请内存失败，则也需要将申请的节点内存释放
      free(newh);
      return false;
    }else{
      // 字符串内存申请成功，则将value拷贝到节点中，返回true
      strcpy(newh -> value, s);    
      newh -> next = q -> head;
      q -> head = newh;
      if(!(q -> tail)) q -> tail = newh; // 如果当前链表尾空的话，则将tail指针也指向它。
      q -> size ++;
      return true;
    }
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if(!q) return false;
    list_ele_t *newt = (list_ele_t *)malloc(sizeof(list_ele_t));
    if(!newt) return false;
    newt -> value = (char *)malloc(strlen(s) + 1);
    if(!(newt -> value)){
      free(newt);
      return false;
    }else{
      strcpy(newt -> value, s);
      newt -> next = NULL;
      if(q -> tail){  // 链表不为空的情况
        q -> tail -> next = newt;
        q -> tail = newt;
      }else{  // 链表为空的情况
        q -> tail = newt;
        q -> head = newt;
      }
      q -> size ++;
    }
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
    if(!q || !(q -> head)) return false;
    list_ele_t *t = q -> head -> next;
    if(sp){  // 将删除的节点的字符串拷贝到sp指针中
      if(q -> head -> value){
        int i = 0;
        for(; i < strlen(q -> head -> value) && i < bufsize - 1; i ++){
          sp[i] = (q -> head -> value)[i];
        }
        sp[i] = '\0';
      }else{
        if(bufsize > 0) sp[0] = '\0';
      }
    }
    free(q -> head -> value); // 释放节点中的字符串
    free(q -> head); // 释放头结点
    q -> head = t;
    if(q -> size == 1) q -> tail = t;  // 如果只有一个元素的话，也需要将tail指针指向空
    q -> size --;  // size变量减1
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if(q) return q -> size;
    else return 0;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if(!q ||  q -> size == 0) return ;
    list_ele_t *cur = q -> head;
    list_ele_t *ne = cur -> next;
    while(ne){
      list_ele_t *nne = ne -> next;
      ne -> next = cur;
      cur = ne;
      ne = nne;
    }
    list_ele_t *t = q -> head;
    q -> head = q -> tail;
    q -> tail = t;
    q -> tail  -> next = NULL;
}

