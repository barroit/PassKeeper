#ifndef OS_H
#define OS_H

#ifdef __linux

#define PK_USE_ARC4RANDOM		/* has libdbs */

#define PK_USE_FHS			/* use filesystem hierarchy standard */

#endif /* __linux */

#endif /* OS_H */