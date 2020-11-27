#include <stdio.h>
#include <assert.h>

#include "cvector.h"
#include "clist.h"


static void static_vectors(void)
{
    int array[64];
    int values[] = { 0, 1, 2, 3 };
    cvector_t v; /* stack allocated vector */

    array[0] = 0;
    array[1] = 1;

    /*
       Here vector will handle the first 32 ints of array, starting size is 2.
       We can add max 30 elements. If you push the 33th element, the callback
       error will be called (no error codes in cvector).
       You can call cvector_set_callback passing NULL, to avoid error callbacks
    */

    cvector_init_ext(&v, array, sizeof(int), 32U, 2U, CVECTOR_DATA);

    assert(!cvector_empty(&v));
    assert(cvector_size(&v) == 2U);
    /* To init an empty array (usual case, just pass 0U) */

    assert(0 == CVECTOR_FRONT(&v, int));
    assert(1 == CVECTOR_BACK(&v, int));

    cvector_push_back(&v, &values[2]); /* push back 2 */

    cvector_insert(&v, 3U, &values[3]); /* push_back, insert after last */

    assert(1 == CVECTOR_ELEM(&v, 1U, int)); /* see also cvector_get_data() */

    cvector_pop_back(&v);

    assert(cvector_size(&v) == 3U);

    /* Here we extend the array size, preserving old data */
    cvector_init_ext(&v, array, sizeof(int), 64U, cvector_size(&v), CVECTOR_DATA);

    assert(0 == CVECTOR_ELEM(&v, 0U, int));
    assert(1 == CVECTOR_ELEM(&v, 1U, int));
    assert(2 == CVECTOR_ELEM(&v, 2U, int));

    cvector_destroy(&v);
}

static void dynamic_vectors(void)
{
    cvector_t* pv;
    int a[4] = { 0, 1, 2, 3 };
    int value = 42;

    /* let cvector decide the initial size of array. Tune according to usage */
    pv = cvector_new(sizeof(int), CVECTOR_DEFAULT_LEN, CVECTOR_DATA);

    cvector_push_back(pv, &value);

    /* insert in head the a array. You can easily concat vectors */
    cvector_insert_n(pv, 0U, 4U, &a[0], 0);

    /* Here we append 3 times zero (see last parameter changed) */
    cvector_insert_n(pv, 0U, 3U, &a[0], 1);

    /* resize with 42 on all added elements. Pass NULL for uninitialized mem */
    cvector_resize(pv, 16U, &value);
    assert(16U == cvector_size(pv));

    /* remove [...,4,5,6,...] form vector */
    cvector_erase(pv, 4U, 3U);
    assert(13U == cvector_size(pv));

    cvector_clear(pv);
    assert(cvector_empty(pv));

    /* reserve AT LEAST space for 32 INT */
    cvector_reserve(pv, 32U);

    cvector_delete(&pv);
    assert(pv == NULL);
}

static void deleted_vectors(void)
{
    const int values[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int* pointers[4];
    cvector_t v, vs;
    int i;

    /*
       Initialize vector creating space for EXACTLY 8 pointers, that will be
       automatically freed (see last paramater)
    */
    cvector_init(&v, sizeof(int*), 8U, CVECTOR_FREE_PTR);

    /* Also static vectors can contain pointers to free automatically */
    cvector_init_ext(&vs, pointers, sizeof(int*), 4U, 0U, CVECTOR_FREE_PTR);

    for (i = 0; i < 8; i++) {
        int* const p_num = (int*)malloc(sizeof(int));
        *p_num = values[i];
        if (i < 4) {
            cvector_push_back(&v, &p_num); /* store pointers into vector */
        }
        else {
            cvector_push_back(&vs, &p_num); /* or into static vector */
        }
    }

    assert(cvector_size(&v) == 4U);
    assert(cvector_size(&v) == 4U);

    for (i = 0; i < 8; i++) {
        const int* p;

        if (i < 4) {
            /* First way to get stored data */
            p = CVECTOR_ELEM(&v, i, int*);
        }
        else {
            /* Second way to get stored data */
            p = *((const int**)cvector_get_data(&vs, (size_t)i - 4U));
        }
        assert(values[i] == *p);
    }

    cvector_destroy(&v);
    cvector_destroy(&vs);

    /* No memory leak, pointers have been freed automatically! */
}

static void standard_lists(void)
{
    const int values[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    clist_node_t* node;
    clist_t l;
    int i, ok;

    ok = clist_init(&l, sizeof(int), CLIST_PAYLOAD_IGNORE);
    assert(EXIT_SUCCESS == ok);

    assert(EXIT_SUCCESS == clist_push_back(&l, &values[2]));

    assert(EXIT_SUCCESS == clist_push_front(&l, &values[0]));

    assert(EXIT_SUCCESS == clist_insert(&l, 1, &values[1]));

    clist_erase(&l, 0U, 1U); /* remove head element */

    assert(!clist_empty(&l));
    assert(clist_size(&l) == 2U);

    for (i = 0; i < 8 && ok == EXIT_SUCCESS; i++) {
        ok = clist_push_back(&l, &values[i]); /* store values */
    }

    /* first way to get the stored value from the list */
    assert(0 == *CLIST_PTR(&l, 2U, int));

    /*
       second way, starting from node. To navigate nodes, see functions:
       clist_head(), clist_tail(), clist_go() or use ->prev/->next pointers
    */
    node = clist_go(&l, 3U); /* Go to 4th node */
    assert(node != NULL);

    /* Given a node, the two ways to get original value */
    assert(1 == *((int*)clist_get_from_node(node)));
    assert(1 == *CLIST_NODE_PTR(node, int));

    assert(ok == EXIT_SUCCESS);
    assert(clist_size(&l) == 10U);

    clist_destroy(&l);
}

static void deleted_lists(void)
{
    const int values[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    clist_t* pl;
    clist_node_t* node;
    int i;

    /* last parameters tells to free pointers */
    pl = clist_new(sizeof(int*), CLIST_PAYLOAD_FREE);
    assert(pl != NULL);

    for (i = 0; i < 8; i++) {
        int* const p_num = (int*)malloc(sizeof(int));
        *p_num = values[i];
        clist_push_back(pl, &p_num); /* store pointers into list */
    }

    assert(clist_size(pl) == 8U);

    for (i = 0, node = clist_head(pl); node != NULL; i++, node = node->next) {
        int* const p = *CLIST_NODE_PTR(node, int*);
        assert(values[i] == *p);
    }

    clist_delete(&pl);

    /* No memory leak here */
}

static void lists(void)
{
    standard_lists(); /* normal lists */
    deleted_lists();  /* list with pointers that will be automatically freed */
}

static void vectors(void)
{
    static_vectors();  /* Test vectors with user memory, no malloc */
    dynamic_vectors(); /* Classic growable vectors */
    deleted_vectors(); /* Growable vectors that free pointer elements */
}

int main(void)
{
    vectors();
    lists();
    puts("All ok");
    return 0;
}
