/*
 * This file is part of liboo.
 */

/**
 * @file	rta.h
 * @brief	Devirtualization of dynamically linked calls through Rapid Type Analysis
 * @author	Steffen Knoth
 * @date	2014
 */


#ifndef OO_RTA_H
#define OO_RTA_H


#include <libfirm/firm.h>

/** sets important callback functions needed to detect object creation or calls (e.g. class intialization) hidden behind frontend-specific nodes
 * @note It's very important for the frontend to implement these callbacks correctly, if anything is missing RTA's assumptions may not hold and it can lead to defective programs!
 * @note Don't give object creation from a generic calloc-like call if VPTR is set explicitly in the graph in conjunction with it. RTA can detect the object creation when VPTR is set. This mechanism is meant for magical functions similar to e.g. _Jv_AllocObjectNoFinalizer and _Jv_InitClass that get an RTTI object and do set VPTR and everything in external code. Also, it doesn't make sense to return entities of native functions or methods.
 * @note The callbacks are only called on statically linked calls that call a function that has no firm graph.
 * @param detect_creation give function that returns the type object (ir_type*) of the class of the object created by the call node or NULL if no object is created //TODO support for more than one
 * @param detect_call give function that returns the entity (ir_entity*) of the method called by the call node or NULL if no method is called //TODO support for more than one
 */
void rta_set_detection_callbacks(ir_type *(*detect_creation)(ir_node *call), ir_entity *(*detect_call)(ir_node *call));


/** runs Rapid Type Analysis and then tries to devirtualize dynamically linked calls and to discard unneeded classes and methods
 * @note RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all nonprivate functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!!
 * @note RTA also won't work with programs that dynamically load classes at runtime! It can lead to defective programs!!
 * @note RTA might also produce incorrect programs if there is some Java Reflections shenanigans in the code (internal or external), especially using java.lang.Class.newInstance()
 * @note Give classes that are instantiated in native methods of a nonexternal standard library or runtime as initial_live_classes, give methods called in these native methods as additional entry points. If something is missing, RTA could produce incorrect programs! This also means that native methods in the program that do arbitrary things are not supported.
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 */
void rta_optimization(ir_entity** entry_points, ir_type **initial_live_classes);


#endif
