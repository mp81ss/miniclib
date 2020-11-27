/**
 * @file      clist.h
 * @version   1.0
 * @brief     CList header-only list library for C89 language
 * @date      Wed Oct 14 18:17:25 2020
 * @author    Michele Pess
 * @copyright BSD-3-Clause
 *
 * This file offers a traditional C++-like double-linked list to C.
 * It is entirely self-contained in this file and offers the traditional
 * functions of a list plus the possibility to free pointers on deletion.
 * The CList APIs are secure, all pointers are NULL-checked and all indexes
 * are checked against out-of-bound errors. If something goes wrong an error is
 * returned or nothing is done
 */

#ifndef CLIST_H_
#define CLIST_H_

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name Init flags
 * These 2 macros are the values to be passed to clist_init() and
 * clist_new() to decide what to do with discarded data. Basically,
 * in one case the elements are simply discarded, while in the other case,
 * the pointers are passed to \a free before being discarded
 * @{
 */
#define CLIST_PAYLOAD_IGNORE 0U /**< Elements are symply discarded */
#define CLIST_PAYLOAD_FREE   1U /**< Elements will be freed before removal */
/**
 * @}
 */

/**
 * @def CLIST_NODE_PTR
 * Helper macro that cast the pointer to value stored on node \a n to type
 * <a>t*</a>
 * @note This macro is safe since just calls clist_get_from_node() and cast
 *       the pointer. No pointer deferentiation occurs
 */
#define CLIST_NODE_PTR(n, t) ((t*)clist_get_from_node(n))

/**
 * @def CLIST_PTR
 * Helper macro that cast the pointer to value stored on node at index \a i
 * to type <a>t*</a>
 * @note This macro is safe since just calls clist_get() and cast the pointer.
 *       No pointer deferentiation occurs
 */
#define CLIST_PTR(l, i, t) ((t*)clist_get((l), (i)))


typedef struct vnut_node_t {
    struct vnut_node_t* next;
    struct vnut_node_t* prev;
} clist_node_t;

typedef struct {
    clist_node_t* head;
    clist_node_t* tail;
    clist_node_t* rul;
    clist_node_t* pkb;
    size_t type_size;
    size_t size;
    size_t ruly;
    size_t zskb;
    unsigned int flags;
} clist_t;

/**
 * @typedef clist_foreach_cb_t
 * Prototype for callback function to pass to clist_foreach()
 */
typedef void (*clist_foreach_cb_t)(void*);

/**
 * @typedef clist_filter_cb_t
 * Prototype for callback function to pass to clist_filter()
 */
typedef int (*clist_filter_cb_t)(void*);


/**
 * @brief Initialize a list
 * @param[in] list The list to initialize
 * @param[in] type_size The size of type of elements of the list
 *            (ex.: sizeof(int))
 * @param[in] dynamic This parameter decides if the data stored on the nodes
 *            must be freed (passed to the \a free function) before deletion.
 *            Pass #CLIST_PAYLOAD_FREE to free pointers. In this case,
 *            \a type_size must be equal to <a>sizeof(void*)</a>. If you want
 *            to simply discard elements, pass #CLIST_PAYLOAD_IGNORE
 * @retval EXIT_SUCCESS If list is correctly initialized
 * @retval EXIT_FAILURE If \a list is NULL or \a dynamic has an invalid value
 * @note Do not call this function on an already initialized list. Always call
 *       clist_destroy() or clist_delete() and then reinitialize
 */
static int clist_init(clist_t* list, size_t type_size, unsigned int dynamic)
{
    int ok = EXIT_FAILURE;
    if ((list != NULL) && (type_size > 0U) &&
        ((dynamic == CLIST_PAYLOAD_IGNORE) || (dynamic == CLIST_PAYLOAD_FREE))
        && ((dynamic == CLIST_PAYLOAD_IGNORE) || (type_size == sizeof(void*))))
    {
        list->head = list->tail = list->pkb = NULL;
        list->type_size = type_size;
        list->size = list->zskb = 0U;
        list->flags = dynamic & 1U;
        ok = EXIT_SUCCESS;
    }
    return ok;
}

/**
 * @brief Allocate and initialize a new list
 * @param[in] type_size See explanation on clist_init()
 * @param[in] dynamic See explanation on clist_init()
 * @return A pointer to a newly allocated list, or NULL if not enough memory or
 *         dynamic has an invalid value
 * @note The pointer returned by this function must be typically passed to
 *       clist_delete() when the list in no longer needed
 */
static clist_t* clist_new(size_t type_size, unsigned int dynamic) {
    clist_t* list = (clist_t*)malloc(sizeof(clist_t));
    if ((list != NULL)
        && (clist_init(list, type_size, dynamic) != EXIT_SUCCESS))
    {
        free(list);
        list = NULL;
    }
    return list;
}

/**
 * @brief Return the size of the list
 * @param[in] list The list to operate with
 * @return The size of the list or zero if \a list is NULL
 */
static size_t clist_size(const clist_t* list) {
    return (list != NULL) ? list->size : 0U;
}

/**
 * @brief Tell if list is empty
 * @param[in] list The list to operate with
 * @retval 1 if \a list is NULL or it is empty
 * @retval 0 if \a list is not NULL and has some elements
 */
static int clist_empty(const clist_t* list) {
    return ((list != NULL) && (list->size > 0U)) ? 0 : 1;
}

/**
 * @brief Return the head of the list
 * @param[in] list The list to get head
 * @return The first node of the list. This is NULL if the list is empty or
 *         \a list is NULL
 */
static clist_node_t* clist_head(clist_t* list) {
    return (list != NULL) ? list->head : NULL;
}

/**
 * @brief Return the tail of the list
 * @param[in] list The list to get tail
 * @return The last node of the list. This is NULL if the list is empty or
 *         \a list is NULL
 */
static clist_node_t* clist_tail(clist_t* list) {
    return (list != NULL) ? list->tail : NULL;
}

/**
 * @brief Return the node of index \a idx in \a list
 * @param[in] list The list to operate with
 * @param[in] idx The index of the list. Must be less than the list size
 * @return The node at index \a idx or NULL on errors (\a list is NULL or
 *         \a idx is >= size of list)
 */
static clist_node_t* clist_go(clist_t* list, size_t idx) {
    clist_node_t* node = NULL;

    if (list != NULL) {
        const size_t len = list->size;

        if (idx < len) {

            if (idx == 0U) {
                node = list->head;
            }
            else if (idx == 1U) {
                node = list->head->next;
            }
            else if (idx == (len - 1U)) {
                node = list->tail;
            }
            else if (idx == (len - 2U)) {
                node = list->tail->prev;
            }
            else {
                int direction;
                size_t steps;

                if ((list->flags & 2U) != 0U) {
                    const size_t cidx = list->ruly;

                    if (idx >= cidx) {
                        if ((idx - cidx) <= (len - idx - 1U)) {
                            node = list->rul;
                            steps = idx - cidx;
                            direction = 0;
                        }
                        else {
                            node = list->tail;
                            steps = len - idx - 1U;
                            direction = -1;
                        }
                    }
                    else {
                        if (idx <= (cidx - idx)) {
                            node = list->head;
                            steps = idx;
                            direction = 0;
                        }
                        else {
                            node = list->rul;
                            steps = cidx - idx;
                            direction = -1;
                        }
                    }
                }
                else {
                    if (idx > (len / 2U)) {
                        node = list->tail;
                        steps = len - idx - 1U;
                        direction = -1;
                    }
                    else {
                        node = list->head;
                        steps = idx;
                        direction = 0;
                    }
                }

                if (direction == 0) {
                    while (steps-- > 0U) {
                        node = node->next;
                    }
                }
                else {
                    while (steps-- > 0U) {
                        node = node->prev;
                    }
                }

                list->rul = node;
                list->ruly = idx;
                list->flags |= 2U;
            }
        }
    }

    return node;
}

/**
 * @brief Gives the pointer to the value stored in the node at position \a idx
 *        inside \a list
 * @param[in] list The list to operate with
 * @param[in] idx The index of the element to retrieve
 * @return A pointer to the data contained by node at index \a idx or NULL
 *         if \a list is NULL or \a idx >= size of list
 */
static void* clist_get(clist_t* list, size_t idx) {
    clist_node_t* const p = clist_go(list, idx);
    return (p != NULL) ? (p + 1) : NULL;
}

/**
 * @brief Return a pointer to the value stored on \a node
 * @param[in] node The node to extract value from
 * @return The value of \a node or NULL if \a node is NULL
 */
static void* clist_get_from_node(clist_node_t* node) {
    return (node != NULL) ? (node + 1) : NULL;
}

/**
 * @brief Set the value stored in the node at position \a idx inside \a list
 * @param[in] list The list to operate with
 * @param[in] idx The index of the element to set
 * @param[in] payload The new value of node
 */
static void clist_set(clist_t* list, size_t idx, const void* payload) {
    if (payload != NULL) {
        clist_node_t* const p = clist_go(list, idx);
        if (p != NULL) {
            memcpy(p + 1, payload, list->type_size);
        }
    }
}

/**
 * @brief Set \a payload as value of \a node
 * @param[in] node The node to update
 * @param[in] payload The new value of \a node
 * @param[in] type_size The size of type of the list that contains or will
 *            contain the node
 */
static void clist_set_to_node(clist_node_t* node,
                              const void* payload,
                              size_t type_size)
{
    if ((node != NULL) && (payload != NULL) && (type_size > 0U)) {
        memcpy(node + 1, payload, type_size);
    }
}

static void vnut_cl_insert_node(clist_t* list, size_t idx, clist_node_t* node) {
    clist_node_t* prev;
    clist_node_t* const next = (idx < list->size) ? clist_go(list, idx) : NULL;

    if (idx == 0U) {
        prev = NULL;
        list->head = node;
    }
    else {
        prev = clist_go(list, idx - 1U);
        prev->next = node;
    }

    if (next != NULL) {
        prev = next->prev;
        next->prev = node;
    }
    else {
        list->tail = node;
    }

    node->next = next;
    node->prev = prev;

    if ((idx < list->size++) && (idx > 0U)) {
        list->rul = node;
        list->ruly = idx;
        list->flags |= 2U;
    }
    else {
        if (idx == 0U) {
            list->ruly++;
        }
    }
}

/**
 * @brief Insert a new node in the list at specified position
 * @param[in] list The list to operate with
 * @param[in] idx The index where the new node will be inserted
 * @param[in] payload The value of the new node. Can be NULL. In this case, the
 *            value of node will be undefined, see clist_set_to_node()
 * @retval EXIT_SUCCESS If element is added
 * @retval EXIT_FAILURE If \a list is NULL or not enough memory to add element
 */
static int clist_insert(clist_t* list, size_t idx, const void* payload) {
    int ok = EXIT_FAILURE;

    if ((list != NULL) && (idx <= list->size)) {
        clist_node_t* node;

        if (list->zskb > 0U) {
            node = list->pkb;
            list->pkb = node->next;
            list->zskb--;
        }
        else {
            node = (clist_node_t*)malloc(sizeof(clist_node_t)
                                         + list->type_size);
        }

        if (node != NULL) {
            if (payload != NULL) {
                memcpy(node + 1, payload, list->type_size);
            }
            vnut_cl_insert_node(list, idx, node);
            ok = EXIT_SUCCESS;
        }
    }

    return ok;
}

/**
 * @brief Erase one or more elements from the list
 * @param[in] list The list containing the elements to remove
 * @param[in] idx The index of the first element to remove
 * @param[in] count The number of elements to remove
 * @note If \a list is NULL or \a idx + count > size of list, the function does
 *       nothing
 */
static void clist_erase(clist_t* list, size_t idx, size_t count) {

    if ((list != NULL) && ((idx + count) <= list->size) && (count > 0U)) {
        size_t i;
        clist_node_t* prev;
        clist_node_t* next;
        clist_node_t* node = clist_go(list, idx);

        prev = node->prev;
        next = clist_go(list, idx + count - 1U)->next;

        for (i = 0U; i < count; i++) {
            clist_node_t* const temp = node->next;

            if ((list->flags & CLIST_PAYLOAD_FREE) != 0U) {
                void** const p = (void**)(node + 1);
                free(*p);
            }
            node->next = list->pkb;
            list->pkb = node;
            list->zskb++;

            node = temp;
        }

        if (prev != NULL) {
            prev->next = next;
        }
        else {
            list->head = next;
        }

        if (next != NULL) {
            next->prev = prev;
        }
        else {
            list->tail = prev;
        }

        list->size -= count;

        if ((idx > 0U) && ((list->size - idx) > 1U)) {
            list->rul = next;
            list->ruly = idx;
            list->flags |= 2U;
        }
        else {
            if (list->ruly >= idx) {
                list->flags &= 1U;
            }
        }
    }
}

/**
 * @brief Resize \a list to \a new_size elements
 * @param[out] list The list to resize
 * @param[in] new_size The new size of \a list
 * @param[in] elem The value for added elements if any. Can be NULL
 * @retval EXIT_SUCCESS If list is correctly resized
 * @retval EXIT_FAILURE If \a list is NULL or not enough memory to add elements
 * @note If list is reduced, elements are removed from tail. If list is
 *       incremented, nodes are appended at tail
 */
static int clist_resize(clist_t* list, size_t new_size, const void* elem) {
    int ok = EXIT_FAILURE;
    if (list != NULL) {
        const size_t old_size = list->size;
        ok = EXIT_SUCCESS;
        if (new_size != old_size) {
            if (new_size < old_size) {
                clist_erase(list, new_size, old_size - new_size);
                if (list->ruly >= new_size) {
                    list->flags &= 1U;
                }
            }
            else {
                const size_t remaining = new_size - old_size;
                size_t i;
                for (i = 0U; (ok == EXIT_SUCCESS) && (i < remaining); i++) {
                    ok = clist_insert(list, list->size, elem);
                }
            }
        }
    }
    return ok;
}

/**
 * @brief Add a node at the beginning of the list
 * @param[in] list The list to operate with
 * @param[in] payload The value of the node to add. Can be NULL
 * @retval EXIT_SUCCESS If element is added
 * @retval EXIT_FAILURE If \a list is NULL or not enough memory to add element
 */
static int clist_push_front(clist_t* list, const void* payload) {
    return clist_insert(list, 0U, payload);
}

/**
 * @brief Add a node at the end of the list
 * @param[in] list The list to operate with
 * @param[in] payload The value of the node to add. Can be NULL
 * @retval EXIT_SUCCESS If element is added
 * @retval EXIT_FAILURE If \a list is NULL or not enough memory to add element
 */
static int clist_push_back(clist_t* list, const void* payload) {
    return (list != NULL) ? clist_insert(list, list->size, payload)
                          : EXIT_FAILURE;
}

/**
 * @brief Remove the first node from the list
 * @param[in] list The list to operate with
 * @note If \a list is NULL or empty, the function does nothing
 */
static void clist_pop_front(clist_t* list) { clist_erase(list, 0U, 1U); }

/**
 * @brief Remove the last node from the list
 * @param[in] list The list to operate with
 * @note If \a list is NULL or empty, the function does nothing
 */
static void clist_pop_back(clist_t* list) {
    if (list != NULL) {
        clist_erase(list, list->size - 1U, 1U);
    }
}

/**
 * @brief Clear a list, removing all its elements
 * @param[in] list The list to clear
 * @note If \a list is NULL or empty, the function does nothing
 */
static void clist_clear(clist_t* list) {
    if (list != NULL) {
        clist_erase(list, 0U, list->size);
    }
}

/**
 * @brief Free all non-necessary memory allocated by list
 * @param[in] list The list to shrink
 * @note If \a list is NULL or empty, the function does nothing
 * @note Calling this function may save some memory, but may also decrease
 *       performance if the list will grow after this call
 */
static void clist_shrink_to_fit(clist_t* list) {
    if (list != NULL) {
        clist_node_t* node = list->pkb;
        while (node != NULL) {
            clist_node_t* const next = node->next;
            free(node);
            node = next;
        }
        list->pkb = NULL;
        list->zskb = 0U;
    }
}

/**
 * @brief Destroy an initialized list
 * @param[in] list The list to destroy
 * @note If \a list is NULL, the function does nothing
 */
static void clist_destroy(clist_t* list) {
    clist_clear(list);
    clist_shrink_to_fit(list);
}

/**
 * @brief Destroy and free a list
 * @param[in] pList Pointer to a pointer to list
 * @note Call this function passing a pointer typically obtained by calling
 *       clist_new()
 * @note If \a pList is NULL, the function does nothing
 * @warning Never call this function on a stack-allocated list, call
 *          clist_destroy() in that case
 */
static void clist_delete(clist_t** pList) {
    if (pList != NULL && *pList != NULL) {
        clist_destroy(*pList);
        free(*pList);
        *pList = NULL;
    }
}

/**
 * @brief Execute passed callback to each element of the list
 * @param[in] list The list to operate with
 * @param[in] f The callback to run on all elements of the list
 */
static void clist_foreach(clist_t* list, clist_foreach_cb_t f) {
    if ((list != NULL) && (f != NULL)) {
        clist_node_t* node = list->head;
        while (node != NULL) {
            f(node + 1);
            node = node->next;
        }
    }
}

/**
 * @brief Filter the list, removing elements according to given predicate
 * @param[in] list The list to operate with
 * @param[in] f The predicate to apply to each element of the list. After this
 *              call, the list will contain \b only those elements for each
 *              the predicate function return non-zero
 */
static void clist_filter(clist_t* list, clist_filter_cb_t f) {
    if ((list != NULL) && (f != NULL)) {
        const size_t len = list->size;
        size_t i, idx, del_idx = len;
        for (i = 0U, idx = len - 1U; i < len; i++, idx--) {
            if (f(clist_go(list, idx) + 1) == 0) {
                clist_erase(list, idx, 1U);
                del_idx = i;
            }
        }
        if (list->ruly >= del_idx) {
            list->flags &= 1U;
        }
    }
}

/**
 * @brief Moves one or more nodes from a list to another (different) list
 * @param[in] source The source list, elements will be removed from this list
 * @param[in] idx The index of the first element to remove from \a source list
 * @param[in] count The number of elements to move from \a source to \a dest
 * @param[in] dest The destination list, elements will be added to this list
 * @param[in] pos The index of the new inserted elements in \a dest list
 * @note If \a source and \a dest are the same list or there is some NULL
 *       pointer or some index is not valid, the function does nothing
 * @warning The type_size and the initialization flags of the two lists must
 *          be equal, or no splice will be done
 *
 */
static void clist_splice(clist_t* source, size_t idx, size_t count,
                         clist_t* dest, size_t pos)
{
    if ((source != NULL) && (dest != NULL) && (source != dest)
        && ((idx + count) <= source->size) && (pos <= dest->size)
        && (source->type_size == dest->type_size)
        && ((source->flags & 1U) == (dest->flags & 1U))
        && (count > 0U))
    {
        clist_node_t* first;
        clist_node_t* last;
        clist_node_t* prev;
        clist_node_t* next;

        first = clist_go(source, idx);
        prev = first->prev;
        last = clist_go(source, idx + count - 1U);
        next = last->next;

        if (idx == 0U) {
            source->head = next;
        }
        else {
            prev->next = next;
        }

        if (next == NULL) {
            source->tail = prev;
        }
        else {
            next->prev = prev;
        }

        source->size -= count;

        if (source->ruly >= idx) {
            source->flags &= 1U;
        }

        if (pos == 0U) {
            prev = NULL;
        }
        else {
            prev = clist_go(dest, pos - 1U);
        }

        if (pos < dest->size) {
            next = clist_go(dest, pos);
        }
        else {
            next = NULL;
        }

        first->prev = prev;
        last->next = next;

        if (prev != NULL) {
            prev->next = first;
        }
        else {
            dest->head = first;
        }

        if (next != NULL) {
            next->prev = last;
        }
        else {
            dest->tail = last;
        }

        dest->size += count;

        if (dest->ruly >= pos) {
            dest->flags &= 1U;
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
