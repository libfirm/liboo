

#ifndef OO_RTA_H
#define OO_RTA_H


#include <libfirm/firm.h>

#include "../src-cpp/adt/cpset.h"
#include "../src-cpp/adt/cpmap.h"


/** run Rapid Type Analysis
 * It runs over a reduced callgraph and detects which classes and methods are actually used and computes reduced sets of potentially called targets for each dynamically linked call.
 * @param entry_points all (public) entry points to program code (as ir_entity*), RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all public functions if it's a library), /!\ It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!!
 * @param used_classes give pointer to empty uninitialized set for receiving results, This is a set where all used classes are put (as ir_type*).
 * @param used_methods give pointer to empty uninitialized set for receiving results, This is a set where all used methods are put (as ir_entity*).
 * @param dyncall_targets give pointer to empty uninitialized map for receiving results, This is a map where call entities are mapped to their actually used potential call targets (ir_entity* -> {ir_entity*}). It's used to optimize dynamically linked calls if possible. (see also function rta_optimize_dyncalls)
 */
void rta_run(cpset_t *entry_points, cpset_t *used_classes, cpset_t *used_methods, cpmap_t *dyncall_targets);

/** frees memory allocated for the results returned by function run_rta
 * /!\ does not free the memory of the sets and maps themself, just their content allocated during RTA
 * @param used_classes as returned by run_rta
 * @param used_methods as returned by run_rta
 * @param dyncall_targets as returned by run_rta
 */
void rta_dispose_results(cpset_t *used_classes, cpset_t *used_methods, cpmap_t *dyncall_targets);


#endif
