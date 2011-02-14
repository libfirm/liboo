package firm.nodes;

import com.sun.jna.Pointer;

import firm.bindings.binding_oo_nodes;

class OONodeWrapperConstruction {
	public static Node createWrapper(Pointer ptr) {
		if (binding_oo_nodes.is_InstanceOf(ptr))
			return new InstanceOf(ptr);
		
		if (binding_oo_nodes.is_Arraylength(ptr))
			return new Arraylength(ptr);
		
		return NodeWrapperConstruction.createWrapper(ptr);
	}
}
