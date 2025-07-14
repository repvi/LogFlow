# LogFlow API Reference

Complete API documentation for the LogFlow paged logger library.

## Table of Contents

- [Data Types](#data-types)
- [Core Functions](#core-functions)
- [Page Management](#page-management)
- [Utility Functions](#utility-functions)
- [Error Codes](#error-codes)
- [Usage Examples](#usage-examples)

## Data Types

### page_type_t

Enumeration defining log level types for pages.

```c
typedef enum {
    PAGE_TYPE_ERROR = -1,      /**< Error level logs */
    PAGE_TYPE_DEFAULT = 0,     /**< Default log level */
    PAGE_TYPE_INFO,            /**< Informational logs */
    PAGE_TYPE_INFO_DEBUG,      /**< Debug information logs */
    PAGE_TYPE_WARNING,         /**< Warning level logs */
} page_type_t;
```

**Values:**
- `PAGE_TYPE_ERROR`: For critical error messages
- `PAGE_TYPE_DEFAULT`: Standard log entries
- `PAGE_TYPE_INFO`: Informational messages
- `PAGE_TYPE_INFO_DEBUG`: Debug information
- `PAGE_TYPE_WARNING`: Warning messages

### LoggerHandler

Opaque handle to a logger instance.

```c
typedef struct logger_t* LoggerHandler;
```

**Usage:**
- Pass to all logger functions
- Created by `logger_create()`
- Destroyed by `logger_destroy()`

## Core Functions

### logger_create

Creates a new logger instance with specified configuration.

```c
LoggerHandler logger_create(int page_amount, int page_size);
```

**Parameters:**
- `page_amount`: Number of pages to allocate (must be > 0)
- `page_size`: Size of each page buffer in bytes (must be > 0)

**Returns:**
- Valid `LoggerHandler` on success
- `NULL` on failure (memory allocation error or invalid parameters)

**Memory Usage:**
```
Total memory = sizeof(logger_t) + (page_amount * (sizeof(page_list) + page_size + alignment))
```

**Example:**
```c
// Create logger with 10 pages of 512 bytes each
LoggerHandler logger = logger_create(10, 512);
if (!logger) {
    fprintf(stderr, "Failed to create logger\n");
    return -1;
}
```

### logger_destroy

Destroys a logger instance and frees all associated memory.

```c
void logger_destroy(LoggerHandler logger);
```

**Parameters:**
- `logger`: Logger instance to destroy (can be NULL)

**Behavior:**
- Frees all allocated memory
- Safe to call with NULL pointer
- Logger handle becomes invalid after call

**Example:**
```c
logger_destroy(logger);
logger = NULL; // Good practice
```

### logger_save_to_page

Saves data to a specific page without adding line termination.

```c
int logger_save_to_page(LoggerHandler logger, const char *data, int size, int index);
```

**Parameters:**
- `logger`: Valid logger instance
- `data`: Pointer to data to save (must not be NULL)
- `size`: Size of data in bytes (if ≤ 0, `strlen(data)` is used)
- `index`: Page index (0-based, must be < page_amount)

**Returns:**
- Number of bytes written on success
- `-1` on error (invalid parameters, page full, or page not found)

**Behavior:**
- Appends data to existing page content
- Null-terminates the string
- Does not add newline character

**Example:**
```c
int written = logger_save_to_page(logger, "Error: Connection failed", -1, 0);
if (written < 0) {
    printf("Failed to save to page\n");
}
```

### logger_save_to_page_line

Saves data to a specific page and appends a newline character.

```c
int logger_save_to_page_line(LoggerHandler logger, const char *data, int size, int index);
```

**Parameters:**
- `logger`: Valid logger instance
- `data`: Pointer to data to save (must not be NULL)
- `size`: Size of data in bytes (if ≤ 0, `strlen(data)` is used)
- `index`: Page index (0-based, must be < page_amount)

**Returns:**
- Number of bytes written (including newline) on success
- `-1` on error

**Behavior:**
- Appends data + '\n' to page content
- Null-terminates the string
- Useful for line-based logging

**Example:**
```c
logger_save_to_page_line(logger, "System startup complete", -1, 0);
logger_save_to_page_line(logger, "Loading configuration", -1, 0);
```

## Page Management

### logger_set_page_type

Sets the log level type for a specific page.

```c
int logger_set_page_type(LoggerHandler logger, int page_index, page_type_t type);
```

**Parameters:**
- `logger`: Valid logger instance
- `page_index`: Page index (0-based)
- `type`: Log level type from `page_type_t` enum

**Returns:**
- `0` on success
- `-1` on error (invalid parameters or page not found)

**Example:**
```c
// Mark page 0 as containing error messages
if (logger_set_page_type(logger, 0, PAGE_TYPE_ERROR) == 0) {
    printf("Page type set successfully\n");
}
```

### logger_get_page_buffer

Gets direct access to a page's buffer for advanced operations.

```c
char *logger_get_page_buffer(LoggerHandler logger, int page_index);
```

**Parameters:**
- `logger`: Valid logger instance
- `page_index`: Page index (0-based)

**Returns:**
- Pointer to page buffer on success
- `NULL` if page not found or invalid parameters

**Warning:**
- Direct buffer access bypasses safety checks
- Ensure you don't write beyond buffer boundaries
- Use with caution to avoid corrupting internal state

**Example:**
```c
char *buffer = logger_get_page_buffer(logger, 0);
if (buffer) {
    // Direct buffer manipulation (use carefully)
    snprintf(buffer, page_size, "Direct write: %d", value);
}
```

### logger_clear_page

Clears a specific page, resetting it to empty state.

```c
void logger_clear_page(LoggerHandler logger, int page_index);
```

**Parameters:**
- `logger`: Valid logger instance
- `page_index`: Page index to clear (0-based)

**Behavior:**
- Resets page buffer to empty
- Preserves page type setting
- Safe to call on already empty pages

**Example:**
```c
// Clear page 3 for reuse
logger_clear_page(logger, 3);
```

### logger_clear_all

Clears all pages in the logger.

```c
void logger_clear_all(LoggerHandler logger);
```

**Parameters:**
- `logger`: Valid logger instance

**Behavior:**
- Clears all page buffers
- Preserves page type settings
- Efficient mass clearing operation

**Example:**
```c
// Clear all logs for new session
logger_clear_all(logger);
```

## Utility Functions

### logger_print_page

Prints the contents of a specific page to stdout.

```c
void logger_print_page(LoggerHandler logger, int page_index);
```

**Parameters:**
- `logger`: Valid logger instance
- `page_index`: Page index to print (0-based)

**Output Format:**
```
=== Page 0 (INFO) ===
[Page content here]
=====================
```

**Example:**
```c
// Print only error logs (assuming page 0 contains errors)
logger_print_page(logger, 0);
```

### logger_print_all

Prints the contents of all non-empty pages to stdout.

```c
void logger_print_all(LoggerHandler logger);
```

**Parameters:**
- `logger`: Valid logger instance

**Behavior:**
- Iterates through all pages
- Skips empty pages
- Shows page index and type for each page

**Example:**
```c
// Print complete log history
printf("=== Complete Log Dump ===\n");
logger_print_all(logger);
```

### logger_debug_dump

Dumps detailed memory layout information for debugging purposes.

```c
void logger_debug_dump(LoggerHandler logger);
```

**Parameters:**
- `logger`: Valid logger instance

**Output Includes:**
- Memory addresses and alignment
- Page structure details
- Buffer boundaries
- Overlap detection

**Warning:**
- This function is deprecated
- Not recommended for production use
- Output format may change between versions

**Example:**
```c
#ifdef DEBUG
    logger_debug_dump(logger); // Only in debug builds
#endif
```

## Error Codes

### Return Value Conventions

| Return Value | Meaning |
|--------------|---------|
| `> 0` | Success (number of bytes written) |
| `0` | Success (no data written or operation completed) |
| `-1` | Error (see specific function documentation) |

### Common Error Conditions

1. **NULL Pointer**: Passing NULL for required parameters
2. **Invalid Index**: Page index out of range
3. **Memory Allocation**: Insufficient memory during creation
4. **Buffer Full**: Attempting to write to full page
5. **Invalid Size**: Negative size values where not allowed

## Usage Examples

### Basic Logging

```c
#include "logger.h"

int main() {
    LoggerHandler logger = logger_create(5, 256);
    
    // Log startup sequence
    logger_save_to_page_line(logger, "System initializing...", -1, 0);
    logger_save_to_page_line(logger, "Loading drivers", -1, 0);
    logger_save_to_page_line(logger, "Starting services", -1, 0);
    
    logger_set_page_type(logger, 0, PAGE_TYPE_INFO);
    logger_print_all(logger);
    
    logger_destroy(logger);
    return 0;
}
```

### Error Logging with Categorization

```c
void log_error(LoggerHandler logger, const char *component, const char *message) {
    char formatted[256];
    snprintf(formatted, sizeof(formatted), "[ERROR][%s] %s", component, message);
    
    // Use page 0 for errors
    logger_save_to_page_line(logger, formatted, -1, 0);
    logger_set_page_type(logger, 0, PAGE_TYPE_ERROR);
}

void log_info(LoggerHandler logger, const char *component, const char *message) {
    char formatted[256];
    snprintf(formatted, sizeof(formatted), "[INFO][%s] %s", component, message);
    
    // Use page 1 for info
    logger_save_to_page_line(logger, formatted, -1, 1);
    logger_set_page_type(logger, 1, PAGE_TYPE_INFO);
}

// Usage
LoggerHandler logger = logger_create(3, 512);
log_error(logger, "NETWORK", "Connection timeout");
log_info(logger, "SYSTEM", "Service started successfully");
```

### Sensor Data Logging

```c
typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
} sensor_data_t;

void log_sensor_data(LoggerHandler logger, const sensor_data_t *data, int page_index) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), 
             "T:%.1f°C H:%.1f%% @%u", 
             data->temperature, data->humidity, data->timestamp);
    
    logger_save_to_page_line(logger, buffer, -1, page_index);
    logger_set_page_type(logger, page_index, PAGE_TYPE_INFO_DEBUG);
}

// Usage
LoggerHandler logger = logger_create(10, 256);
sensor_data_t reading = {23.5, 45.2, 1234567890};
log_sensor_data(logger, &reading, 0);
```

### Circular Page Usage

```c
typedef struct {
    LoggerHandler logger;
    int current_page;
    int max_pages;
} circular_logger_t;

void circular_log(circular_logger_t *circ, const char *message) {
    // Use pages in circular fashion
    logger_save_to_page_line(circ->logger, message, -1, circ->current_page);
    
    circ->current_page = (circ->current_page + 1) % circ->max_pages;
    
    // Clear the next page for fresh data
    logger_clear_page(circ->logger, circ->current_page);
}

// Usage
circular_logger_t circ = {
    .logger = logger_create(8, 256),
    .current_page = 0,
    .max_pages = 8
};
```

### Performance Monitoring

```c
#include <time.h>

void performance_test(LoggerHandler logger) {
    clock_t start = clock();
    
    // Write 1000 log entries
    for (int i = 0; i < 1000; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Log entry %d", i);
        logger_save_to_page_line(logger, msg, -1, i % 5);
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Logged 1000 entries in %.3f seconds\n", time_spent);
    printf("Rate: %.0f entries/second\n", 1000.0 / time_spent);
}
```

## Memory Management Notes

### Alignment Requirements

The library ensures proper memory alignment for optimal performance:

- Page structures are aligned to their natural boundaries
- Buffers are aligned to `BUFFER_ALIGNMENT` (default: 8 bytes)
- Total memory usage includes alignment padding

### Memory Layout

```
Logger Memory Layout:
┌──────────────────┐ ← logger_t structure
├──────────────────┤
│ Page 0 metadata  │ ← page_list structure (aligned)
│ Page 0 buffer    │ ← Buffer data (aligned)
├──────────────────┤
│ Page 1 metadata  │
│ Page 1 buffer    │
├──────────────────┤
│       ...        │
└──────────────────┘
```

### Platform-Specific Allocations

```c
// ESP32/ESP-IDF
#ifdef __XTENSA__
void *mallocv(int size) {
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}
#else
// Standard C
void *mallocv(int size) {
    return malloc(size);
}
#endif
```

## Thread Safety

The current implementation is **not thread-safe**. For multi-threaded applications:

1. **External Synchronization**: Use mutexes around logger calls
2. **Per-Thread Loggers**: Create separate logger instances per thread
3. **Message Queues**: Use a producer-consumer pattern with thread-safe queues

### Example Thread-Safe Wrapper

```c
#include <pthread.h>

typedef struct {
    LoggerHandler logger;
    pthread_mutex_t mutex;
} thread_safe_logger_t;

int ts_logger_save_line(thread_safe_logger_t *ts_logger, 
                       const char *data, int size, int index) {
    pthread_mutex_lock(&ts_logger->mutex);
    int result = logger_save_to_page_line(ts_logger->logger, data, size, index);
    pthread_mutex_unlock(&ts_logger->mutex);
    return result;
}
```

---

## Version History

### v1.0.0
- Initial API release
- Cross-platform support
- Memory-aligned page management
- Complete function set

---

For additional help, examples, or to report issues, visit the [GitHub repository](https://github.com/repvi/LogFlow).
