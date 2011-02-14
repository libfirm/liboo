package firm.nodes;

import com.sun.jna.Pointer;

import firm.Construction;
import firm.Graph;
import firm.bindings.binding_ircons;
import firm.bindings.binding_oo_nodes;

public class OOConstruction extends Construction {
	
	public OOConstruction(Graph graph) {
		super(graph);
	}

	public Node newInstanceOf(Node mem, Node objptr, firm.Type type) {
		Pointer result_ptr = binding_oo_nodes.new_r_InstanceOf(binding_ircons.get_r_cur_block(graph.ptr), mem.ptr, objptr.ptr, type.ptr);
		return OONodeWrapperConstruction.createWrapper(result_ptr);
	}
	
	public Node newArraylength(Node mem, Node arrayref) {
		Pointer result_ptr = binding_oo_nodes.new_r_Arraylength(binding_ircons.get_r_cur_block(graph.ptr), mem.ptr, arrayref.ptr);
		return OONodeWrapperConstruction.createWrapper(result_ptr);
	}
}
