from spec_util import abstract, op

name = "oo"
external = "liboo"

@op
class InstanceOf:
	"""Check if an object is an instance of a specified type (or a subtype)"""
	ins = [
		("mem",   "memory dependency"),
		("ptr",   "pointer to object"),
	]
	outs = [
		("M",     "memory result"),
		("res",   "result of instanceof check"), # mode_b
	]
	attrs = [
		dict(
			name    = "type",
			type    = "ir_type*",
			comment = "classtype to check for",
		)
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
