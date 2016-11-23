/* 
 * File:   dbg.h
 * Author: Bela Bursan
 *
 * Created on October 12, 2016, 10:54 AM
 */

#ifndef DBG_H
#define DBG_H

#include <stdarg.h>
#include <syslog.h>


#ifdef __cplusplus
extern "C" {
#endif

void dbg (const char *txt, ...);
void error (const char *txt, ...);
void sysl(const int prio, const char *txt, ...);

#ifdef __cplusplus
}
#endif

#endif /* DBG_H */
