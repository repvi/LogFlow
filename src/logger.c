#include "logger.h"
#include "genList.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h> // for malloc/free

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define ALIGNOF(type) _Alignof(type)
#else
    #define ALIGNOF(type) __alignof__(type)
#endif

// Set buffer alignment as required by your hardware (DMA, cache line, etc.)
#define BUFFER_ALIGNMENT 8

// Aligns pointer p up to the next multiple of align (align must be power of 2)
#define ALIGN_PTR(p, align) ((uintptr_t)(((uintptr_t)(p) + ((align)-1)) & ~((uintptr_t)(align)-1)))

// Platform-specific malloc/free
#if defined(__XTENSA__)
#include "esp_system.h"
#include "esp_heap_caps.h"

void *mallocv(int size) 
{ 
    return heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_DMA); 
}

void freev(void *object) 
{ 
    heap_caps_free(object); 
}
#else
void *mallocv(int size) 
{ 
    return malloc(size); 
}
void freev(void *object) 
{
    free(object); 
}
#endif

typedef struct page_list {
    page_type_t type;
    int remaining;
    char *buffer;
    struct list_head list;
} page_list;

struct logger_t {
    int page_buffer_size;
    int total_pages;
    page_list pages;
};

// --- Alignment-aware block size calculation ---
#define LOGGER_SIZE_BASE (sizeof(struct logger_t))

// Each block: aligned page_list + aligned buffer (with padding)
#define LOGGER_BLOCK_SIZE(page_size) \
    (ALIGN_PTR(sizeof(page_list), ALIGNOF(page_list)) \
    + ALIGN_PTR(page_size, BUFFER_ALIGNMENT))

// Total allocation size
#define LOGGER_ALLOC_SIZE(pages, size) \
    (LOGGER_SIZE_BASE + (pages) * LOGGER_BLOCK_SIZE(size) + BUFFER_ALIGNMENT)

// --- List manipulation helpers (assume Linux-style list_head) ---
static void logger_page_add(page_list *main, page_list *page) 
{
    list_add_tail(&page->list, &main->list);
}

// --- Page initialization with correct alignment ---
static void page_init(page_list *pages, uint8_t *memory, int page_amount, int page_size)
{
    const size_t entry_align = ALIGNOF(page_list);
    uintptr_t mem_addr = ALIGN_PTR(memory, entry_align);
    memory = (uint8_t *)mem_addr;

    INIT_LIST_HEAD(&pages->list);

    const size_t block_size = LOGGER_BLOCK_SIZE(page_size);
    const uintptr_t memory_end = (uintptr_t)memory + (page_amount + 2) * block_size;

    for (int i = 0; i < page_amount; i++) {
        // Align each block for page_list
        uintptr_t block = (uintptr_t)memory + i * block_size;
        page_list *new_page = (page_list *)ALIGN_PTR(block, entry_align);

        // Align buffer after page_list struct
        uintptr_t buf_start = ALIGN_PTR((uintptr_t)(new_page + 1), BUFFER_ALIGNMENT);
        new_page->buffer = (char *)buf_start;
        new_page->buffer[0] = '\0'; // Initialize first byte
        new_page->buffer[1] = '\0'; // Initialize second byte
        new_page->remaining = page_size;
        new_page->type = PAGE_TYPE_DEFAULT;

        uintptr_t buffer_end = (uintptr_t)new_page->buffer + page_size;
        assert(buffer_end <= memory_end);

        logger_page_add(pages, new_page);
    }
}

// --- Logger creation ---
LoggerHandler logger_create(int page_amount, int page_size) 
{
    if (page_amount <= 0 || page_size <= 0) {
        return NULL; // Invalid parameters
    }

    if (page_size < 2) {
        page_size = 2; // Ensure minimum size for buffer
    }

    const size_t block_size = LOGGER_BLOCK_SIZE(page_size);
    const size_t alloc_size = LOGGER_ALLOC_SIZE(page_amount, page_size);

    void *memory = mallocv(alloc_size);
    if (memory != NULL) {
        LoggerHandler logger = (LoggerHandler)memory;
        logger->page_buffer_size = page_size;
        logger->total_pages = page_amount;

        uintptr_t raw = (uintptr_t)((unsigned char *)logger + LOGGER_SIZE_BASE);
        uintptr_t aligned = ALIGN_PTR(raw, ALIGNOF(page_list));
        uint8_t *ptr = (uint8_t *)aligned;

        page_init(&logger->pages, ptr, page_amount, page_size);
        return logger;
    } 
    else {
        return NULL;
    }
}

void logger_print_page(LoggerHandler logger, int page_index)
{
    page_list *current, *tmp;
    int index = 0;

    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        if (index == page_index) {
            printf("Page %d: %s\n", index, current->buffer);
            return;
        }
        index++;
    }
}

void logger_print_all(LoggerHandler logger) 
{
    struct page_list *current, *tmp;
    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        printf("remaining: %i", current->remaining);
        printf("---[");
        printf("%s", current->buffer);
        printf("]---\n");
    }
}

// --- Logger destroy ---
void logger_destroy(LoggerHandler logger) 
{
    freev(logger);
}

// --- Logger debug dump ---
__attribute__((deprecated("This is a debug function and may not be suitable for production use.")))
void logger_debug_dump(LoggerHandler logger) 
{
    page_list *current, *tmp;
    int index = 0;

    printf("🔍 Logger Memory Map Dump\n");

    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        uintptr_t entry_addr   = (uintptr_t)current;
        uintptr_t buffer_addr  = (uintptr_t)current->buffer;
        uintptr_t buffer_end   = buffer_addr + logger->page_buffer_size;

        printf("Page %d\n", index);
        printf("  Entry address : %p\n", (void *)entry_addr);
        printf("  Buffer address: %p\n", (void *)buffer_addr);
        printf("  Buffer end    : %p\n", (void *)buffer_end);

        if (entry_addr % ALIGNOF(page_list) != 0)
            printf("Misaligned page_list struct!\n");

        if (buffer_addr % BUFFER_ALIGNMENT != 0)
            printf("Misaligned buffer!\n");

        // Overlap detection
        if ((void *)tmp != (void *)(&logger->pages.list)) {
            uintptr_t next_entry = (uintptr_t)tmp;
            if (buffer_end > next_entry) {
                printf("     Overlap detected with next page!\n");
                printf("     Buffer end: %p > Next entry: %p\n",
                       (void *)buffer_end, (void *)next_entry);
            }
        }
        index++;
    }
    printf("  Dump complete: %d pages checked.\n", index);
}

static int __logger_add_data_helper(LoggerHandler logger, const char *data, int size, int index, const char end)
{
    if (index < logger->total_pages && index >= 0) {
        if (size <= 0) {
            size = strlen(data);
        }

        page_list *current, *tmp;
        int page_index = 0;

        list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
            if (page_index == index) {
                int offset = logger->page_buffer_size - current->remaining;
                if (size > current->remaining) {
                    size = current->remaining; // Limit size to remaining space
                }

                if (end != '\0') {
                    current->buffer[offset + size] = end; // Add end character if provided
                    current->buffer[offset + size + 1] = '\0'; // Null-terminate the string
                    current->remaining -= (size + 1); // Adjust remaining space
                }
                else {
                    current->buffer[offset + size] = '\0'; // Null-terminate the string
                    current->remaining -= size;
                }
                memcpy(current->buffer + offset, data, size);
                return size;
            }
            page_index++;
        }
    }
    return -1;
}

int logger_save_to_page(LoggerHandler logger, const char *data, int size, int index)
{
    return __logger_add_data_helper(logger, data, size, index, '\0');
}

int logger_save_to_page_line(LoggerHandler logger, const char *data, int size, int index)
{
    return __logger_add_data_helper(logger, data, size, index, '\n');
}

int logger_set_page_type(LoggerHandler logger, int page_index, page_type_t type)
{
    page_list *current, *tmp;
    int index = 0;

    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        if (index == page_index) {
            current->type = type;
            return 0; // Success
        }
        index++;
    }
    return -1;
}

char *logger_get_page_buffer(LoggerHandler logger, int page_index)
{
    page_list *current, *tmp;
    int index = 0;

    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        if (index == page_index) {
            return current->buffer; // Return the buffer of the specified page
        }
        index++;
    }
    return NULL; // Page not found
}

void logger_clear_page(LoggerHandler logger, int page_index)
{
    page_list *current, *tmp;
    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        if (page_index == 0) {
            memset(current->buffer, 0, logger->page_buffer_size);
            current->remaining = logger->page_buffer_size; // Reset remaining space
            current->type = PAGE_TYPE_DEFAULT; // Reset type
            return;
        }
        page_index--;
    }
}

void logger_clear_all(LoggerHandler logger)
{
    page_list *current, *tmp;
    list_for_each_entry_safe(current, tmp, &logger->pages.list, list) {
        current->buffer[0] = '\0'; // Clear first byte
        current->buffer[1] = '\0'; // Clear second byte
        current->remaining = logger->page_buffer_size; // Reset remaining space
        current->type = PAGE_TYPE_DEFAULT; // Reset type
    }
}