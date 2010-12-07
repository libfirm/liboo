package firm;

import firm.bindings.binding_mangle;
import firm.bindings.binding_oo;

/**
 * Object-Orientation helper library API
 */
public final class OO {
	private static boolean initialized;

	private OO() {
	}

	/**
	 * initializes OO library. Must be called once before using any other
	 * functions from this class.
	 */
	public static void init() {
		assert initialized == false; /* we may only use 1 OO object at once... */
		binding_oo.oo_init();
	}

	/**
	 * deinitializes OO library. This frees some allocated resources
	 */
	public static void deinit() {
		binding_oo.oo_deinit();
		initialized = false;
	}

	/**
	 * Lower Object-Oriented constructs into low-level stuff
	 */
	public static void lowerProgram() {
		binding_oo.lower_oo();
	}

	/**
	 * lets you configure which compound-types need a vtable
	 */
	public static void setClassOmitVTable(CompoundType type, boolean needsVTable) {
		binding_oo.set_class_omit_vtable(type.ptr, needsVTable);
	}

	/**
	 * lets you configure which methods should be included in the vtable
	 */
	public static void setMethodExcludeFromVTable(Entity entity, boolean includeInVTable) {
		binding_oo.set_method_exclude_from_vtable(entity.ptr, includeInVTable);
	}

	/**
	 * lets you mark a method as abstract
	 */
	public static void setMethodAbstract(Entity entity, boolean isAbstract) {
		binding_oo.set_method_is_abstract(entity.ptr, isAbstract);
	}

	/**
	 * lets you specify the binding mode of a method
	 */
	public static void setEntityBinding(Entity entity, binding_oo.ddispatch_binding binding) {
		binding_oo.set_entity_binding(entity.ptr, binding.val);
	}

	public static void setClassVTableEntity(ClassType classType, Entity entity) {
		binding_oo.set_class_vptr_entity(classType.ptr, entity.ptr);
	}

	/**
	 * define names for primitive names for the name mangling
	 */
	public static void setPrimitiveTypeName(Type type, String name) {
		binding_mangle.mangle_set_primitive_type_name(type.ptr, name);
	}

	/**
	 * define method name substitions for name mangling
	 * (typically used for operator X methods)
	 */
	public static void addNameSubstitution(String name, String mangled) {
		binding_mangle.mangle_add_name_substitution(name, mangled);
	}
}
