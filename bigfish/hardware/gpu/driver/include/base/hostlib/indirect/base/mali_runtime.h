/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2006-2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * @file mali_runtime.h
 * Wraps all platform/libc functions we use.
 */

#ifndef _MALI_RUNTIME_H_
#define _MALI_RUNTIME_H_

#include <base/mali_types.h>
#include <base/mali_macros.h>

#include <mali_intrinsics_os_tc.h>
#include <mali_intrinsics_tc.h>
#include <mali_intrinsics_os.h>
#include <mali_intrinsics_cmn.h>
#include <mali_neon_tc.h>
#include <mali_convert_intrinsics.h>

#ifdef __cplusplus
extern "C" {
#endif


/*** Memory functions ***/
/**
 * Allocate a memory. A buffer of minimum ''size'' bytes is allocated.
 * Remember to free memory using @see _mali_sys_free.
 * @param size Number of bytes to allocate
 * @return The buffer allocated
 */
MALI_IMPORT void *_mali_sys_malloc(u32 size);

/**
 * Free memory. Returns the buffer pointed to by ''pointer'' to the system.
 * Remember to free all allocated memory, and to only free it once.
 * @param pointer Pointer to buffer to free
 */
MALI_IMPORT void _mali_sys_free(void *pointer);

/**
 * Allocate zero-initialized memory. Returns a buffer capable of containing ''nelements'' of ''bytes'' size each.
 * The buffer is zero-initialized.
 * @param nelements Number of elements to allocate
 * @param bytes Size of each element
 * @return The zero-initialized buffer allocated
 */
MALI_IMPORT void *_mali_sys_calloc(u32 nelements, u32 bytes);

/**
 * Reallocate memory. Tries to shrink or extend the memory buffer pointed to by ''ptr'' to ''bytes'' bytes.
 * If there is no room for an inline expansion the buffer will be relocated.
 * @param ptr Pointer to buffer to try to shrink/expand
 * @param bytes New size of buffer after resize
 * @return Pointer to the possibly moved memory buffer.
 */
MALI_IMPORT void *_mali_sys_realloc(void *ptr, u32 bytes);

/**
 * Creates a duplicate of the string argument.
 * Creates a copy of the string argument. The copy will
 * contain the same bytes as the original up to and including
 * the first zero terminator. A zero-length string will result
 * in another zero-length string.
 * @param str The string to duplicate
 * @return The duplicated string, or NULL if the string duplication failed
 */
MALI_IMPORT char *_mali_sys_strdup(const char *str);

/**
 * Copies the values of num bytes from the location pointed by source directly to
 * the memory block pointed by destination
 * @param destination Pointer to the destination array where the content is to be copied.
 * @param source Pointer to the source of data to be copied.
 * @param num Number of bytes to copy.
 */
MALI_IMPORT void *_mali_sys_memcpy(void *destination, const void *source, u32 num);

/**
 * Copies the values of num bytes from the location pointed by source directly to
 * the memory block pointed by destination
 *
 * This function is for copying memory block to/from Mali memory.
 *
 * @param destination Pointer to the destination array where the content is to be copied.
 * @param source Pointer to the source of data to be copied.
 * @param num Number of bytes to copy.
 * @param element_size Size of a single copied item in bytes
 */
MALI_IMPORT void *_mali_sys_memcpy_cpu_to_mali(void *destination, const void *source, u32 num, u32 element_size);

/**
 * Copies the values of num bytes from the location in mali memory to mali
 * memory. The function performs the copy in such manner that byte order
 * of data is not changed from mali point of view.
 *
 * This function is for copying memory block from on location to another in
 * Mali memory.
 *
 * @param destination Pointer to the destination array where the content is to be copied.
 * @param source Pointer to the source of data to be copied.
 * @param num Number of bytes to copy.
 * @param element_size Size of a single copied item in bytes
 */
MALI_IMPORT void *_mali_sys_memcpy_mali_to_mali(void *destination, const void *source, u32 num, u32 element_size);

/**
 * Sets the first num bytes of the block of memory pointed to by ptr to the specified value
 * @param ptr Pointer to the block of memory to fill.
 * @param value Value to be set, passed as u32 big  block is filled with unsigned char conversion.
 * @param num Number of bytes to be set to the value.
 */
MALI_IMPORT void *_mali_sys_memset(void *ptr, u32 value, u32 num);

/**
 * As _mali_sys_memset but takes into account endianess of memory area
 * Used for memsetting mali memory
 * @param ptr Pointer to the block of memory to fill.
 * @param value Value to be set, passed as u32 big  block is filled with unsigned char conversion.
 * @param num Number of bytes to be set to the value.
 */
MALI_IMPORT void *_mali_sys_memset_endian_safe(void *ptr, u32 value, u32 num);


/**
 * Compares n bytes of two regions of memory, treating each byte as an unsigned character.
 * It returns an integer less than, equal to or greater than zero, according to whether s1
 * is lexicographically less than, equal to or greater than s2.
 * @param s1 Points to the first buffer to compare
 * @param s2 Points to second buffer
 * @param n Number of bytes to compare
 */

MALI_IMPORT s32 _mali_sys_memcmp(const void *s1, const void *s2, u32 n);

/*** Spinlock functions ***/

typedef struct mali_spinlock_type *mali_spinlock_handle;

MALI_IMPORT mali_spinlock_handle _mali_sys_spinlock_create(void);
MALI_IMPORT void _mali_sys_spinlock_lock(mali_spinlock_handle);
MALI_IMPORT void _mali_sys_spinlock_unlock(mali_spinlock_handle);
MALI_IMPORT mali_err_code _mali_sys_spinlock_try_lock(mali_spinlock_handle);
MALI_IMPORT void _mali_sys_spinlock_destroy(mali_spinlock_handle);

/*** Mutex functions ***/
/**
 * Type definition for use of mutex-functions
 */
typedef struct mali_mutex_type *mali_mutex_handle;

/**
 * Statically initialised mutex identifiers to use with _mali_sys_mutex_static()
 */
typedef enum mali_static_mutex
{
	MALI_STATIC_MUTEX_BASE,         /**< Statically initialised mutex in use by the Base Driver */
	MALI_STATIC_MUTEX_TEST_SYSTEM,  /**< Statically initialised mutex in use by the MALI test system */
	MALI_STATIC_MUTEX_USER,         /**< Statically initialised mutex for use by user */
	MALI_STATIC_MUTEX_MAX           /**< Number of statically initialised mutexes available */
} mali_static_mutex;

/**
 * Auto initialize a mutex on first use.
 * Used to initialize a mutex by the first thread referencing it.
 * This function will fail if the mutex could not be created or the given pointer is NULL.
 * This function should only be used when you don't know if a mutex has been created or not,
 * use @see _mali_sys_mutex_create for normal initialization functions.
 * @param pHandle A pointer to the mutex
 * @return MALI_ERR_NO_ERROR if mutex is ready for use, or MALI_ERR_FUNCTION_FAILED on failure
 */
MALI_IMPORT mali_err_code _mali_sys_mutex_auto_init(volatile mali_mutex_handle *pHandle) MALI_CHECK_RESULT;

/**
 * Obtains access to a statically initialized mutex. Initially these
 * mutexes are in an unlocked state.
 * @param id Identifier of a statically initialized mutex
 * @return A handle to a statically initialized mutex. NULL on failure.
 */
MALI_IMPORT mali_mutex_handle _mali_sys_mutex_static(mali_static_mutex id);

/**
 * Create a mutex.
 * Creates a mutex, initializes it to an unlocked state and returns a handle to it.
 * When the mutex is no longer needed it has to destroyed, see @see _mali_mutex_destroy.
 * @return A handle to a new mutex
 */
MALI_IMPORT mali_mutex_handle _mali_sys_mutex_create(void);

/**
 * Destroy a mutex.
 * Destroys a mutex previously allocated by @see _mali_mutex_create.
 * Returns the resources used by this mutex to the system.
 * @note The mutex being destroyed must not be locked
 * @param mutex The mutex to destroy.
 */
MALI_IMPORT void _mali_sys_mutex_destroy(mali_mutex_handle mutex);

/**
 * Lock a mutex.
 * Locks the mutex, blocks until a lock can be taken if the mutex is already locked.
 * When finished with a mutex, remember to unlock it with @see _mali_mutex_unlock.
 * Multiple locking of the same mutex from the same thread is not supported.
 * @param mutex The mutex to lock
 */
MALI_IMPORT void _mali_sys_mutex_lock(mali_mutex_handle mutex);

/**
 * Try to lock a mutex, but don't block if it's already locked.
 * This function tries to lock a mutex, but returns with an error if the mutex can't be locked right away.
 * When finished with a mutex, remember to unlock it with @see _mali_mutex_unlock.
 * Multiple locking of the same mutex from the same thread is not supported.
 * @param mutex The mutex to try to lock
 * @return If the mutex was successfully locked: MALI_ERR_NO_ERROR,
 *          on error or if the mutex was locked: MALI_ERR_FUNCTION_FAILED
 */
MALI_IMPORT mali_err_code _mali_sys_mutex_try_lock(mali_mutex_handle mutex) MALI_CHECK_RESULT;

/**
 * Unlock a locked mutex.
 * This function unlocks a previously locked mutex.
 * Multiple locking of the same mutex from the same thread is not supported.
 * @param mutex The mutex to unlock
 */
MALI_IMPORT void _mali_sys_mutex_unlock(mali_mutex_handle mutex);

/**
 * Create a reentrant mutex.
 * Creates a reentrant mutex, initializes it to an unlocked state and returns a handle to it.
 * When the reentrant mutex is no longer needed it has to destroyed, see @see _mali_mutex_reentrant_destroy.
 * @return A handle to a new reentrant mutex, or NULL if creation failed.
 */
MALI_IMPORT mali_mutex_reentrant_handle _mali_sys_mutex_reentrant_create(void) MALI_CHECK_RESULT;

/**
 * Destroy a reentrant mutex.
 * Destroys a reentrant mutex previously allocated by @see _mali_mutex_reentrant_create.
 * Returns the resources used by this reentrant mutex to the system.
 * @note The reentrant mutex being destroyed must not be locked
 * @param mutex The reentrant mutex to destroy.
 */
MALI_IMPORT void _mali_sys_mutex_reentrant_destroy(mali_mutex_reentrant_handle mutex);

/**
 * Lock a reentrant mutex.
 * Locks the reentrant mutex, blocks until a lock can be taken if the mutex is already locked by a different thread.
 * When finished with a reentrant mutex, remember to unlock it with @see _mali_mutex_reentrant unlock.
 * @param mutex The reentrant mutex to lock
 */
MALI_IMPORT void _mali_sys_mutex_reentrant_lock(mali_mutex_reentrant_handle mutex);

/**
 * Unlock a locked reentrant mutex.
 * This function unlocks a previously locked reentrant mutex.
 * @param mutex The reentrant mutex to unlock
 */
MALI_IMPORT void _mali_sys_mutex_reentrant_unlock(mali_mutex_reentrant_handle mutex);

/**
 * Check if current thread has locked reentrant mutex.
 * @param mutex The reentrant mutex to check.
 * @return MALI_TRUE if current thread has locked this reentrant mutex, MALI_FALSE if not.
 */
MALI_IMPORT mali_bool _mali_sys_mutex_reentrant_is_locked(mali_mutex_reentrant_handle mutex);

/**
 * Create a lock object.
 * Lock objects extends mutexes by being unlockable from a different thread than the one doing the locking.
 * Locks are initially in an unlocked state.
 * @return Handle of a new lock or MALI_NO_HANDLE on error
 */
MALI_IMPORT mali_lock_handle _mali_sys_lock_create(void);

/**
 * Auto initialize a lock object on first use.
 * Used to initialize a lock object by the first thread referencing it.
 * This function will fail if the lock object could not be created or the given pointer is NULL.
 * This function should only be used when you don't know if a lock object has been created or not,
 * use @see _mali_sys_lock_create for normal initialization functions.
 * @param pHandle A pointer to the lock handle
 * @return MALI_ERR_NO_ERROR if the lock object is ready for use, or MALI_ERR_FUNCTION_FAILED on failure
 */
MALI_IMPORT mali_err_code _mali_sys_lock_auto_init(volatile mali_lock_handle *pHandle) MALI_CHECK_RESULT;

/**
 * Destroy a lock object.
 * Destroy a lock previously created by @see _mali_sys_lock_create
 * Locks must be in the unlocked state when being destroyed
 * @param lock Handle to the lock object to destroy
 */
MALI_IMPORT void _mali_sys_lock_destroy(mali_lock_handle lock);

/**
 * Lock a lock object.
 * When returning the lock object will be in a locked state.
 * If the lock object is already in this state the function
 * will wait until it comes into the unlocked state before taking the lock.
 * @param lock Handle to the lock object to lock
 */
MALI_IMPORT void _mali_sys_lock_lock(mali_lock_handle lock);

/**
 * Lock a lock object with timeout.
 * When returning the lock object will be in a locked state unless the timeout expired.
 * If the lock object is already in this state the function
 * will wait until it comes into the unlocked state before taking the lock.
 * If the supplied timeout expires then the function will return without locking.
 * @param lock Handle to the lock object to lock
 * @param timeout Relative time in microseconds of timeout
 * @return MALI_ERR_NO_ERROR if the lock was obtained or MALI_ERR_TIMEOUT on timeout
 */
MALI_IMPORT mali_err_code _mali_sys_lock_timed_lock(mali_lock_handle lock, u64 timeout);

/**
 * Try to lock a lock object, but don't block if it's already locked.
 * This function tries to lock a lock object, but returns with an error if the lock object can't be locked right away.
 * @param lock Handle to the lock object to try to lock
 * @return If the mutex was successfully locked: MALI_ERR_NO_ERROR,
 *          on error or if the mutex was locked: MALI_ERR_FUNCTION_FAILED
 */
MALI_IMPORT mali_err_code _mali_sys_lock_try_lock(mali_lock_handle lock) MALI_CHECK_RESULT;

/**
 * Unlock a lock object.
 * Will unlock the lock object, notifying any other threads waiting for the lock before returning.
 * Calling unlock on a lock object in the unlocked state is invalid and will result in undefined behavior.
 * @param lock Handle to the lock object to unlock
 */
MALI_IMPORT void _mali_sys_lock_unlock(mali_lock_handle lock);


/*** Thread Local Storage ***/
/**
 * Thread Local Storage (TLS) are used for threads to store thread local context data.
 * Each thread can have its own data stored without the risk of other threads corrupting it
 * or needing any access synchronization.
 * TLS is divided into multiple slots, each accessed by a special key. A module/subsystem gets a key of its own
 * if it needs a separate TLS slot.
 * We use static thread keys, which are stored in the base project.
 * This is due to the limitations in Symbian, which doesn't support writable static data, which is needed for dynamic keys.
 */
typedef enum mali_thread_keys
{
	MALI_THREAD_KEY_EGL_CONTEXT = 0,
	MALI_THREAD_KEY_INSTRUMENTED_DATA,
	MALI_THREAD_KEY_DUMP_CALLBACK,
	MALI_THREAD_KEY_GLES_CONTEXT,
	MALI_THREAD_KEY_VG_CONTEXT,
	MALI_THREAD_KEY_MALI_EGL_IMAGE,
	MALI_THREAD_KEY_MAX
} mali_thread_keys;

/** Prototype for thread key desctruktor callback
 */
typedef void (*mali_thread_key_destructor)(void *);

/**
 * Set data stored in the TLS slot referenced by ''key''.
 * @param key The TLS key
 * @param value The value to store in the slot
 * @param desctructor The destructor that will be called when the thread exits
 */
MALI_IMPORT mali_err_code _mali_sys_thread_key_set_data(mali_thread_keys key, void *value, mali_thread_key_destructor destructor) MALI_CHECK_RESULT;

/**
 * Get data stored in the TLS slot referenced by ''key''.
 * @param key The TLS key
 * @return The value stored in the slot
 */
MALI_IMPORT void *_mali_sys_thread_key_get_data(mali_thread_keys key);


/**
 * Get current thread identifier
 * @return 32-bit thread identifier
 */
MALI_IMPORT u32 _mali_sys_thread_get_current(void);


/*** Generic functions ***/
/**
 * printf wrapper.
 * Follows the usual behavior of printf, so you can see its documentation for its usage.
 */
MALI_IMPORT int _mali_sys_printf(const char *format, ...);


/**
 * snprintf wrapper.
 * A failsafe snprintf wrapper. The function is like a printf that prints
 * its contents to a text string with a given maximum length.
 * We guarantee that the last byte written will be a Zero byte, that
 * terminates the text string, and that the total number of bytes written
 * will never exceed size bytes. This guarantee also implies that the
 * maximum letters written to the string is (size-1), before the
 * Zero-termination.
 * @param str The string that this function writes to.
 * @param size Number of bytes in the buffer.
 * @param format The input text string that formats the output string.
 * @param ... Input parameters for the input text string.
 */
MALI_IMPORT int _mali_sys_snprintf(char *str, u32 size, const char *format, ...);

/**
 * Sort elements of an array.
 * Sort the \a num elements of the array given by \a base, each element being
 * \a size bytes long.
 *
 * Comparison between two elements has to be performed by the given \a compare
 * function pointer. \a compare must return a negative, zero or positive value
 * to indicate if its first argument is considered to be less, equal or greater
 * than its second argument.
 * @param base The array to sort
 * @param num Number of elements in the array
 * @param size Size of each element
 * @param compare Function pointer to compare function
 */
MALI_IMPORT void _mali_sys_qsort(void *base, u32 num, u32 size, int (*compare)(const void *, const void *));

/**
 * Yield the processor from the currently executing thread to another ready to run,
 * active thread of equal or higher priority
 * Function will return if there are no such threads, and continue to run
 * @return MALI_ERR_NO_ERROR if successful, or MALI_ERR_FUNCTION_FAILED on failure
 */
MALI_IMPORT mali_err_code _mali_sys_yield(void);

/**
 * A sleep function.
 * Takes the number of microseconds to sleep.
 * The function will sleep AT LEAST usec microseconds.
 * The sleep duration might be longer due to scheduling on the system.
 * @param usec Number of microseconds to sleep
 */
MALI_IMPORT void _mali_sys_usleep(u64 usec);

/**
 * Get a timer with at most, microseconds precision.
 *
 * Intended usage is instrumented functions.
 *
 * The function is a wrapper to get the time from the system.
 * It either has the precision of microseconds, or the precision that is
 * supported by the underlying system - whichever is lower.
 *
 * If the system does not have a timer the function returns Zero. The function
 * only returns Zero when it fails.
 *
 * The returned value will be microseconds since some platform-dependant date,
 * and so cannot be used as a platform independent way of determining the current
 * time/date. Only the difference in time between two events may be measured.
 *
 * @return A 64bit counter with microsecond precision, or zero.
 */
MALI_IMPORT u64 _mali_sys_get_time_usec(void);

/**
 * Program will exit if this function is called. Function is called from
 * assert-macro in mali_debug.h.
 */
/* coverity[+kill] */
MALI_IMPORT MALI_NORETURN void _mali_sys_abort(void);

/**
 * Sets breakpoint at point where function is called.
 */
MALI_IMPORT void _mali_sys_break(void);

/**
 * Converts the initial part of the string in nptr to an unsigned 64 bit integer value
 * according to the given base, which must be between 2 and 36 inclusive or be
 * the special value 0.
 * @param nptr Character string to convert
 * @param endptr Pointer to the character that ended the scan or a null value
 * @param base Specifies the base to use for the conversion.
 */
MALI_IMPORT u64 _mali_sys_strtoull(const char *nptr, char **endptr, u32 base);

/**
 * Converts the initial part of the string in nptr to a signed 64 bit integer value
 * according to the given base, which must be between 2 and 36 inclusive or be
 * the special value 0.
 * @param nptr Character string to convert
 * @param endptr Pointer to the character that ended the scan or a null value
 * @param base Specifies the base to use for the conversion.
 */
MALI_IMPORT s64 _mali_sys_strtoll(const char *nptr, char **endptr, u32 base);

/**
 * Converts the initial portion of the string pointed to by nptr to a double representation
 * @param nptr Character string to convert
 * @param endptr Pointer to the character that ended the scan or a null value
 */
MALI_IMPORT double _mali_sys_strtod(const char *nptr, char **endptr);

/**
 * Returns the length of str
 * @param str C string
 */

MALI_IMPORT u32 _mali_sys_strlen(const char *str);

/**
 * Returns the length of str
 * @param str C string
 * @param count Max length of the string
 */
MALI_IMPORT u32 _mali_sys_strnlen(const char *str, u32 count);

/**
 * Performs a lexical comparison of str1 and str2 up to n characters
 * @param str1 C string
 * @param str2 C string
 * @param n    Maximum number of characters to compare
 * @return If str1==str2, returns 0. If str1<str2, returns value<0, if str1>str2, returns >0.
 */

MALI_IMPORT u32 _mali_sys_strncmp(const char *str1, const char *str2, u32 n);

/**
 * Performs a copy of up to n chars from src to dest
 */

MALI_IMPORT char *_mali_sys_strncpy(char *dest, const char *src, u32 n);

/**
 * String concatenation, limited to n bytes
 * Appends the src string to the destination string, limited to maximum n bytes
 * @param dest The string to append to
 * @param src The string to append
 * @param n The maximum bytes to add to dest (or: maximum bytes to read from src)
 * @return The destination string
 */
MALI_IMPORT char *_mali_sys_strncat(char *dest, const char *src, u32 n);

/**
 * Converts a string into integer
 * @param str The string to be converted
 * @return The integer value of the string
 */
MALI_IMPORT s32 _mali_sys_atoi(const char *str);

/*
 *  Math functions
 */

/**
 *  Square root
 *  @param value    Value to take the square root of
 *  @return         The square root
 */
MALI_IMPORT float _mali_sys_sqrt(float value);

/**
 *  Sine
 *  @param value    Value to take the sine of
 *  @return         The sine
 */
MALI_IMPORT float _mali_sys_sin(float value);

/**
 *  Cosine
 *  @param value    Value to take the cosine of
 *  @return         The cosine
 */
MALI_IMPORT float _mali_sys_cos(float value);

/**
 *      Arc tangent
 *      @param y y-coordinate
 *      @param x x-coordinate
 *      @return  Principal arc tangent of y/x in the range [-pi,pi] (radians)
 */
MALI_IMPORT float _mali_sys_atan2(float y, float x);


/**
 *  Ceil - Round up
 *  @param value    Value to take the ceil of
 *  @return         The ceiled value
 */
MALI_IMPORT float _mali_sys_ceil(float value);

/**
 *  Floor - Round down
 *  @param value    Value to round down
 *  @return         The floored value
 */
MALI_IMPORT float _mali_sys_floor(float value);

/**
 *  Absolute value
 *  @param value    Value to take the absolute value of
 *  @return         The absolute value
 */
MALI_IMPORT float _mali_sys_fabs(float value);

/**
 * Absolute value
 * @param value Value to take the absolute value of
 * @return The absolute value
 */
MALI_IMPORT s32 _mali_sys_abs(s32 value);

/**
 * Absolute value
 * @param value Value to take the absolute value of
 * @return The absolute value
 */
MALI_IMPORT s64 _mali_sys_abs64(s64 value);


/**
 *  Exponent
 *  @param value    Value to take the exponent of
 *  @return         The exponent
 */
MALI_IMPORT float _mali_sys_exp(float value);

/**
 *  Power function
 *  @param base     Base value to raise to a power
 *  @param exponent Exponent to which to raise the base
 *  @return         The value of base to the power of exponent
 */
MALI_IMPORT float _mali_sys_pow(float base, float exponent);

/**
 *  Natural Logarithm function
 *  @param value    Value to find logarithm of
 *  @return         Log to base e of value
 */
MALI_IMPORT float _mali_sys_log(float value);

/**
 *  Base 2 Logarithm function
 *  @param value    Value to find logarithm of
 *  @return         Log to base 2 of value
 */
MALI_IMPORT float _mali_sys_log2(float value);

/**
 *  Judge whether the input is a NaN
 *  @param value    Value to be judged
 *  @return         If value has NaN, return non-zero value
 */
MALI_IMPORT int _mali_sys_isnan(float value);

/**
 *  Floating point modulus function
 *  @param n The value to take the modulus of
 *  @param m The modulus
 *  @return n - x*2 for some x so that the restult is smaller than m
 */
MALI_IMPORT float _mali_sys_fmodf(float n, float m);

/*
 * Atomic integer operations
 */

/**
 * Atomic integer increment
 * @param atomic Pointer to mali_atomic_int to atomically increment
 */
MALI_IMPORT void _mali_sys_atomic_inc(mali_atomic_int *atomic);

/**
 * Atomic integer increment and return post-increment value
 * @param atomic Pointer to mali_atomic_int to atomically increment
 * @return Value of val after increment
 */
MALI_IMPORT u32 _mali_sys_atomic_inc_and_return(mali_atomic_int *atomic);

/**
 * Atomic integer decrement
 * @param atomic Pointer to mali_atomic_int to atomically decrement
 */
MALI_IMPORT void _mali_sys_atomic_dec(mali_atomic_int *atomic);

/**
 * Atomic integer decrement and return post-decrement value
 * @param atomic Pointer to mali_atomic_int to atomically decrement
 * @return Value of val after decrement
 */
MALI_IMPORT u32 _mali_sys_atomic_dec_and_return(mali_atomic_int *atomic);

/**
 * Atomic integer get. Read value stored in the atomic integer.
 * @param atomic Pointer to mali_atomic_int to read contents of
 * @return Value currently in mali_atomic_int
 * @note If an increment/decrement/set is currently in progress you'll get either the pre- or post-modification value.
 */
MALI_IMPORT u32 _mali_sys_atomic_get(mali_atomic_int *atomic);

/**
 * Atomic integer set. Used to explicitly set the contents of an atomic integer to a specific value.
 * @param atomic Pointer to mali_atomic_int to set
 * @param value Value to write to the mali_atomic_int
 * @note If increment or decrement is currently in progress the mali_atomic_int will contain either the value set, or the post-increment/decrement value.
 * @note If another set is currently in progress it's uncertain which will persist when the function returns.
 */
MALI_IMPORT void _mali_sys_atomic_set(mali_atomic_int *atomic, u32 value);

/**
 * Initialize an atomic integer. Used to initialize an atomic integer to a sane value when created.
 * @param atomic Pointer to mali_atomic_int to set
 * @param value Value to initialize the atomic integer to
 * @note This function has to be called when a mali_atomic_int is created/before first use
 */
MALI_IMPORT void _mali_sys_atomic_initialize(mali_atomic_int *atomic, u32 value);

MALI_IMPORT u32 _mali_sys_rand(void);

MALI_IMPORT PidType _mali_sys_get_pid(void);

MALI_IMPORT PidType _mali_sys_get_tid(void);

/*
 * File operations
 */

typedef void mali_file;

/**
 * A wrapper for remove.
 * This works like the standard remove.
 * \param filename the path to and name of the file to be opened, in the notation of the
 *      underlying operating system
 * \return Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned.
 */
MALI_IMPORT int _mali_sys_remove(const char *filename);

/**
 * Open a file. Same semantics (and probably wraps to) libc's fopen().
 */
MALI_IMPORT mali_file *_mali_sys_fopen(const char *path, const char *mode) MALI_CHECK_RESULT;

/**
 * Close a file. Same semantics (and probably wraps to) libc's fclose().
 */
MALI_IMPORT void _mali_sys_fclose(mali_file *file);

/**
 * Print to file. Same semantics (and probably wraps to) libc's fprintf().
 */
MALI_IMPORT int _mali_sys_fprintf(mali_file *file, const char *format, ...);

/**
 * Write to binary file. Same semantics (and probably wraps to) libc's fwrite().
 * \param data Pointer to the memory we want to write from
 * \param element_size Size of the element we want to print
 * \param element_number Number of elements of element_size that should be written. Usually 1
 * \param file The file function writes to. Opened with _mali_sys_fopen
 * \return Number of elements written. If this is unequal to \a element_number there was an error.
 */
MALI_IMPORT u32 _mali_sys_fwrite(const void *data, u32 element_size, u32 element_number, mali_file *file);

/**
 * Load (extra) configuration variables.
 * Some platforms do not have a built-in method for specifying config variables
 * for a specific application. This function will make sure the application specific
 * configuration variables are loaded. All variables loaded can afterwards be
 * accessed through _mali_sys_config_string_get() and related functions.
 *      @return                 void
 */
MALI_IMPORT void _mali_sys_load_config_strings(void);

/**
 * Get text string from the OS.
 * The intended use is to be able to read environment variables set in the OS.
 * This will make it easier to configure the programs and drivers without doing it
 * through configuration files or recompilation.
 *      @note You must free the string you get by a call to \a _mali_sys_config_string_release()
 *      @param name             The name of the variable we want to read
 *      @return                 The string with text if the name existed, or NULL.
 */
MALI_IMPORT const char *_mali_sys_config_string_get(const char *name);

/**
 * Set (or unset) text string in the OS environment.
 * The intended use is to be able to set environment variables in the OS.
 * This will make it easier to configure the programs and drivers without doing it
 * through configuration files or recompilation.
 *      @param name             The name of the variable we want to set
 *      @param value            The string with text or NULL to unset the variable
 *      @return                 MALI_TRUE on success, MALI_FALSE otherwise
 */
MALI_IMPORT mali_bool _mali_sys_config_string_set(const char *name, const char *value);

/**
 * Free allocated text string from the OS.
 * The function frees a text string allocated by \a _mali_sys_config_string_get()
 * It is legal to input a NULL-pointer.
 *      @param env_var  The env_var returned from \a _mali_sys_config_string_get()
 */
MALI_IMPORT void _mali_sys_config_string_release(const char *env_var);

/**
 * Get an integer value from the OS.
 * The intended use is to be able to read environment variables set in the OS.
 * This will make it easier to configure the programs and drivers without doing it
 * through configuration files or recompilation.
 * This function might be slow, and should therefore only be used during
 * initialization of the system.
 *      @param name           The name of the variable we want to read
 *      @param default_val    Value returned if the variable \a name is not found.
 *      @param min_val        Minimum value the function will return
 *      @param max_val        Maximum value this function will return
 *      @return               The value of \a name if exist, or \a default_val.
 */
MALI_IMPORT s64 _mali_sys_config_string_get_s64(const char *name, s64 default_val, s64 min_val, s64 max_val);

/**
 * Get a boolean value from the OS.
 * The intended use is to be able to read environment variables set in the OS.
 * The OS environment variable should have the value "TRUE" or "FALSE".
 * The function also support "1" as true and "0" as false.
 * If the OS environment variable does not exist or has another value, the
 * function will return the \a default_val input parameter.
 * This function might be slow, and should therefore only be used during
 * initialization of the system.
 *      @param name           The name of the variable we want to read
 *      @param default_val    Value returned if the variable \a name is not found.
 *      @return               The value of \a name if exist, or \a default_val.
 */
MALI_IMPORT mali_bool _mali_sys_config_string_get_bool(const char *name, mali_bool default_val);


typedef void mali_library;

/**
 * Dynamically load a shared library.
 * @param name Name of library file to load.
 * @return A handle to the loaded library, or NULL for failure.
 */
MALI_IMPORT mali_library *_mali_sys_library_load(const char *name) MALI_CHECK_RESULT;

/**
 * Unload a previously dynamically loaded shared library.
 * @param handle Handle to the library to unload (as returned from _mali_sys_library_load).
 */
MALI_IMPORT void _mali_sys_library_unload(mali_library *handle);

/**
 * Calls the specific initialization function inside the specified library
 *
 * The initialization function is declared as;
 *     int mali_library_init(void* arg);
 *
 * For operating systems relying on ordinals, this function must be the first one.
 *
 * @param handle Handle to dynimic library to initialize.
 * @param param Pointer passed to initialization funcion within library.
 * @param retval value returned by the init function.
 * @return MALI_TRUE if init function was called, otherwise MALI_FALSE.
 */
MALI_IMPORT mali_bool _mali_sys_library_init(mali_library *handle, void *param, u32 *retval) MALI_CHECK_RESULT;


/* for indirect version do not use optimized memcpy functions */
#define _mali_sys_memcpy_32(destination, source, num) _mali_sys_memcpy(destination, source, num)
#define _mali_sys_memcpy_cpu_to_mali_32(destination, source, num, element_size) _mali_sys_memcpy_cpu_to_mali(destination, source, num, element_size)
#define _mali_sys_memcpy_mali_to_cpu_32(destination, source, num, element_size) _mali_sys_memcpy_mali_to_cpu(destination, source, num, element_size)
#define _mali_sys_memcpy_mali_to_mali_32(destination, source, num, element_size) _mali_sys_memcpy_mali_to_mali(destination, source, num, element_size)

#define _mali_sys_memcpy_runtime_32(destination, source, num) _mali_sys_memcpy(destination, source, num)
#define _mali_sys_memcpy_cpu_to_mali_runtime_32(destination, source, num, element_size) _mali_sys_memcpy_cpu_to_mali(destination, source, num, element_size)
#define _mali_sys_memcpy_mali_to_cpu_runtime_32(destination, source, num, element_size) _mali_sys_memcpy_mali_to_cpu(destination, source, num, element_size)
#define _mali_sys_memcpy_mali_to_mali_runtime_32(destination, source, num, element_size) _mali_sys_memcpy_mali_to_mali(destination, source, num, element_size)


#ifdef __cplusplus
}
#endif

#endif /* _MALI_RUNTIME_H_ */
