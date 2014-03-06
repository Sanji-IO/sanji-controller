#ifndef _DEBUG_H_
#define _DEBUG_H_

/* debug macro */
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) do { fprintf(stderr, "[sanji] %s: %s: %d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#endif /* _DEBUG_H_ */
