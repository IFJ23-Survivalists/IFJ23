/**
 * @file string.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief A module for managing dynamically allocated strings in C.
 */

#ifndef STRING_H
#define STRING_H

#include <strings.h>
#include "error.h"

/**
 * @struct String
 * @brief A struct representing a dynamically allocated string.
 *
 * This struct contains a dynamically allocated character array to store the string data and a length
 * field to keep track of the string's length.
 * The inner data is is null-terminated, making it compatible with
 * standard C string functions from `<string.h>`.
 */
typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} String;

/**
 * @brief Initialize an empty string.
 *
 * This function initializes an empty string, but does not allocate data yet.
 *
 * @param[out] str The String struct to initialize.
 */
void string_init(String *str);


/**
 * @brief Free the memory used by the string.
 *
 * This function releases the dynamically allocated memory used by the string.
 *
 * @param[in,out] str The String struct to free.
 */
void string_free(String *str);

/**
 * @brief Clear the contents of the string, making it empty.
 *
 * This function clears the contents of the string, making it an empty string.
 * It does not deallocate the underlying memory, so you can continue using the string without reserving space for it
 * To both clear the contents and deallocate memory, use `string_free`.
 *
 * @param[in,out] str The String struct to clear.
 */
void string_clear(String *str);

/**
 * @brief Get the length of the string.
 *
 * This function returns the length of the string, which is the number of characters in the string
 * (excluding the null-terminator).
 *
 * @param[in] str The String struct to examine.
 * @return The length of the string.
 */
size_t string_len(String *str);

/**
 * @brief Reserve memory for the string to accommodate a specified capacity.
 *
 * This function reserves memory for the string to accommodate a specified capacity.
 * If the requested capacity is less than the current capacity, no action is taken.
 *
 * @param[in,out] str The String struct to reserve memory for.
 * @param[in] capacity The desired capacity to reserve.
 */
void string_reserve(String *str, size_t capacity);

/**
 * @brief Append a character to the end of the string.
 *
 * This function appends the provided character to the end of the string.
 * If memory allocation fails during the operation, the error is set to InternalError.
 *
 * @param[in,out] str The String struct to modify.
 * @param[in] ch The character to append to the string.
 */
void string_push(String *str, char ch);

String string_from_c_str(const char *str);

/**
 * @brief Create a new string with the data from the given string and empty the given string.
 *
 * This function creates a new string with the data from the given string and leaves the given
 * string empty. The caller is responsible for managing the memory of the new string.
 *
 * @param[in,out] str The String struct to take data from and empty.
 * @return A new String struct containing the data from the original string.
 */
String string_take(String *str);

/**
 * @brief Create a new string by cloning the given string.
 *
 * This function creates a new string by cloning the data from the given string.
 * If memory allocation fails during cloning, the error is set to InternalError.
 *
 * @param[in] str The String struct to clone.
 * @return A new String struct containing a copy of the data from the original string.
 *         If memory allocation fails, the error is set to InternalError.
 */
String string_clone(String *str);

void string_remove_ident(String *str, int ident_level);

#endif
