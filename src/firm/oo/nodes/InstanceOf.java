/* Warning: Automatically generated file */
package firm.oo.nodes;

import com.sun.jna.Pointer;
import firm.bindings.binding_irop;
import firm.bindings.binding_ircons;
import firm.nodes.Node;
import firm.nodes.NodeVisitor;
import firm.nodes.NodeWrapperFactory;
import firm.Construction;

public class InstanceOf extends Node {
	static class Factory implements NodeWrapperFactory {
		@Override
		public Node createWrapper(Pointer ptr) {
			return new InstanceOf(ptr);
		}
	}

	static void init() {
		Pointer op = firm.bindings.binding_nodes.get_op_InstanceOf();
		Node.registerFactory(binding_irop.get_op_code(op), new Factory());
	}
	public static Node create(Node block, Node mem, Node ptr, firm.Type type) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_InstanceOf(block.ptr, mem.ptr, ptr.ptr, type.ptr));
	}

	public static Node create(Construction cons, Node mem, Node ptr, firm.Type type) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_InstanceOf(binding_ircons.get_r_cur_block(cons.getGraph().ptr), mem.ptr, ptr.ptr, type.ptr));
	}

	public InstanceOf(Pointer ptr) {
		super(ptr);
	}

	public Node getMem() {
		return createWrapper(firm.bindings.binding_nodes.get_InstanceOf_mem(ptr));
	}

	public void setMem(Node mem) {
		firm.bindings.binding_nodes.set_InstanceOf_mem(this.ptr, mem.ptr);
	}

	public Node getPtr() {
		return createWrapper(firm.bindings.binding_nodes.get_InstanceOf_ptr(ptr));
	}

	public void setPtr(Node ptr) {
		firm.bindings.binding_nodes.set_InstanceOf_ptr(this.ptr, ptr.ptr);
	}

	public firm.Type getType() {
		Pointer _res = firm.bindings.binding_nodes.get_InstanceOf_type(ptr);
		return firm.Type.createWrapper(_res);
	}

	public void setType(firm.Type _val) {
		firm.bindings.binding_nodes.set_InstanceOf_type(this.ptr, _val.ptr);
	}

	@Override
	public void accept(NodeVisitor visitor) {
		visitor.visitUnknown(this);
	}

	/** memory result */
	public static final int pnM = 0;

	/** result of instanceof check */
	public static final int pnRes = 1;

	public static final int pnMax = 2;
}
