/* Warning: Automatically generated file */
package firm.oo.nodes;

import com.sun.jna.Pointer;
import firm.bindings.binding_ircons;
import firm.nodes.Node;
import firm.nodes.NodeVisitor;
import firm.nodes.NodeWrapperFactory;
import firm.Construction;

public class Arraylength extends Node {
	static class Factory implements NodeWrapperFactory {
		@Override
		public Node createWrapper(Pointer ptr) {
			return new Arraylength(ptr);
		}
	}

	static void init() {
		Pointer op = firm.bindings.binding_nodes.get_op_Arraylength();
		Node.registerFactory(firm.bindings.binding_irop.get_op_code(op), new Factory());
	}
	public static Node create(Node block, Node mem, Node _ptr) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_Arraylength(block.ptr, mem.ptr, _ptr.ptr));
	}

	public static Node create(Construction cons, Node mem, Node _ptr) {
		return Node.createWrapper(firm.bindings.binding_nodes.new_r_Arraylength(binding_ircons.get_r_cur_block(cons.getGraph().ptr), mem.ptr, _ptr.ptr));
	}

	public Arraylength(Pointer ptr) {
		super(ptr);
	}

	public Node getMem() {
		return createWrapper(firm.bindings.binding_nodes.get_Arraylength_mem(ptr));
	}

	public void setMem(Node mem) {
		firm.bindings.binding_nodes.set_Arraylength_mem(this.ptr, mem.ptr);
	}

	public Node getPtr() {
		return createWrapper(firm.bindings.binding_nodes.get_Arraylength_ptr(ptr));
	}

	public void setPtr(Node _ptr) {
		firm.bindings.binding_nodes.set_Arraylength_ptr(this.ptr, _ptr.ptr);
	}

	@Override
	public void accept(NodeVisitor visitor) {
		visitor.visitUnknown(this);
	}

	/** memory result */
	public static final int pnM = 0;

	/** length of the array */
	public static final int pnRes = 1;

	public static final int pnMax = 2;
}
