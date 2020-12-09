/*
clang -Ofast -ocl.exe -DCLIST cl_bench.c
clang -Ofast -oul.exe cl_bench.c

gcc -Ofast -ocl -DCLIST cl_bench.c
gcc -Ofast -oul cl_bench.c

cl /O2 /Fecl -DCLIST cl_bench.c
cl /O2 /Feul cl_bench.c

measure execution times of both exe on your env
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef CLIST
#include "clist.h"
#else
#include <string.h>
#include "utlist.h"
typedef struct element {
    int data;
    struct element *next;
    struct element *prev;
} node_t;
#endif

#define MAIN_LOOP (1 << 17)
#define SEARCH_LOOP (MAIN_LOOP / 2)

int main(void)
{
#ifdef CLIST
    clist_t* pl;
    clist_node_t* elt;
#else
    node_t* head = NULL;
    node_t *elt, *tmp, *tail;
    size_t j;
#endif
    size_t i;

    srand(time(NULL));

#ifdef CLIST
    pl = clist_new(sizeof(int), CLIST_PAYLOAD_IGNORE);
#endif

    for (i = 0; i < MAIN_LOOP; i++) {
#ifdef CLIST
        clist_push_back(pl, &i);
#else
        elt = malloc(sizeof(node_t));
        memcpy(&elt->data, &i, sizeof(int));
        DL_APPEND(head, elt);
#endif
    }

#ifndef CLIST
    tail = elt;
#endif

    for (i = 0; i < SEARCH_LOOP; i++) {
        const size_t pos = (size_t)rand() % MAIN_LOOP;
        int x;

#ifdef CLIST
        x = *CLIST_PTR(pl, pos, int);
#else
        if (pos < MAIN_LOOP / 2) {
            elt = head;
            for (j = 0; j < pos; j++, elt = elt->next) ;
        }
        else {
            elt = tail;
            for (j = MAIN_LOOP - pos - 1; j > 0; j--, elt = elt->prev) ;
        }
        x = elt->data;
#endif

        if (x != pos) {
            puts("impossible");
            exit(-1);
        }
    }

#ifdef CLIST
        clist_delete(&pl);
#else
        DL_FOREACH_SAFE(head,elt,tmp) {
            DL_DELETE(head,elt);
            free(elt);
        }
#endif

    return 0;
}
