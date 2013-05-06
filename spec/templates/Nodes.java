{{warning}}
package {{package}};

public class Nodes {
	public static void init() {
		{%- for node in nodes -%}
		{% if not isAbstract(node) %}
		{{node.classname}}.init();
		{%- endif -%}
		{% endfor %}
	}
}
