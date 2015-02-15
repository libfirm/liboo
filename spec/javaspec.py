from oo_nodes_spec import nodes, abstract_nodes
from javabits import preprocess_node
from jinjautil import export

java_binding = "firm.bindings.binding_nodes"
java_package = "firm.oo.nodes"
export(java_package, "java_package")
export(java_binding, "java_binding")

for node in nodes+abstract_nodes:
	preprocess_node(node)
