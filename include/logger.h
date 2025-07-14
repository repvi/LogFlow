#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum page_type_t
 * @brief Defines the type/severity level of a log page
 */
typedef enum {
    PAGE_TYPE_ERROR = -1,      /**< Error level logs */
    PAGE_TYPE_DEFAULT = 0,     /**< Default log level */
    PAGE_TYPE_INFO,            /**< Informational logs */
    PAGE_TYPE_INFO_DEBUG,      /**< Debug information logs */
    PAGE_TYPE_WARNING,         /**< Warning level logs */
} page_type_t;

typedef enum {
    LOGGER_DEFAULT = 0,
    LOGGER_FLUSH
} logger_command_t;

/**
 * @struct logger_t
 * @brief Internal logger structure
 */
typedef struct logger_t logger_t;

/**
 * @typedef LoggerHandler
 * @brief Handle to a logger instance
 */
typedef struct logger_t* LoggerHandler;

/**
 * @brief Creates a new logger with specified number of pages and page size
 * @param page_amount Number of pages to allocate
 * @param page_size Size of each page in bytes
 * @return Handle to the created logger or NULL on failure
 */
LoggerHandler logger_create(int page_amount, int page_size);

/**
 * @brief Destroys a logger and frees all associated resources
 * @param logger Logger to destroy
 */
void logger_destroy(LoggerHandler logger);

void logger_print_page_line(LoggerHandler logger, int page_index);

/**
 * @brief Prints the contents of a specific page to stdout
 * @param logger Logger instance
 * @param page_index Index of the page to print
 */
void logger_print_page(LoggerHandler logger, int page_index, logger_command_t command);

/**
 * @brief Prints the contents of all pages to stdout
 * @param logger Logger instance
 */
void logger_print_all(LoggerHandler logger);

/**
 * @brief Dumps detailed memory information about the logger (for debugging)
 * @param logger Logger instance
 * @note This function is deprecated and not recommended for production use
 */
void logger_debug_dump(LoggerHandler logger);

/**
 * @brief Saves data to a specific page
 * @param logger Logger instance
 * @param data Pointer to the data to save
 * @param size Size of data in bytes (if ≤0, strlen(data) is used)
 * @param index Page index to save to
 * @return Number of bytes written or -1 on error
 */
int logger_save_to_page(LoggerHandler logger, const char *data, int size, int index);

/**
 * @brief Saves data to a specific page and appends a newline
 * @param logger Logger instance
 * @param data Pointer to the data to save
 * @param size Size of data in bytes (if ≤0, strlen(data) is used)
 * @param index Page index to save to
 * @return Number of bytes written or -1 on error
 */
int logger_save_to_page_line(LoggerHandler logger, const char *data, int size, int index);

/**
 * @brief Sets the type/severity level of a page
 * @param logger Logger instance
 * @param page_index Index of the page to modify
 * @param type New type to assign to the page
 * @return 0 on success, -1 on error
 */
int logger_set_page_type(LoggerHandler logger, int page_index, page_type_t type);

/**
 * @brief Gets direct access to a page's buffer
 * @param logger Logger instance
 * @param page_index Index of the page to access
 * @return Pointer to the page buffer or NULL if page not found
 * @warning Use with caution as direct modifications may corrupt internal state
 */
char *logger_get_page_buffer(LoggerHandler logger, int page_index);

/**
 * @brief Clears a specific page, resetting it to empty
 * @param logger Logger instance
 * @param page_index Index of the page to clear
 */
void logger_flush_page(LoggerHandler logger, int page_index);

/**
 * @brief Clears all pages in the logger
 * @param logger Logger instance
 */
void logger_flush_all(LoggerHandler logger);

#ifdef __cplusplus
}
#endif