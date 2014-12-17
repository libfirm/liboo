/*
 * This file is part of liboo.
 */

/**
 * @file	rta.h
 * @brief	Devirtualization of dynamically bound calls through Rapid Type Analysis
 * @author	Steffen Knoth
 * @date	2014
 */


#ifndef OO_RTA_H
#define OO_RTA_H


#include <libfirm/firm.h>

/** sets important callback functions needed to detect calls (e.g. class intialization) hidden behind frontend-specific nodes
 * @note It's very important for the frontend to implement these callbacks correctly, if anything is missing RTA's assumptions may not hold and it can lead to defective programs!
 * @note This mechanism is meant for functions similar to e.g. _Jv_InitClass that hide calls to analyzable methods in native code.
 * @note It doesn't make much sense to return entities of native functions or methods. Return methods that can and should be analyzed even if they are called indirectly by other native methods that are called by the method/function in question.
 * @note The callbacks are only called on statically bound call nodes that call a function that has no firm graph.
 * @param detect_call give function that returns the entity (ir_entity*) of the method called by the call node or NULL if no method is called //TODO support for more than one
 */
void rta_set_detection_callbacks(ir_entity *(*detect_call)(ir_node *call));


/** runs Rapid Type Analysis and then tries to devirtualize dynamically bound calls and to discard unneeded classes and methods
 * @note RTA requires the analyzed code to be typesafe, meaning objects on which dynamically bound calls are invoked have to be of the static type of the reference or a subclass of it, otherwise the results can be incorrect and can lead to defective programs!
 * @note RTA must know of _all_ definitely executed code parts (main, class initializers, global contructors or all nonprivate functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!
 * @note RTA also won't work with programs that dynamically load classes or code at runtime! It can lead to defective programs!
 * @note Give classes that are instantiated in native methods of a nonexternal standard library or runtime as initial_live_classes, give methods called in these native methods as additional entry points. If something is missing, RTA could produce defective programs! This also means that native methods in the program that do arbitrary things are not supported as long as there is no way to tell RTA what classes are instantiated and what methods are called in all those native methods.
 * @note C++ is currently not supported (C++ constructor semantics, function pointers, ...).
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 */
void rta_optimization(ir_entity** entry_points, ir_type **initial_live_classes);


#endif
