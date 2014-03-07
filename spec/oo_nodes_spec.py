from spec_util import abstract, op, Attribute

name = "oo"
external = "liboo"
java_binding = "firm.bindings.binding_nodes"
java_package = "firm.oo.nodes"

@op
class InstanceOf:
	"""Check if an object is an instance of a specified type (or a subtype).
	Passing a null pointer results in undefined behaviour."""
	ins = [
		("mem",   "memory dependency"),
		("ptr",   "pointer to object"),
	]
	outs = [
		("M",     "memory result"),
		("res",   "result of instanceof check"), # mode_b
	]
	attrs = [
		Attribute("type", type="ir_type*", comment="classtype to check for"),
	]
	flags       = [ "uses_memory" ]
	pinned      = "no"
	attr_struct = "op_InstanceOf_attr_t"

@op
class Arraylength:
	"""Return the lenght of a (dynamic) array"""
	ins = [
		("mem", "memory dependency"),
		("ptr", "pointer to array"),
	]
	outs = [
		("M",   "memory result"),
		("res", "length of the array"),
	]
	flags   = [ "uses_memory" ]
	pinned  = "no"

@op
class MethodSel:
	"""Performs a vtable lookup for a method (or rtti info)."""
	ins = [
		("mem", "memory dependency"),
		("ptr", "pointer to an object with a vtable"),
	]
	outs = [
		("M",   "memory result"),
		("res", "address of method"),
	]
	attrs = [
		Attribute("entity", type="ir_entity*",
		          comment="method entity which should be selected"),
	]
	attr_struct = "op_MethodSel_attr_t"
	flags       = [ "uses_memory" ]
	pinned      = "no"
