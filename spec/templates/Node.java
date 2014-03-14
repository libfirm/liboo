{{warning}}
package {{package}};

import com.sun.jna.Pointer;
{%- if spec.external %}
import firm.bindings.binding_ircons;
import firm.nodes.Node;
import firm.nodes.NodeVisitor;
import firm.nodes.NodeWrapperFactory;
import firm.Construction;
{%- endif %}

public {% if isAbstract(node) %}abstract {%endif-%} class {{node.classname}} extends {{node.parent.classname}} {

	{%- if not isAbstract(node) %}
	static class Factory implements NodeWrapperFactory {
		@Override
		public Node createWrapper(Pointer ptr) {
			return new {{node.classname}}(ptr);
		}
	}

	static void init() {
		Pointer op = {{binding}}.get_op_{{node.name}}();
		Node.registerFactory(firm.bindings.binding_irop.get_op_code(op), new Factory());
	}

	{%- if spec.external %}
	public static Node create(
		{%- filter parameters %}
			{{node|blockparameter}}
			{{node|nodeparameters}}
		{%- endfilter %}) {
		return Node.createWrapper({{binding}}.new_r_{{node.name}}(
			{%- filter arguments %}
				{{node|blockargument}}
				{{node|nodearguments}}
			{%- endfilter %}));
	}

	public static Node create(Construction cons, {{node.arguments|argdecls}}) {
		return Node.createWrapper({{binding}}.new_r_{{node.name}}(
			{%- filter parameters %}
				{{node|block_construction}}
				{{node.arguments|bindingargs}}
			{%- endfilter %}));
	}
	{%- endif %}

	{%- endif %}

	public {{node.classname}}(Pointer ptr) {
		super(ptr);
	}

	{% for input in node.ins -%}
	{%if node.parent.classname != "Node"%}@Override
	{%endif-%}
	public Node get{{input[0]|CamelCase}}() {
		return createWrapper({{binding}}.get_{{node.name}}_{{input[0]}}(ptr));
	}

	{%if node.parent.classname != "Node"%}@Override
	{%endif-%}
	public void set{{input[0]|CamelCase}}(Node {{input[0]|filterkeywords}}) {
		{{binding}}.set_{{node.name}}_{{input[0]}}(this.ptr, {{input[0]|filterkeywords}}.ptr);
	}

	{% endfor -%}

	{%- if not isAbstract(node) %}
	{%- for attr in node.attrs -%}
	public {{attr.java_type}} get{{attr.java_name|CamelCase}}() {
		{{attr.wrap_type}} _res = {{binding}}.get_{{node.name}}_{{attr.name}}(ptr);
		return {{attr.from_wrapper % "_res"}};
	}

	public void set{{attr.java_name|CamelCase}}({{attr.java_type}} _val) {
		{{binding}}.set_{{node.name}}_{{attr.name}}(this.ptr, {{attr.to_wrapper % "_val"}});
	}

	{% endfor -%}
	{% endif -%}

	{{- node.java_add -}}
	{%- if not isAbstract(node) -%}
	@Override
	public void accept(NodeVisitor visitor) {
		{%- if not spec.external %}
		visitor.visit(this);
		{%- else %}
		visitor.visitUnknown(this);
		{%- endif %}
	}

	{% endif -%}

	{%- for out in node.outs -%}
	{%- if out[1] != "" -%}
	/** {{out[1]}} */
	{% endif -%}
	public static final int pn{{out[0]|CamelCase}} = {{loop.index0}};

	{% endfor -%}
	public static final int pnMax = {{len(node.outs)}};
}
