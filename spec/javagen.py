#!/usr/bin/python
from jinja2 import Environment, Template, FileSystemLoader
import re

from spec_util import verify_node, is_dynamic_pinned, setdefault, isAbstract, setldefault, load_spec
import sys

java_keywords = [ "public", "private", "protected", "true", "false" ]
def format_filter_keywords(arg):
	if arg in java_keywords:
		return "_" + arg
	return arg

def format_argdecls(arglist):
	argstrings = map(lambda arg : (arg["type"] + " " + format_filter_keywords(arg["name"])), arglist)
	return ", ".join(argstrings)

def format_parameterlist(parameterlist):
	return "\n".join(parameterlist)

def format_nodearguments(node):
	def format_argument(arg):
		if arg['type'] == "Node[]":
			return arg["name"] + ".length, Node.getBufferFromNodeList(" + arg["name"] + ")"
		elif "to_wrapper" in arg:
			return arg["to_wrapper"] % arg['name']
		else:
			return "%s.ptr" % arg['name']
	arguments = map(format_argument, node.arguments)
	return format_parameterlist(arguments)

def format_nodeparameters(node):
	parameters = map(lambda arg: arg["type"] + " " + arg["name"], node.arguments)
	return format_parameterlist(parameters)

def format_args(arglist):
	argstrings = map(lambda arg : arg["name"], arglist)
	return ", ".join(argstrings)

def format_blockparameter(node):
	if not hasattr(node, "knownBlock"):
		return "Node block"
	return ""

def format_blockargument(node):
	if hasattr(node, "knownBlock"):
		if hasattr(node, "knownGraph"):
			return ""
		return "this.ptr"
	else:
		return "block.ptr"

def format_block_construction(node):
	if hasattr(env.globals['spec'], "external"):
		graph = "cons.getGraph().ptr"
	else:
		graph = "graph.ptr"

	if hasattr(node, "knownBlock"):
		if hasattr(node, "knownGraph"):
			return ""
		return graph
	else:
		return "binding_ircons.get_r_cur_block(%s)" % graph

def format_arguments(string, voidwhenempty = False):
	args = re.split('\s*\n\s*', string)
	if args[0] == '':
		args = args[1:]
	if len(args) > 0 and args[-1] == '':
		args = args[:-1]
	if len(args) == 0 and voidwhenempty:
		return "void"
	return ", ".join(args)

def format_parameters(string):
	return format_arguments(string)

def format_binding_args(arglist, need_graph = False):
	first = True
	res   = ""
	if need_graph:
		res = "this.ptr"
		first = False

	for arg in arglist:
		if not first:
			res += ", "
		first = False
		if arg["type"] == "Node[]":
			res += arg["name"] + ".length, Node.getBufferFromNodeList(" + arg["name"] + ")"
			continue
		name = format_filter_keywords(arg["name"])
		if "to_wrapper" in arg:
			res += arg["to_wrapper"] % name
		else:
			res += name + ".ptr"

	return res

def format_camel_case_helper(string, firstbig):
	result  = ""
	nextbig = firstbig
	for p in range(0,len(string)):
		c = string[p]
		if nextbig:
			c = c.upper()
		if c == '_':
			nextbig = True
			continue
		result += c
		nextbig = False
	
	if result == "":
		return string
	return result

def format_camel_case(string):
	return format_camel_case_helper(string, False)

def format_camel_case_big(string):
	return format_camel_case_helper(string, True)

env = Environment(loader=FileSystemLoader("."))
env.filters['argdecls']    = format_argdecls
env.filters['args']        = format_args
env.filters['bindingargs'] = format_binding_args
env.filters['camelCase']   = format_camel_case
env.filters['CamelCase']   = format_camel_case_big
env.filters['filterkeywords'] = format_filter_keywords

env.filters['arguments']          = format_arguments
env.filters['parameters']         = format_parameters
env.filters['nodeparameters']     = format_nodeparameters
env.filters['nodearguments']      = format_nodearguments
env.filters['blockparameter']     = format_blockparameter
env.filters['blockargument']      = format_blockargument
env.filters['block_construction'] = format_block_construction
env.globals['isAbstract']         = isAbstract
env.globals['len']                = len
env.globals['warning']            = "/* Warning: Automatically generated file */"

def get_java_type(type):
	if type == "ir_type*":
		java_type    = "firm.Type"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "firm.Type.createWrapper(%s)"
	elif type == "ir_mode*":
		java_type    = "firm.Mode"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "new firm.Mode(%s)"
	elif type == "ir_tarval*":
		java_type    = "firm.TargetValue"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "new firm.TargetValue(%s)"
	elif type == "ir_node*":
		java_type    = "firm.Node"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "firm.Node.createWrapper(%s)"
	elif type == "ident*":
		java_type    = "firm.Ident"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "new firm.Ident(%s)"
	elif type == "ir_relation":
		java_type    = "firm.Relation"
		wrap_type    = "int"
		to_wrapper   = "%s.value()"
		from_wrapper = "firm.Relation.fromValue(%s)"
	elif type == "int":
		java_type    = "int"
		wrap_type    = "int"
		to_wrapper   = "%s"
		from_wrapper = "%s"
	elif type == "unsigned":
		java_type    = "int"
		wrap_type    = "int"
		to_wrapper   = "%s"
		from_wrapper = "%s"
	elif type == "long":
		java_type    = "int"
		wrap_type    = "com.sun.jna.NativeLong"
		to_wrapper   = "new com.sun.jna.NativeLong(%s)"
		from_wrapper = "%s.intValue()"
	elif type == "cons_flags":
		java_type    = "firm.bindings.binding_ircons.ir_cons_flags"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_ircons.ir_cons_flags.getEnum(%s)"
	elif type == "ir_where_alloc":
		java_type    = "firm.bindings.binding_ircons.ir_where_alloc"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_ircons.ir_where_alloc.getEnum(%s)"
	elif type == "ir_entity*":
		java_type    = "firm.Entity"
		wrap_type    = "Pointer"
		to_wrapper   = "%s.ptr"
		from_wrapper = "new firm.Entity(%s)"
	elif type == "op_pin_state":
		java_type    = "firm.bindings.binding_ircons.op_pin_state"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_ircons.op_pin_state.getEnum(%s)"
	elif type == "ir_builtin_kind":
		java_type    = "firm.bindings.binding_ircons.ir_builtin_kind"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_ircons.ir_builtin_kind.getEnum(%s)"
	elif type == "ir_cons_flags":
		java_type    = "firm.bindings.binding_ircons.ir_cons_flags"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_ircons.ir_cons_flags.getEnum(%s)"
	elif type == "ir_volatility":
		java_type    = "firm.bindings.binding_irnode.ir_volatility"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_irnode.ir_volatility.getEnum(%s)"
	elif type == "ir_align":
		java_type    = "firm.bindings.binding_irnode.ir_align"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_irnode.ir_align.getEnum(%s)"
	elif type == "cond_kind":
		java_type    = "int"
		wrap_type    = "int"
		to_wrapper   = "%s"
		from_wrapper = "%s"
	elif type == "cond_jmp_predicate":
		java_type    = "firm.bindings.binding_irnode.cond_jmp_predicate"
		wrap_type    = "int"
		to_wrapper   = "%s.val"
		from_wrapper = "firm.bindings.binding_irnode.cond_jmp_predicate.getEnum(%s)"
	elif type in ("ir_node**", "ident**", "ir_asm_constraint*", "ir_switch_table*"):
		# cheat...
		java_type    = "Pointer"
		wrap_type    = "Pointer"
		to_wrapper   = "%s"
		from_wrapper = "%s"
	else:
		print "UNKNOWN TYPE %s" % type
		java_type    = "BAD"
		wrap_type    = "BAD"
		to_wrapper   = "BAD"
		from_wrapper = "BAD"
	return (java_type,wrap_type,to_wrapper,from_wrapper)

def prepare_attr(attr):
	type = attr["type"]
	(java_type,wrap_type,to_wrapper,from_wrapper) = get_java_type(type)
	attr["java_type"] = java_type
	attr["wrap_type"] = wrap_type
	attr["to_wrapper"] = to_wrapper
	attr["from_wrapper"] = from_wrapper
	if "java_name" not in attr:
		attr["java_name"] = attr["name"]

class Node:
	classname = "Node"

def preprocess_node(node):
	if not isAbstract(node):
		setdefault(node, "java_add", "")
		if hasattr(node, "__base__"):
			setldefault(node, "parent", node.__base__)
		else:
			setldefault(node, "parent", Node)
		verify_node(node)
	else:
		setldefault(node, "parent", Node)
	setldefault(node, "attrs", [])
	setldefault(node, "constructor_args", [])
	node.classname = format_camel_case_big(node.name)

	# dynamic pin node?
	if is_dynamic_pinned(node) and not hasattr(node, "pinned_init"):
		node.constructor_args.append(dict(
			name = "pin_state",
			type = "op_pin_state"
		))

	# transform outs into name, comment tuples if not in this format already
	if hasattr(node, "outs"):
		for i in range(0,len(node.outs)):
			out = node.outs[i]
			if not isinstance(out, tuple):
				out = (out, "")
			node.outs[i] = out

	# construct node arguments
	if not isAbstract(node):
		arguments = [ ]
		for input in node.ins:
			arguments.append(dict(
				type    = "Node",
				name    = input[0],
				comment = input[1]
			))
		if node.arity == "variable" or node.arity == "dynamic":
			arguments.append(dict(
				name = "ins",
				type = "Node[]"
			))
		if not hasattr(node, "mode"):
			arguments.append(dict(
				name = "mode",
				type = "firm.Mode"
			))
		for attr in node.attrs:
			prepare_attr(attr)
			if "init" in attr:
				continue
			
			arguments.append(dict(
				name = attr["java_name"],
				type = attr["java_type"],
				to_wrapper = attr["to_wrapper"],
				comment = attr["comment"]
			))
		for arg in node.constructor_args:
			old_type = arg["type"]
			(java_type,wrap_type,to_wrapper,from_wrapper) = get_java_type(old_type)

			arguments.append(dict(
				name = arg["name"],
				type = java_type,
				to_wrapper = to_wrapper
			))

		for arg in arguments:
			arg['name'] = format_filter_keywords(arg['name'])
			
		node.arguments = arguments

def main(argv):
	if len(argv) < 3:
		print "usage: %s specfile templatefile [-nodes]" % argv[0]
		sys.exit(1)

	specfile     = argv[1]
	templatefile = argv[2]
	nodesmode    = "-nodes" in argv

	spec  = load_spec(specfile)
	nodes = spec.nodes
	env.globals['nodes']   = nodes
	env.globals['spec']    = spec
	env.globals['binding'] = spec.java_binding
	env.globals['package'] = spec.java_package

	template = env.get_template(templatefile)

	for node in nodes:
		preprocess_node(node)

	if nodesmode:
		for node in nodes:
			filename = "%s.java" % node.classname
			print "Create: %s" % filename
			file = open(filename, "w");
			
			file.write(template.render(node = node).encode("utf-8") + "\n")
			file.close()
	else:
		sys.stdout.write(template.render().encode("utf-8") + "\n")

if __name__ == "__main__":
	main(sys.argv)
