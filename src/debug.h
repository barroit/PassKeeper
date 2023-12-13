#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#define DEBUG_ONLY(fn) (fn)

struct app_option;

void print_appopt(const struct app_option *appopt);

#else /* DEBUG */

#define DEBUG_ONLY(fn) ((void)0)

#endif /* DEBUG */

#endif /* DEBUG_H */