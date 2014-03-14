/* Warning: Automatically generated file */
package firm.oo.nodes;

import com.sun.jna.Pointer;
import firm.bindings.binding_ircons;
import firm.nodes.Node;
import firm.nodes.NodeVisitor;
import firm.nodes.NodeWrapperFactory;
import firm.Construction;

public class MethodSel extends Node {
	static class Factory implements NodeWrapperFactory {
		@Override
		public Node createWrapper(Pointer ptr) {
			return new MethodSel(ptr);
		}
	}

	static void init() {
		Pointer op = firm.bindings.binding_nodes.get_op_MethodSel();
		Node.registerFactory(firm.bindings.binding_irop.get_op_code(op), new Factory());
	}
	public static Node create(Node block, Node mem, Node _ptr, firm.Entity entity) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_MethodSel(block.ptr, mem.ptr, _ptr.ptr, entity.ptr));
	}

	public static Node create(Construction cons, Node mem, Node _ptr, firm.Entity entity) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_MethodSel(binding_ircons.get_r_cur_block(cons.getGraph().ptr), mem.ptr, _ptr.ptr, entity.ptr));
	}

	public MethodSel(Pointer ptr) {
		super(ptr);
	}

	public Node getMem() {
		return createWrapper(firm.bindings.binding_nodes.get_MethodSel_mem(ptr));
	}

	public void setMem(Node mem) {
		firm.bindings.binding_nodes.set_MethodSel_mem(this.ptr, mem.ptr);
	}

	public Node getPtr() {
		return createWrapper(firm.bindings.binding_nodes.get_MethodSel_ptr(ptr));
	}

	public void setPtr(Node _ptr) {
		firm.bindings.binding_nodes.set_MethodSel_ptr(this.ptr, _ptr.ptr);
	}

	public firm.Entity getEntity() {
		Pointer _res = firm.bindings.binding_nodes.get_MethodSel_entity(ptr);
		return new firm.Entity(_res);
	}

	public void setEntity(firm.Entity _val) {
		firm.bindings.binding_nodes.set_MethodSel_entity(this.ptr, _val.ptr);
	}

	@Override
	public void accept(NodeVisitor visitor) {
		visitor.visitUnknown(this);
	}

	/** memory result */
	public static final int pnM = 0;

	/** address of method */
	public static final int pnRes = 1;

	public static final int pnMax = 2;
}
