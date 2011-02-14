package firm.nodes;

public interface OONodeVisitor extends NodeVisitor {
	
	void visit(InstanceOf node);
	void visit(Arraylength node);

}
