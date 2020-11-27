/**
 * @file      cvector.h
 * @version   1.0
 * @brief     CVector header-only vector library for C89 language
 * @date      Thu Sep  3 23:11:45 2020
 * @author    Michele Pes
 * @copyright BSD-3-Clause
 *
 * This file offers a traditional C++-like vector to C.
 * It is entirely self-contained in this file and offers the
 * traditional functions of a vector plus some configuration  macros.
 * CVector offers an interface very similar to C++ std::vector.
 * Moreover it can be configured to operate with dynamic memory (PCs)
 * or with static memory only (embedded systems).
 * When requesting unavailable memory, error conditions arise.
 * The CVector philosophy is to never return error codes/values.
 * It uses the callback mechanism. On errors, a callback 
 * (the default one or a user-defined one) is called.
 * The only handled errors are those related to available memory.
 * If for example you pass a NULL pointer where not explicitely
 * allowed, or you pass wrong indexes, etc, no callback will be 
 * called. Your program will probably crash
 */

#ifndef CVECTOR_H_
#define CVECTOR_H_

#ifdef __STDC__
typedef unsigned char cv_uchar;
#else
#include <stdint.h>
typedef uint8_t cv_uchar;
#endif

#include <string.h>

#ifdef DOXYGEN_ONLY
/**
 * @def CVECTOR_NO_DYNAMIC_MEMORY
 * This is the most important configuration parameter.
 * If this macro is defined \b before including cvector.h then 
 * no dynamic memory will be used, no malloc/free nor stdio/stdlib inclusion.
 * This way, CVector will use string.h for memcpy/memmove only. No other
 * functions of the C library will be used. This is the typical configuration
 * in embedded systems
 */
#define CVECTOR_NO_DYNAMIC_MEMORY
#endif

#ifndef CVECTOR_NO_DYNAMIC_MEMORY
#include <stdlib.h>
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def cv_ui
 * This is the unsigned integer type used by CVector. If you do not specify
 * your type, the default is \b size_t
 * @warning Changing this type will change the size \b and capacity of vectors.
 *          Keep this in mind also if you need to serialize
 */
#ifndef cv_ui
#define cv_ui size_t
#endif

/**
 * @def CVECTOR_MIN_SIZE
 * This is the default size in \b bytes that all vectors will allocate when the
 * value #CVECTOR_DEFAULT_LEN is passed as 3rd parameter of function
 * cvector_init().
 * You can tune this parameter as you wish according to the usage you intend.
 * @note If #CVECTOR_NO_DYNAMIC_MEMORY is defined, this has no effect, since no
 *       dynamic memory allocation is used
 */
#ifndef CVECTOR_MIN_SIZE
#define CVECTOR_MIN_SIZE 4096U
#endif

/**
 * @def CVECTOR_DEFAULT_LEN
 * This macro should be used as 3rd parameter in function cvector_init()
 * When this macro is passed, the initial number of elements is decided
 * according to user configuration. By \b default cvector allocates
 * #CVECTOR_MIN_SIZE \b bytes, unless configured differently by user. So cvector
 * has room for #CVECTOR_MIN_SIZE / sizeof(vector element type) if the size of
 * type is smaller or equal to #CVECTOR_MIN_SIZE. If the type size is \b greater
 * than #CVECTOR_MIN_SIZE, than cvector will allocate \b exactly the space
 * of one element, even if its size is much more bigger than #CVECTOR_MIN_SIZE
 * @note This behaviour, (of course), is valid if dynamic memory is enabled, see
 *       #CVECTOR_NO_DYNAMIC_MEMORY
 */
#define CVECTOR_DEFAULT_LEN 0U

/**
 * @name Vector elements type
 * These macros should be passed as 4th parameter of function
 * cvector_init(). Pass #CVECTOR_DATA for normal usage. In this case, the
 * elements of vector are simply discarded when elements are removed. When
 * #CVECTOR_FREE_PTR is passed, the elements of vector are assumed to be
 * allocated pointers (by malloc or similar). So when elements are removed (for
 * example with cvector_erase() and others), all removed elements will be passed
 * to the \b free function \b before being removed by vector. This may be
 * useful when user can safely ignore dynamic pointers while working with
 * cvector. This behaviour, (of course), is valid only if dynamic memory is
 * enabled, see #CVECTOR_NO_DYNAMIC_MEMORY
 * @{
 */
#define CVECTOR_DATA        0U /**< Elements are simply discarded */
#define CVECTOR_FREE_PTR    1U /**< Elements will be freed before removal */
/**
 * @}
 */

/**
 * @def CVECTOR_PTR
 * This macro returns a \b pointer of type \a t to the ith element of pv
 * @param[in] pv A pointer to the vector to work with
 * @param[in] i The index of the element
 * @param[in] t The type of the returned pointer
 * @note #CVECTOR_ELEM is just the dereferentiation of this macro
 */
#define CVECTOR_PTR(pv, i, t)  ((t*)((pv)->p) + (cv_ui)(i))

/**
 * @def CVECTOR_ELEM
 * This macro returns the ith element of pv as an instance of type \a t
 * @param[in] pv A pointer to the vector to work with
 * @param[in] i The index of the element to return
 * @param[in] t The type of the returned element
 * @note #CVECTOR_FRONT and #CVECTOR_BACK are just wrapper of this macro
 * @warning This macro makes <b>pointer deferentiation</b>, so be careful
 */
#define CVECTOR_ELEM(pv, i, t) (*(CVECTOR_PTR((pv), (i), t)))

/**
 * @def CVECTOR_FRONT
 * This macro returns the first element of pv as an element of type \a t
 * @param[in] pv A pointer to the vector to work with
 * @param[in] t The type of the returned element
 */
#define CVECTOR_FRONT(pv, t)   CVECTOR_ELEM((pv), 0U, t)

/**
 * @def CVECTOR_BACK
 * This macro returns the last element of pv as an element of type \a t
 * @param[in] pv A pointer to the vector to work with
 * @param[in] t The type of the returned element
 */
#define CVECTOR_BACK(pv, t)    CVECTOR_ELEM((pv), (pv)->n - 1U, t)


typedef struct {
    cv_uchar*  p;
    cv_uchar*  f;
    cv_ui  n;
    cv_ui  m;
    cv_ui  t;
    cv_ui  c;
    cv_ui  d;
} cvector_t;

/**
 * @typedef cvector_error_callback_t
 * This is the callback signature that will be called on memory errors.
 * If for example a call to insert fails, the error callback will be called.
 * If you don't want any callback to be called, just pass NULL to
 * cvector_set_callback(). In any moment you can restore the default error
 * callback by calling cvector_set_default_callback().
 *
 * The parameter \a failed_len contains the number of \b elements that cvector
 * failed to allocate
 */
typedef void (*cvector_error_callback_t)(cv_ui failed_len);

static void cvector_default_error_callback(cv_ui failed_len)
{
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
#ifdef _WIN32
    (void)printf("CVECTOR was unable to allocate %Iu elements, aborting...\n",
                 failed_len);
#else
    (void)printf("CVECTOR was unable to allocate %zu elements, aborting...\n",
                 failed_len);
#endif
    abort();
#else
    (void)failed_len;
    while (0 == 1) {}
#endif
}

static cvector_error_callback_t cvector_error_callback =
    &cvector_default_error_callback;

/**
 * @brief Set user-defined callback that will be called in case of error
 * @param[in] error_callback The callback to call on errors. This parameter
 *            can be NULL (when you do not want to call any callback).
 * @note There is a default error callback, that printf a message and then
 *       calls abort if dynamic memory is configured, otherwise the calling 
 *       thread will hang on an infinite loop
 */
static void cvector_set_callback(cvector_error_callback_t error_callback) {
    cvector_error_callback = error_callback;
}

/**
 * @brief Restore the default error callback
 */
static void cvector_set_default_callback(void) {
    cvector_error_callback = &cvector_default_error_callback;
}

/**
 * @brief Initialize a vector with passed memory space. No dynamic memory is
 *        required. This is typically called in embedded environment
 * @param[in] pv A pointer to the vector to initialize
 * @param[in] buffer A pointer to the memory the vector will use
 * @param[in] type_size The size of the type of elements (ex.: sizeof(int))
 * @param[in] reserved The maximum number of \b elements that can be added.
 *                     Be sure to have at least <a>type_size*reserved</a> bytes
 *                     in buffer
 * @param[in] initial_size The initial size of the vector. This is usually zero
 *                         but if the memory you pass in buffer already contains
 *                         some relevant data, you can pass a greater value.
 *                         After successful init, the vector will contain
 *                         \a initial_size elements. Existing data is never
 *                         touched by this function. This parameter must be less
 *                         than or equal to \a reserved
 * @param[in] dynamic If #CVECTOR_FREE_PTR is passed, you are indicating that
 *                    the vector will contain pointers \b allocated by malloc
 *                    or similar. This means that \b all operations that
 *                    involve removing elements or clearing/destroying the
 *                    vector, will call \b free to each value that is going to
 *                    be removed. In this case, \a type_size must be equal to
 *                    <a>sizeof(void*)</a>, otherwise the function will fail.
 *                    If you want to just discard stored elements,
 *                    you can pass #CVECTOR_DATA. If #CVECTOR_NO_DYNAMIC_MEMORY
 *                    is defined, this parameter is simply ignored.
 * @note If at a certain point you need more space than initially allocated, you
 *       can recall this function in any moment and even on the same buffer,
 *       changing for example the \a reserved parameter
 * @note This function fails if \a type_size is zero or if \a initial_size is
 *       greater than \a reserved
 * @note When this function fails, the error callaback is called. Moreover, the
 *       pointer pv->p is set to NULL. This can be checked by user for failure.
 */
static void cvector_init_ext(cvector_t* pv,
                             void* buffer,
                             cv_ui type_size,
                             cv_ui reserved,
                             cv_ui initial_size,
                             int dynamic)
{
    const cv_ui* p_error = NULL;

    if (type_size == 0U
        || ((dynamic == CVECTOR_FREE_PTR) && (type_size != sizeof(void*))))
    {
        p_error = &type_size;
    }
    else if (initial_size > reserved) {
        p_error = &initial_size;
    }
    else {
        pv->c = ((cv_ui)-1) / type_size;
        if (reserved > pv->c) {
            p_error = &reserved;
        }
    }

    if (p_error == NULL) {
        pv->p = (cv_uchar*)buffer;
        pv->f = pv->p + (initial_size * type_size);
        pv->n = initial_size;
        pv->m = reserved;
        pv->t = type_size;
        pv->d = ((cv_ui)dynamic & 1U) | 2U;
    }
    else {
        pv->p = NULL;
        if (cvector_error_callback != NULL) {
            (*cvector_error_callback)(*p_error);
        }
    }
}

static cv_ui vnut_init(cvector_t* pv,
                       cv_ui type_size,
                       cv_ui num_elems,
                       int dynamic)
{
#ifdef CVECTOR_NO_DYNAMIC_MEMORY
    (void)num_elems;
    (void)dynamic;
    pv->p = NULL;
    return type_size;
#else
    cv_ui* p_error = NULL;

    if (type_size == 0U
        || ((dynamic == CVECTOR_FREE_PTR) && (type_size != sizeof(void*))))
    {
        p_error = &type_size;
    }
    else {
        pv->c = (cv_ui)(-1) / type_size;
        if (num_elems > pv->c) {
            p_error = &num_elems;
        }
    }

    if (p_error == NULL) {

        if (num_elems == CVECTOR_DEFAULT_LEN) {
            if (type_size > CVECTOR_MIN_SIZE) {
                num_elems = 1U;
            }
            else {
                num_elems = CVECTOR_MIN_SIZE / type_size;
            }
        }

        pv->p = (cv_uchar*)malloc(num_elems * type_size);
        if (pv->p != NULL) {
            pv->f = pv->p;
            pv->n = 0U;
            pv->m = num_elems;
            pv->t = type_size;
            pv->d = (cv_ui)dynamic & 1U;
        }
        else {
            p_error = &num_elems;
        }
    }

    if (p_error != NULL) {
        pv->p = NULL;
    }

    return (p_error == NULL) ? 0U : *p_error;
#endif
}

/**
 * @brief This function initialize a vector using dynamic memory. To initialize
 *        a vector without dynamic memory, see cvector_init_ext(). See also
 *        #CVECTOR_NO_DYNAMIC_MEMORY
 * @param[in] pv A pointer to the vector to initialize
 * @param[in] type_size The size of the type of elements (ex.: sizeof(int))
 * @param[in] num_elems The number of elements to be allocated. To let CVector
 *                      decide the initial size, you can pass
 *                      #CVECTOR_DEFAULT_LEN. Please note that
 *                      #CVECTOR_DEFAULT_LEN has value zero, so there is no way
 *                      to initialize an empty array. See also #CVECTOR_MIN_SIZE
 *                      Note also that the allocator policy is ignored here. If
 *                      you pass for example 1 as \a num_elems, exactly
 *                      \a type_size bytes will be allocated. This is to allow
 *                      user to do one allocation only at init time, if you know
 *                      the maximum size that the vector will have
 * @param[in] dynamic If #CVECTOR_FREE_PTR is passed, you are indicating that
 *                    the vector will contain pointers \b allocated by malloc
 *                    and similar. This means that \b all operations that
 *                    involve removing elements or clearing/destroying the
 *                    vector, will call \b free to each value that is going to
 *                    be removed. In this case, \a type_size must be equal to
 *                    <a>sizeof(void*)</a>, otherwise the function will fail.
 *                    If you want to just discard stored elements,
 *                    you can pass #CVECTOR_DATA
 * @note This function fails if \a type_size is zero
 * @note This function will always fail if #CVECTOR_NO_DYNAMIC_MEMORY is defined
 * @note When this function fails, the error callaback is called. Moreover, the
 *       pointer pv->p is set to NULL. This can be checked by user for failure.
 * @warning Do not call any function (except init) after failure to this
 *          function
 */
static void cvector_init(cvector_t* pv,
                         cv_ui type_size,
                         cv_ui num_elems,
                         int dynamic)
{
    const cv_ui init_err = vnut_init(pv, type_size, num_elems, dynamic);
    if ((pv->p == NULL) && (cvector_error_callback != NULL)) {
        (*cvector_error_callback)(init_err);
    }
}

/**
 * @brief Initialize a new vector, using dynamic memory
 * @param[in] type_size The size of vector elements type (ex.: sizeof(int))
 * @param[in] num_elems The number of initial elements to be allocated.
 *                      See cvector_init() for details on this parameter
 * @param[in] dynamic Tells whether stored elements are to be freed or not, see
 *            cvector_init() for details on this parameter
 * @return On error, NULL is returned and <b>no callback</b> is called.
 *         On successful invocation, a pointer to the initialized vector is
 *         returned
 * @note If #CVECTOR_NO_DYNAMIC_MEMORY is defined, this function will always
 *       fail
 */
static cvector_t* cvector_new(cv_ui type_size, cv_ui num_elems, int dynamic)
{
#ifdef CVECTOR_NO_DYNAMIC_MEMORY
    (void)type_size;
    (void)num_elems;
    (void)dynamic;
    return NULL;
#else
    cvector_t* pv = (cvector_t*)malloc(sizeof(cvector_t));
    if (pv != NULL) {
        (void)vnut_init(pv, type_size, num_elems, dynamic);
        if (pv->p == NULL) {
            free(pv);
            pv = NULL;
        }
    }
    return pv;
#endif
}

/**
 * @brief Clone given vector
 * @param[in] clone The new clone. If it is NULL, it will be allocated. The
 *            pointed vector must not be initialized, or must be destroyed
 *            \b before calling this function. If #CVECTOR_NO_DYNAMIC_MEMORY is
 *            defined, \a clone must point to an initialized vector. The
 *            original memory block and its capacity will remain the same. The
 *            function will fail if \a clone has not enough space to contain all
 *            elements from \a pv
 * @param[in] pv The vector to clone
 * @return The new clone or NULL on errors. Error callback is never called
 * @note If #CVECTOR_NO_DYNAMIC_MEMORY is defined and \a clone is NULL, the
 *       function will always fail. If \a clone is not NULL, the function will
 *       return it on succesful cloning
 * @note A cloned vector obtained from heap can be passed to cvector_delete()
 */
static cvector_t* cvector_clone(cvector_t* clone, const cvector_t* pv) {

#ifndef CVECTOR_NO_DYNAMIC_MEMORY

    cvector_t* other = (clone == NULL) ? (cvector_t*)malloc(sizeof(cvector_t))
                                       : clone;
    if (other != NULL) {
        const cv_ui t = pv->t;

        *other = *pv;

        other->p = (cv_uchar*)malloc(pv->m * t);
        if (other->p != NULL) {
            memcpy(other->p, pv->p, pv->n * t);
            other->f = other->p + (pv->n * t);
        }
        else {
            if (clone == NULL) {
                free(other);
            }

            other = NULL;
        }
    }

#else

    cvector_t* const other = ((clone == NULL) || (clone->m < pv->n)) ? NULL
                                                                     : clone;
    if (other != NULL) {
        cv_uchar* const p = other->p;
        const cv_ui m = other->m;

        *other = *pv;

        other->p = p;
        other->f = other->p + (pv->n * pv->t);
        other->m = m;

        memcpy(other->p, pv->p, pv->n * pv->t);
    }

#endif

    return other;
}

/**
 * @brief Clear the vector
 * @param[in] pv A pointer to the vector to clear
 * @note No memory will be freed after this call, the new vector size will be
 *       zero. See cvector_shrink_to_fit() to save memory
 */
static void cvector_clear(cvector_t* pv) {
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
    if ((pv->d & 1U) == 1U) {
        void** const p = (void**)pv->p;
        const cv_ui n = pv->n;
        cv_ui i;
        for (i = 0U; i < n; i++) {
            free(p[i]);
        }
    }
#endif
    pv->f = pv->p;
    pv->n = 0U;
}

/**
 * @brief Destroy a vector, deallocating its payload
 * @param[in] pv A pointer to the vector to destroy
 * @note This call frees all memory occupied by elements.
 *       After destroy, the only allowed operations are init and delete.
 *       Calling destroy consecutively on the same vector is useless but allowed
 * @note If a vector was initialized by cvector_init_ext(), no memory will be
 *       freed
 * @sa cvector_delete()
 */
static void cvector_destroy(cvector_t* pv) {
    cvector_clear(pv);
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
    if ((pv->d & 2U) == 0U) {
        free(pv->p);
    }
#endif
    pv->p = NULL;
}

/**
 * @brief Delete a vector, tipically obtained by cvector_new()
 * @param[out] pv A pointer to a pointer to the vector to delete. This
 *                parameter can be NULL (in this case the function does nothing)
 * @note This function call cvector_destroy() and then free the vector
 *       The typical usage is something like:
 * @code{.c}
 * cvector_t* pv = cvector_new(...);
 * ...
 * cvector_delete(&pv);
 * @endcode
 * @note You can call cvector_destroy() and the manually free the pointer. This
 *       is equivalent to calling this function.
 * @note After this call, the passed variable will be NULL (that's why a double
 *       pointer is required)
 * @warning Never call this function on a stack-allocated vector, call
 *          cvector_destroy() in that case
 */
static void cvector_delete(cvector_t** pv) {
    if (*pv != NULL) {
        cvector_destroy(*pv);
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
        free(*pv);
#endif
        *pv = NULL;
    }
}

static int vnut_reserve(cvector_t* pv, cv_ui new_size) {
#ifdef CVECTOR_NO_DYNAMIC_MEMORY
    (void)pv;
    (void)new_size;
    return 0;
#else
    int ok = ((pv->d & 2U) == 0U);
    if (ok != 0) {
        const cv_ui n = pv->n;
        const cv_ui ts = pv->t;
        const cv_ui c = pv->c;
        cv_ui trying;
        void* p;

        if (n < (c / 4U)) {
            trying = n * 2U;
        }
        else {
            trying = n + (n / 8U);

            if ((trying > pv->c) || (trying == n)) {
                trying = new_size;
            }
        }

        if (trying < new_size) {
            trying = new_size;
        }

        p = realloc(pv->p, trying * ts);
        if (p == NULL && trying > new_size) {
            trying = new_size;
            p = realloc(pv->p, trying * ts);
        }

        ok = p != NULL;
        if (ok != 0) {
            pv->p = p;
            pv->f = (cv_uchar*)p + (n * ts);
            pv->m = trying;
        }
    }
    return ok;
#endif
}

/**
 * @brief Append an element to the vector
 * @param[in] pv A pointer to the vector
 * @param[in] elem A pointer to the element to append
 */
static void cvector_push_back(cvector_t* pv, const void* elem) {
    int ok = -1;
    if (pv->n == pv->m) {
        ok = (pv->n < pv->c) && (vnut_reserve(pv, pv->n + 1U) != 0);
    }
    if (ok != 0) {
        const cv_ui t = pv->t;
        memcpy(pv->f, elem, t);
        pv->f += t;
        pv->n++;
    }
    else {
        if (cvector_error_callback != NULL) {
            (*cvector_error_callback)(pv->n + 1U);
        }
    }
}

/**
 * @brief Remove the last element from the vector
 * @param[in] pv A pointer to the vector
 */
static void cvector_pop_back(cvector_t* pv) {
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
    if ((pv->d & 1U) == 1U) {
        void** const p = (void**)pv->p;
        free(p[pv->n - 1U]);
    }
#endif
    pv->n--;
    pv->f -= pv->t;
}

/**
 * @brief Return the number of elements currently present in the vector
 * @param[in] pv A constant pointer to the vector
 * @return The current length of the vector
 */
static cv_ui cvector_size(const cvector_t* pv) {
    return pv->n;
}

/**
 * @brief Tell if vector is empty
 * @param[in] pv A constant pointer to the vector
 * @return Non-zero if vector is empty, zero otherwise
 */
static int cvector_empty(const cvector_t* pv) {
    return (pv->n == 0U) ? 1 : 0;
}
    
/**
 * @brief Return a \a void* pointer to passed element index
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the element
 * @return A \a void* pointer, see #CVECTOR_PTR for a typed pointer
 */
static void* cvector_get_data(cvector_t* pv, cv_ui idx) {
    return pv->p + (idx * pv->t);
}

/**
 * @brief Return a \a void* pointer to to the first element
 * @param[in] pv A pointer to the vector
 * @return A \a void* pointer, see #CVECTOR_FRONT for a typed pointer
 */
static void* cvector_front(cvector_t* pv) {
    return pv->p;
}

/**
 * @brief Return a \a void* pointer to to the last element
 * @param[in] pv A pointer to the vector
 * @return A \a void* pointer, see #CVECTOR_BACK for a typed pointer
 */
static void* cvector_back(cvector_t* pv) {
    return (pv->p + ((pv->n - 1U) * pv->t));
}

/**
 * @brief Set a value to an element
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the element to be copied
 * @param[in] elem The element to be copied
 * @sa cvector_set_elems()
 */
static void cvector_set_data(cvector_t* pv, cv_ui idx, const void* elem) {
    memcpy(pv->p + (idx * pv->t), elem, pv->t);
}

/**
 * @brief Set a subset of elements
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the first element to set
 * @param[in] len The number of elements to set
 * @param[in] elem A pointer to the element to be replicated
 * @param[in] is_same_element If non-zero, the element pointed by \a elem will
 *                            be replicated \a len times, at pv[idx..idx+len-1].
 *                            If zero, \a len elements will be copied from
 *                            elem[0..len-1] to pv[idx..idx+len-1]
 * @warning This function assumes that requested space is \b already available
 */
static void cvector_set_elems(cvector_t* pv,
                              cv_ui idx,
                              cv_ui len,
                              const void* elem,
                              int is_same_element)
{
    if (len > 0U) {
        const cv_ui t = pv->t;
        if (is_same_element != 0) {
            cv_uchar* dest = pv->p + (idx * t);
            cv_ui i;
            for (i = 0U; i < len; i++, dest += t) {
                memcpy(dest, elem, t);
            }
        }
        else {
            memcpy(pv->p + (idx * t), elem, len * t);
        }
    }
}

/**
 * @brief Insert an element to the vector
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the newly inserted element
 * @param[in] elem The value of the element to insert
 * @note The index can be lesser or equal to the vector size. In the latter
 *       case, it is equivalent to calling cvector_push_back()
 */
static void cvector_insert(cvector_t* pv, cv_ui idx, const void* elem) {
    int ok = -1;
    const cv_ui n = pv->n;
    if (n == pv->m) {
        ok = (pv->n < pv->c) && (vnut_reserve(pv, n + 1U) != 0);
    }
    if (ok != 0) {
        const cv_ui t = pv->t;
        const cv_ui idx_t = idx * t;
        cv_uchar* const p = pv->p;
        if (idx < n) {
            memmove(p + idx_t + t, p + idx_t, (n - idx) * t);
        }
        memcpy(p + idx_t, elem, t);
        pv->n++;
        pv->f += t;
    }
    else {
        if (cvector_error_callback != NULL) {
            (*cvector_error_callback)(n + 1U);
        }
    }
}

/**
 * @brief Insert element[s] to the vector
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the starting position of inserted elements
 * @param[in] len The number of elements to be inserted
 * @param[in] elem The initial value of \b all newly inserted elements.
 *                 This parameter can be NULL to leave memory uninitialized
 * @param[in] is_same_element If non-zero, the element pointed by \a elem will
 *                            be inserted \a len times, at pv[idx..idx+len-1].
 *                            If zero, \a len elements will be copied from
 *                            elem[0..len-1] to pv[idx..idx+len-1]. This is a
 *                            good way to prepend/insert/append another array
 * @note The index can be lesser or equal to the vector size. In the latter
 *       case, element[s] will be pushed back
 */
static void cvector_insert_n(cvector_t* pv,
                             cv_ui idx,
                             cv_ui len,
                             const void* elem,
                             int is_same_element)
{
    if (len > 0) {
        const cv_ui new_size = pv->n + len;
        int ok = 0;

        if ((new_size > pv->n) && (new_size <= pv->c)) {
            if (new_size <= pv->m) {
                ok--;
            }
            else {
                ok = vnut_reserve(pv, new_size);
            }
            if (ok != 0) {
                const cv_ui t = pv->t;
                if (idx < pv->n) {
                    memmove(pv->p + ((idx + len) * t),
                            pv->p + (idx * t),
                            (pv->n - idx) * t);
                }
                if (elem != NULL) {
                    cvector_set_elems(pv, idx, len, elem, is_same_element);
                }
                pv->n += len;
                pv->f += (len * t);
            }
        }
        if ((ok == 0) && (cvector_error_callback != NULL)) {
            (*cvector_error_callback)(len);
        }
    }
}

/**
 * @brief Erase elements from the vector
 * @param[out] pv A pointer to the vector
 * @param[in] idx The index of the first element to remove
 * @param[in] len The number of elements to remove
 */
static void cvector_erase(cvector_t* pv, cv_ui idx, cv_ui len) {
    if (len > 0) {
        const cv_ui t = pv->t;
#ifndef CVECTOR_NO_DYNAMIC_MEMORY
        if ((pv->d & 1U) != 0U) {
            void** const p = (void**)pv->p;
            cv_ui i = idx, n = len;
            do { free(p[i++]); } while (--n);
        }
#endif
        if ((idx + len) < pv->n) {
            memmove(pv->p + (idx * t),
                    pv->p + (idx + len) * t,
                    (pv->n - (idx + len)) * t);
        }
        pv->n -= len;
        pv->f -= (len * t);
    }
}

/**
 * @brief Remove elements from vector, faster but break ordering 
 * @param[in] pv A pointer to the vector
 * @param[in] idx The index of the first element to remove
 * @param[in] len The number of elements to remove
 * @note This function is much more faster than cvector_erase(), but breaks
 *       ordering of elements
 */
static void cvector_erase_fast(cvector_t* pv, cv_ui idx, cv_ui len)
{
    if (len > 0) {
        const cv_ui sz = pv->n;
        const cv_ui t = pv->t;

#ifndef CVECTOR_NO_DYNAMIC_MEMORY
        if ((pv->d & 1U) != 0U) {
            void** const p = (void**)pv->p;
            cv_ui i = idx, n = len;
            do free(p[i++]); while (--n);
        }
#endif

        if ((idx + len) < sz) {
            const cv_ui to_move = (sz - (idx + len)) < len ? (sz - (idx + len))
                                                           : len;
            memcpy(pv->p + (idx * t),
                   pv->p + ((sz - to_move) * t),
                   to_move * t);
        }

        pv->n -= len;
        pv->f -= (len * t);
    }
}

/**
 * @brief Resize the vector
 * @param[in] pv A pointer to the vector
 * @param[in] new_size The new size of the vector. This parameter represents the
 *            new number of elements that the vector will have
 * @param[in] elem A pointer to the element to be replicated when enlarging
 *                 the vector. This parameter can be NULL when reducing the
 *                 size or simply leaving the new space uninitialized
 * @note If vector is reduced, elements are removed from tail. If vector is
 *       incremented, elements are appended at tail
 */
static void cvector_resize(cvector_t* pv, cv_ui new_size, const void* elem) {
    const cv_ui n = pv->n;
    if (new_size != n) {
        if (new_size > n) {
            cvector_insert_n(pv, n, new_size - n, elem, 1);
        }
        else {
            cvector_erase(pv, new_size, n - new_size);
        }
    }
}

/**
 * @brief Reserve space for <b>at least</b> new_size elements
 * @param[in] pv A pointer to the vector
 * @param[in] length The maximum length that the vector can reach \b without
 *                   any further allocations 
 */
static void cvector_reserve(cvector_t* pv, cv_ui length) {
    if (length > pv->m) {
        const int ok = (length <= pv->c) && (vnut_reserve(pv, length) != 0);
        if ((ok == 0) && (cvector_error_callback != NULL)) {
            (*cvector_error_callback)(length);
        }
    }
}

/**
 * @brief Resize allocated memory block to the minimum, effectively
 *        saving memory
 * @param[in] pv A pointer to the vector
 * @note If this function is called on an empty array, the new memory block size
 *       will not be zero, but it will allocate room for <b>exactly one</b>
 *       element
 */
static void cvector_shrink_to_fit(cvector_t* pv) {
#ifdef CVECTOR_NO_DYNAMIC_MEMORY
    (void)pv;
#else
    const cv_ui n = pv->n;
    if ((n < pv->m) && ((pv->d & 2U) == 0U)) {
        const cv_ui total = ((n == 0U) ? 1 : n) * pv->t;
        void* const p = malloc(total);
        if (p != NULL) {
            memcpy(p, pv->p, total);
            pv->m = total / pv->t;
            pv->f = (cv_uchar*)p + total;
            free(pv->p);
            pv->p = p;
        }
    }
#endif
}

#ifdef __cplusplus
}
#endif

#endif
