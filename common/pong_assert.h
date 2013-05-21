/* ************************************************************************** */
/*
 * This will define 'assert macro', which may be left active in real releases
 * too. At least in current implementation also some problems which may not be
 * bugs in code are reported using this. Main advantage is to have file name and
 * line where problem was detected in debug prints without trouble. When the
 * startup and shutdown IFs are implemented, this should call the global shutdown
 * in order to eliminate possible 'phantom threads/processes'. Also a severity
 * level which tells whether only one client is eliminated, or if server and all
 * clients are killed should be added.
 *
 *    Revision history:
 *
 *    -0.0.2  12.06.2008/Maz  a bugfix in macro
 *    -0.0.1  11.06.2008/Maz
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef PONG_ASSERT_H
#define PONG_ASSERT_H
#include <stdio.h>

#define pong_assert(foo,bar) \
{ if(foo) \
{ \
    printf("%s at %s:%d",bar,__FILE__,__LINE__); \
    exit(-1);  \
}       \
}
#endif // PONG_ASSERT_H
