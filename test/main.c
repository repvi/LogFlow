#include "logger.h"
#include "stdio.h"

// cmake .. -G "MinGW Makefiles"
// cmake --build . --config Debug
// ./Debug/LoggerTest.exe
int main() {
    // Example usage of the logger_create function
    int page_amount = 6; // Number of pages
    int page_size = 1024; // Size of each page in bytes

    LoggerHandler logger = logger_create(page_amount, page_size);
    if (logger == NULL) {
        return -1; // Handle error
    }

    // Use the logger...
    printf("New\n");

    printf("Logger created successfully with %d pages of size %d bytes each.\n", page_amount, page_size);
    
    logger_clear_all(logger);
    logger_save_to_page(logger, "Hello, World!", 13, 0);
    logger_save_to_page(logger, "Hello, World!", 13, 1);
    logger_save_to_page_line(logger, "This is a test line.", 20, 1);
    logger_save_to_page_line(logger, "This is another test line.", 26, 1);
    logger_save_to_page(logger, "This is a test.", 15, 1);
    printf("Saved data to page 1 and 0.\n");
    logger_print_page(logger, 0);
    logger_print_all(logger);
    //logger_debug_dump(logger);
    printf("Destroying...");
    logger_destroy(logger);
    return 0; // Success
}