package firm.nodes;

import com.sun.jna.Pointer;

import firm.Type;
import firm.bindings.binding_oo_nodes;

public class InstanceOf extends Node {

	public InstanceOf(Pointer ptr) {
		super(ptr);
	}
	
	public Node getMem() {
		return OONodeWrapperConstruction.createWrapper(
				binding_oo_nodes.get_InstanceOf_mem(this.ptr));
	}
	
	public void setMem(Node mem) {
		binding_oo_nodes.set_InstanceOf_mem(this.ptr, mem.ptr);
	}
	
	public Node getObjPtr() {
		return OONodeWrapperConstruction.createWrapper(
				binding_oo_nodes.get_InstanceOf_objptr(this.ptr));
	}
	
	public void setObjPtr(Node objPtr) {
		binding_oo_nodes.set_InstanceOf_objptr(this.ptr, objPtr.ptr);
	}
	
	public Type getType() {
		return Type.createWrapper(
				binding_oo_nodes.get_InstanceOf_type(this.ptr));
	}
	
	public void setType(Type type) {
		binding_oo_nodes.set_InstanceOf_type(this.ptr, type.ptr);
	}
	
	@Override
	public void accept(NodeVisitor visitor) {
		assert visitor instanceof OONodeVisitor;
		((OONodeVisitor)visitor).visit(this);
	}
	
	public static final int pnM   = 0;
	public static final int pnRes = 1;
	public static final int pnMax = 1;
}
