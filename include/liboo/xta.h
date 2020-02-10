/*
 * This file is part of liboo and contains the implementation
 * of XTA to devirtualize dynamic calls.
 */

/**
 * @file	xta.h
 * @brief	Devirtualization of dynamically bound calls through Hyprid Type Analysis
 * @author	Daniel Biester
 * @date	2018
 */




#ifndef OO_XTA_H
#define OO_XTA_H


#include <stdbool.h>
#include <libfirm/firm.h>
#include "../../src-cpp/adt/cpmap.h"
#include "../../src-cpp/adt/arrayset.h"

/*
 * sets important callback function for detecting constructors.
 * @note Is is very important for XTA to have the ability to detect constructors, otherwise results can be incorrect.
 * @param is_constr The callback function returning a boolean, TRUE if the parameter method of type ir_emtity is a constructor, FALSE otherwise
 */
void xta_set_is_constructor_callback(bool (*is_constr)(ir_entity *method));

/*
 * This method gives frontends the ability to pass a set of ir_entities (methods) to XTA which are library methgds and don't leak arguments to other library methods.
 * @param methods the set of non-leaking methods
 */
void xta_set_non_leaking_ext_methods(arrayset_t *methods);

/**
 * Calls all methods on passed subtypes of the method's specified parameter.
 * @param the method in which the calls take place
 * @param the parameter number of the parameter;
 */
void call_methods_on_param_type(ir_entity *method, int param_n);

/**
 * Adds a type to a method, which is live in the caller after the call.
 * Useful for methods which cannot be analyzed.
 * @param method the method which is leaking the type
 * @param leaking_type the type
 */
void add_leaking_type_to_method(ir_entity *method, ir_type *leaking_type);

/**
 * Given entity will leak subtypes of return type, which are globally live (there exists one instantiation in the program)
 * @param the method which leaks all subtypes of return type
 */
void leak_all_live_ret_subtypes(ir_entity *method);

/** sets important callback function needed to detect calls (e.g. class intialization) hidden behind frontend-specific nodes
 * @note It's very important for the frontend to implement these callbacks correctly, if anything is missing XTA's assumptions may not hold and it can lead to defective programs!
 * @note This mechanism is meant for functions similar to e.g. _Jv_InitClass that hide calls to analyzable methods in native code.
 * @note It doesn't make much sense to return entities of native functions or methods. Return methods that can and should be analyzed even if they are called indirectly by other native methods that are called by the method/function in question.
 * @note The callbacks are only called on statically bound call nodes that call a function that has no firm graph.
 * @param detect_call give function that returns the entity (ir_entity*) of the method called by the call node or NULL if no method is called
 */
void xta_set_detection_callbacks(ir_entity * (*detect_call)(ir_node *call));


/** runs Hybrid Type Analysis (XTA) and then tries to devirtualize dynamically bound calls and to discard unneeded classes and methods
 * @note XTA requires object creations to be marked with VptrIsSet nodes.
 * @note XTA requires the analyzed code to be typesafe, meaning objects on which dynamically bound calls are invoked have to be of the static type of the reference or a subclass of it, otherwise the results can be incorrect and can lead to defective programs!
 * @note XTA won't work with programs that dynamically load classes at runtime or use generic object creation (like Java Class.newInstance)! It can lead to defective programs!
 * @note C++ is currently not supported (C++ constructor semantics, function pointers, ...).
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 */
void xta_optimization(ir_entity **entry_points, ir_type **initial_live_classes);


#endif
