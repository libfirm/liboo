#include "liboo/devirt_local.h"

#include <assert.h> 

#include <liboo/oo.h>
#include <liboo/nodes.h>

#include "adt/cpset.h"
#include "adt/pdeq.h"
#include "adt/hashptr.h"

typedef struct global_env {
	pdeq *workqueue;
	cpset_t *done_methods;
} global_env;

static ir_entity *get_class_member_by_name(ir_type *cls, ident *ident) // function which was removed from newer libfirm versions
{
	for (size_t i = 0, n = get_class_n_members(cls); i < n; ++i) {
		ir_entity *entity = get_class_member(cls, i);
		if (get_entity_ident(entity) == ident)
			return entity;
	}
	return NULL;
}

static int ptr_equals(const void *pt1, const void *pt2) // missing default pointer compare function
{
	return pt1 == pt2;
}

/*static inline cpset_t *new_cpset(cpset_hash_function hash_function, cpset_cmp_function cmp_function) // missing new function for cpset
{
	cpset_t *cpset = (cpset_t*)malloc(sizeof(cpset_t));
	cpset_init(cpset, hash_function, cmp_function);
	return cpset;
}*/

static ir_entity *default_detect_call(ir_node *call) { (void)call; return NULL; } 

static ir_entity *(*detect_call)(ir_node *call) = &default_detect_call;

void devirt_set_detection_callbacks(ir_entity *(*detect_call_callback)(ir_node *call)) {
	assert(detect_call_callback);
	detect_call = detect_call_callback;
}

static void add_to_workqueue(ir_entity *entity, global_env *env)
{
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	if (cpset_find(env->done_methods, entity) == NULL) { // only enqueue if not already done
		pdeq_putr(env->workqueue, entity);
	}
}

static void walk_graph_and_devirtualize(ir_node *node, void *env) {
	global_env *glob_env = (global_env*) env;
	switch(get_irn_opcode(node)) {
	case iro_Address: {  
		ir_node *address = node;
		ir_entity *entity = get_Address_entity(address);
		if(is_method_entity(entity)) 
			add_to_workqueue(entity, glob_env);
		break;
	}
	case iro_Call: {
		ir_node *call = node;
		ir_node *callee = get_irn_n(call, 1);
		if(is_Address(callee)) {
			ir_entity *entity = get_Address_entity(callee);
			if(is_method_entity(entity)) 
				add_to_workqueue(entity, glob_env);
			
			//TODO: Do stuff here
		} else if(is_Proj(callee) && !oo_get_call_is_statically_bound(call)) {
			ir_node *pred = get_Proj_pred(callee);
			if(is_MethodSel(pred)) {
				ir_node *methodsel = pred;
				ir_entity *entity = get_MethodSel_entity(methodsel);
				ir_node *proj = get_irn_n(methodsel, 1);
				if(is_Proj(proj)) {
					ir_node *vptrIsSet = get_Proj_pred(proj);
					if(is_VptrIsSet(vptrIsSet)) {
						ir_type *type = get_VptrIsSet_type(vptrIsSet);
						ir_entity *target = get_class_member_by_name(type, get_entity_ident(entity));
						printf("DEBUG: %s \n", get_entity_name(entity));
						if(target != NULL) {
							ir_graph *graph = get_irn_irg(methodsel);
							ir_node *address = new_r_Address(graph, target);
							ir_node *mem = get_irn_n(methodsel, 0);
							ir_node *input[] = { mem, address };
							turn_into_tuple(methodsel, 2, input);
						}
					}
				}
			} else {
				//Should not really happen
			}
		} else {
			//should not happen either
		}
		break;
	}
	default:
		/*if(is_VptrIsSet(node)) {
			ir_type *klass = get_VptrIsSet_type(node);
			//object creation. via _ptr pointer available
		}*/
		break;
	}

}

void devirtualize_calls_to_local_objects(ir_entity **entry_points) {
	cpset_t done_methods;
	cpset_init(&done_methods, hash_ptr, ptr_equals);

	pdeq *workqueue = new_pdeq();
	global_env env = {
		.workqueue = workqueue,
		.done_methods = &done_methods,
	};

	ir_entity *entity;
	for(size_t i = 0; (entity = entry_points[i]) != NULL; i++) {
		assert(is_method_entity(entity));
		ir_graph *graph = get_entity_irg(entity);
		if(graph) {
			pdeq_putr(workqueue, entity);
		}
	}
	while(!pdeq_empty(workqueue)) {
		ir_entity *entity = pdeq_getl(workqueue);
		assert(entity && is_method_entity(entity));

		if(cpset_find(&done_methods, entity) != NULL) continue;
		cpset_insert(&done_methods, entity);

		ir_graph *graph = get_entity_irg(entity);
		if(graph != NULL) {
			irg_walk_graph(graph, NULL, walk_graph_and_devirtualize, &env);
		}
	}
}

