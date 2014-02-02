

#ifndef OO_RTA_H
#define OO_RTA_H


#include <libfirm/firm.h>


/** runs Rapid Type Analysis and then tries to devirtualize dynamically linked calls and discards unused classes and methods
 * @note RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all public functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!! RTA also won't work with programs that dynamically load classes at run-time!
 * @param n_entry_points number of entry points in array entry_points
 * @param entry_points array of ir_entity*, give all entry points to program code
 */
void rta_optimization(size_t n_entry_points, ir_entity** entry_points);


#endif
