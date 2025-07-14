# LogFlow - Simple Paged Logger Library

A lightweight, memory-efficient logging system that stores and retrieves logs in a paged manner. Designed for embedded systems and general-purpose applications with focus on memory optimization and cross-platform compatibility.

## Features

- **Paged Memory Management**: Organizes logs into fixed-size pages for efficient memory usage
- **Cross-Platform**: Works on ESP32 (ESP-IDF), Arduino, and standard C environments
- **Type-Safe Logging**: Support for different log levels (ERROR, INFO, DEBUG, WARNING)
- **Memory Alignment**: Optimized for performance with proper memory alignment
- **Flexible Output**: Print individual pages or all logs at once
- **Zero Dependencies**: Pure C implementation with minimal external requirements
- **Thread-Safe Design**: Suitable for multi-threaded environments

## Quick Start

### Installation

#### Option 1: ESP-IDF Component Manager

Add to your project's `idf_component.yml`:

```yaml
dependencies:
  logflow:
    git: https://github.com/repvi/LogFlow.git
    version: "^1.0.0"
```

#### Option 2: Git Submodule

```bash
git submodule add https://github.com/repvi/LogFlow.git components/logger
```

#### Option 3: Direct Download

Download and extract the source code into your project's components directory.

### Basic Usage

```c
#include "logger.h"

int main() {
    // Create logger with 5 pages of 512 bytes each
    LoggerHandler logger = logger_create(5, 512);
    if (!logger) {
        printf("Failed to create logger\n");
        return -1;
    }
    
    // Log some data
    logger_save_to_page_line(logger, "System started", -1, 0);
    logger_save_to_page_line(logger, "Loading configuration", -1, 1);
    logger_save_to_page_line(logger, "Ready to accept connections", -1, 2);
    
    // Set page types for filtering
    logger_set_page_type(logger, 0, PAGE_TYPE_INFO);
    logger_set_page_type(logger, 1, PAGE_TYPE_DEBUG);
    logger_set_page_type(logger, 2, PAGE_TYPE_INFO);
    
    // Print all logs
    logger_print_all(logger);
    
    // Print specific page
    logger_print_page(logger, 0);
    
    // Cleanup
    logger_destroy(logger);
    return 0;
}
```

### ESP-IDF Example

```c
#include "logger.h"
#include "esp_log.h"

void app_main(void) {
    LoggerHandler logger = logger_create(10, 256);
    
    // Log system events
    logger_save_to_page_line(logger, "ESP32 boot complete", -1, 0);
    logger_set_page_type(logger, 0, PAGE_TYPE_INFO);
    
    // Log sensor data
    char sensor_data[64];
    snprintf(sensor_data, sizeof(sensor_data), "Temperature: 25.3C");
    logger_save_to_page_line(logger, sensor_data, -1, 1);
    logger_set_page_type(logger, 1, PAGE_TYPE_INFO_DEBUG);
    
    // Print logs when needed
    logger_print_all(logger);
    
    // Keep logger alive for the application lifetime
    // logger_destroy(logger); // Call this before app exit
}
```

## Building

### Requirements

- **CMake**: 3.10 or higher
- **Compiler**: C11 compatible compiler (GCC, Clang, MSVC)
- **ESP-IDF**: 4.4+ (for ESP32 builds only)

### Standalone Build

```bash
# Clone the repository
git clone https://github.com/repvi/LogFlow.git
cd LogFlow

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Debug

# Run tests
./Debug/LoggerTest    # Windows
./LoggerTest          # Linux/macOS
```

### Cross-Platform Configuration

The library automatically detects the build environment:

- **ESP-IDF**: Uses `heap_caps_malloc()` for ESP32-optimized allocation
- **Standard**: Uses system `malloc()`
- **Arduino**: Compatible with Arduino framework
- **Zephyr**: Compatible with Zephyr RTOS

### Build Options

```bash
# Debug build with additional checks
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build optimized for size
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..

# Custom compiler
cmake -DCMAKE_C_COMPILER=clang ..
```

## Architecture

### Memory Layout

```
Logger Structure:
┌─────────────────┐
│   logger_t      │  ← Main structure
├─────────────────┤
│   Page 0        │  ← page_list + buffer
│   ├─ Metadata   │
│   └─ Buffer     │
├─────────────────┤
│   Page 1        │
│   ├─ Metadata   │
│   └─ Buffer     │
├─────────────────┤
│   ...           │
└─────────────────┘
```

### Page Structure

Each page contains:
- **Metadata**: Remaining space, buffer pointer, list pointers
- **Buffer**: Actual log data storage
- **Alignment**: Memory-aligned for optimal performance

## API Overview

### Core Functions

| Function | Description |
|----------|-------------|
| `logger_create()` | Create a new logger instance |
| `logger_destroy()` | Free logger resources |
| `logger_save_to_page()` | Save data to specific page |
| `logger_save_to_page_line()` | Save data with automatic newline |
| `logger_print_page()` | Print contents of specific page |
| `logger_print_all()` | Print all pages |

### Page Management

| Function | Description |
|----------|-------------|
| `logger_set_page_type()` | Set log level for page |
| `logger_get_page_buffer()` | Get direct buffer access |
| `logger_clear_page()` | Clear specific page |
| `logger_clear_all()` | Clear all pages |

For detailed API documentation, see [API.md](API.md).

## Configuration

### Memory Alignment

The logger uses platform-specific alignment:

```c
#define BUFFER_ALIGNMENT 8  // Default alignment
#define ALIGNOF(type) _Alignof(type)  // C11 alignment
```

### Platform-Specific Options

```c
// ESP32 specific heap allocation
#ifdef __XTENSA__
    void *mallocv(int size) {
        return heap_caps_malloc(size, MALLOC_CAP_8BIT);
    }
#endif
```

## Error Handling

The library provides robust error handling:

- **NULL checks**: All functions validate input parameters
- **Memory allocation**: Graceful handling of allocation failures
- **Bounds checking**: Page index validation
- **Return codes**: Functions return -1 on error, byte count on success

## Performance Considerations

### Memory Usage

- **Base overhead**: ~32 bytes per logger instance
- **Page overhead**: ~24 bytes per page
- **Alignment padding**: Minimal with proper page size selection

### Optimal Page Sizes

- **Small logs**: 64-128 bytes per page
- **Medium logs**: 256-512 bytes per page
- **Large logs**: 1024+ bytes per page

### Best Practices

1. **Choose appropriate page count**: Balance memory usage vs. log capacity
2. **Align page sizes**: Use powers of 2 for optimal alignment
3. **Minimize allocations**: Create logger once, reuse throughout application
4. **Clear when full**: Use `logger_clear_page()` to recycle pages

## Testing

### Running Tests

```bash
# Build and run test suite
cd build
make LoggerTest
./LoggerTest
```

### Test Coverage

The test suite covers:
- Memory allocation and deallocation
- Page management operations
- Error conditions and edge cases
- Cross-platform compatibility
- Memory alignment verification

## Contributing

We welcome contributions! Please follow these guidelines:

### Development Setup

```bash
git clone https://github.com/repvi/LogFlow.git
cd LogFlow
git checkout -b feature/your-feature-name
```

### Coding Standards

- **C11 Standard**: Use modern C features appropriately
- **Memory Safety**: Always check allocation returns
- **Documentation**: Document all public functions
- **Testing**: Add tests for new features

### Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add/update tests
5. Update documentation
6. Submit pull request

## Troubleshooting

### Common Issues

**Build Errors:**
```bash
# Missing CMake
sudo apt-get install cmake  # Ubuntu/Debian
brew install cmake          # macOS

# Compiler issues
export CC=gcc
export CXX=g++
```

**Memory Issues:**
- Ensure sufficient heap space for page allocation
- Check page size alignment (prefer powers of 2)
- Verify `logger_destroy()` is called to prevent leaks

**ESP-IDF Integration:**
- Verify ESP-IDF version compatibility (4.4+)
- Check component.yml syntax
- Ensure proper include paths

### Debug Mode

Enable debug output by compiling with:

```bash
cmake -DDEBUG_LOGGER=ON ..
```

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Support

- **Issues**: [GitHub Issues](https://github.com/repvi/LogFlow/issues)
- **Repository**: [GitHub](https://github.com/repvi/LogFlow)
- **Documentation**: [API Reference](api.md)

## Changelog

### v1.0.0
- Initial release
- Cross-platform support
- ESP-IDF component integration
- Memory-aligned page management
- Comprehensive test suite

---

**Note**: This library is optimized for embedded systems where memory efficiency is crucial. For high-throughput applications, consider the memory alignment and page size settings for your specific use case.
