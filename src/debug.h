#ifndef _DEBUG_H
#define _DEBUG_H

/* debug macro */
#if defined DEBUG
#define DEBUG_PRINT(fmt, ...) do { fprintf(stderr, "[SANJI] %s: %s: %d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)
#elif defined VERBOSE
#define DEBUG_PRINT(fmt, ...) do { fprintf(stderr, "[SANJI] " fmt "\n", ##__VA_ARGS__); } while (0)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#endif /* _DEBUG_H */
