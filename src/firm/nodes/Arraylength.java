package firm.nodes;

import com.sun.jna.Pointer;

import firm.bindings.binding_firm_common.pn_generic;
import firm.bindings.binding_oo_nodes;

public class Arraylength extends Node {

	public Arraylength(Pointer ptr) {
		super(ptr);
	}
	
	public Node getMem() {
		return OONodeWrapperConstruction.createWrapper(
				binding_oo_nodes.get_Arraylength_mem(this.ptr));
	}
	
	public void setMem(Node mem) {
		binding_oo_nodes.set_Arraylength_mem(this.ptr, mem.ptr);
	}
	
	public Node getArrayRef() {
		return OONodeWrapperConstruction.createWrapper(
				binding_oo_nodes.get_Arraylength_arrayref(this.ptr));
	}
	
	public void setArrayRef(Node arrayRef) {
		binding_oo_nodes.set_Arraylength_arrayref(this.ptr, arrayRef.ptr);
	}
	
	@Override
	public void accept(NodeVisitor visitor) {
		assert visitor instanceof OONodeVisitor;
		((OONodeVisitor)visitor).visit(this);
	}
	
	public static final int pnM   = pn_generic.pn_Generic_M.ordinal();
	public static final int pnRes = pn_generic.pn_Generic_other.ordinal();
	public static final int pnMax = pnRes+1;
}
