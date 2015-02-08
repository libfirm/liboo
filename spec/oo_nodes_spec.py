from irops import op, Attribute, prepare_nodes, Node
from jinjautil import export

name = "oo"
external = "liboo"
java_binding = "firm.bindings.binding_nodes"
java_package = "firm.oo.nodes"

@op
class InstanceOf(Node):
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
class Arraylength(Node):
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
class MethodSel(Node):
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

@op
class VptrIsSet(Node):
	"""Mark that an objects vptr has been set. This node duplicates its pointer
	input and guarantees that all users of this node have an object of the
	specified type.

	In practice you would put such a node behind allocation calls in a java-like
	language or behind an external constructor call in a C++-like language.

	You could think of this as a special case of Confirm node, ensuring a
	certain object type."""
	ins = [
		("mem", "memory dependency"),
		("ptr", "pointer to an object"),
	]
	outs = [
		("M",   "memory result"),
		("res", "pointer to object"),
	]
	attrs = [
		Attribute("type", type="ir_type*",
		          comment="type of the object"),
	]
	attr_struct = "op_VptrIsSet_attr_t"
	flags       = [ "uses_memory" ]
	pinned      = "no"

(nodes, abstract_nodes) = prepare_nodes(globals())
export(nodes, "nodes")
export(abstract_nodes, "abstract_nodes")
export(globals(), "spec")
