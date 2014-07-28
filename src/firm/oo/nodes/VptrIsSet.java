/* Warning: Automatically generated file */
package firm.oo.nodes;

import com.sun.jna.Pointer;
import firm.bindings.binding_ircons;
import firm.nodes.Node;
import firm.nodes.NodeVisitor;
import firm.nodes.NodeWrapperFactory;
import firm.Construction;

public class VptrIsSet extends Node {
	static class Factory implements NodeWrapperFactory {
		@Override
		public Node createWrapper(Pointer ptr) {
			return new VptrIsSet(ptr);
		}
	}

	static void init() {
		Pointer op = firm.bindings.binding_nodes.get_op_VptrIsSet();
		Node.registerFactory(firm.bindings.binding_irop.get_op_code(op), new Factory());
	}
	public static Node create(Node block, Node mem, Node _ptr, firm.Type type) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_VptrIsSet(block.ptr, mem.ptr, _ptr.ptr, type.ptr));
	}

	public static Node create(Construction cons, Node mem, Node _ptr, firm.Type type) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_VptrIsSet(binding_ircons.get_r_cur_block(cons.getGraph().ptr), mem.ptr, _ptr.ptr, type.ptr));
	}

	public VptrIsSet(Pointer ptr) {
		super(ptr);
	}

	public Node getMem() {
		return createWrapper(firm.bindings.binding_nodes.get_VptrIsSet_mem(ptr));
	}

	public void setMem(Node mem) {
		firm.bindings.binding_nodes.set_VptrIsSet_mem(this.ptr, mem.ptr);
	}

	public Node getPtr() {
		return createWrapper(firm.bindings.binding_nodes.get_VptrIsSet_ptr(ptr));
	}

	public void setPtr(Node _ptr) {
		firm.bindings.binding_nodes.set_VptrIsSet_ptr(this.ptr, _ptr.ptr);
	}

	public firm.Type getType() {
		Pointer _res = firm.bindings.binding_nodes.get_VptrIsSet_type(ptr);
		return firm.Type.createWrapper(_res);
	}

	public void setType(firm.Type _val) {
		firm.bindings.binding_nodes.set_VptrIsSet_type(this.ptr, _val.ptr);
	}

	@Override
	public void accept(NodeVisitor visitor) {
		visitor.visitUnknown(this);
	}

	/** memory result */
	public static final int pnM = 0;

	/** pointer to object */
	public static final int pnRes = 1;

	public static final int pnMax = 2;
}
