package firm;

import com.sun.jna.Pointer;

import firm.bindings.binding_oo;
import firm.nodes.Node;

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
		binding_oo.oo_lower();
	}

	/**
	 * lets you configure which methods should be included in the vtable
	 */
	public static void setMethodExcludeFromVTable(Entity entity, boolean includeInVTable) {
		binding_oo.oo_set_method_exclude_from_vtable(entity.ptr, includeInVTable);
	}

	/**
	 * lets you mark a method as abstract
	 */
	public static void setMethodAbstract(Entity entity, boolean isAbstract) {
		binding_oo.oo_set_method_is_abstract(entity.ptr, isAbstract);
	}

	/**
	 * @return true if the given field is transient
	 */
	public static boolean getFieldIsTransient(Entity entity) {
		return binding_oo.oo_get_field_is_transient(entity.ptr);
	}

	/**
	 * lets you mark a method as transient
	 */
	public static void setFieldIsTransient(Entity entity, boolean isTransient) {
		binding_oo.oo_set_field_is_transient(entity.ptr, isTransient);
	}

	/**
	 * lets you specify the binding mode of a method
	 */
	public static void setEntityBinding(Entity entity, binding_oo.ddispatch_binding binding) {
		binding_oo.oo_set_entity_binding(entity.ptr, binding.val);
	}

	/**
	 * Lets you specify static binding for a single call.
	 * This overwrites any possibly different binding information in the
	 * method entity referenced by the call.
	 */
	public static void setCallIsStaticallyBound(Node call, boolean isStaticallyBound) {
		binding_oo.oo_set_call_is_statically_bound(call.ptr, isStaticallyBound);
	}

	/**
	 * @return the unique compile time type id for the given class type.
	 */
	public static int getClassUID(ClassType classType) {
		return binding_oo.oo_get_class_uid(classType.ptr);
	}

	/**
	 * Sets the the unique compile time type id for the given class type.
	 */
	public static void setClassUID(ClassType classType, int uid) {
		binding_oo.oo_set_class_uid(classType.ptr, uid);
	}

	/**
	 * @return the previously set vtable entity for the given class type.
	 */
	public static Entity getClassVTableEntity(ClassType classType) {
		Pointer vtable = binding_oo.oo_get_class_vtable_entity(classType.ptr);
		if (vtable == Pointer.NULL)
			return null;
		return new Entity(vtable);
	}

	/**
	 * lets you specify the entity containing classType's vtable.
	 * Use an entity with a primitive pointer type, and set the ld name.
	 */
	public static void setClassVTableEntity(ClassType classType, Entity vtable) {
		binding_oo.oo_set_class_vtable_entity(classType.ptr, vtable.ptr);
	}

	/**
	 * lets you specify the entity that represents the pointer to the vtable in an instance
	 */
	public static void setClassVPtrEntity(ClassType classType, Entity entity) {
		binding_oo.oo_set_class_vptr_entity(classType.ptr, entity.ptr);
	}

	/**
	 * lets you specify the entity that represents the run-time type info data.
	 * Use an entity with a primitive pointer type, and set the ld name.
	 */
	public static void setClassRTTIEntity(ClassType classType, Entity entity) {
		binding_oo.oo_set_class_rtti_entity(classType.ptr, entity.ptr);
	}

	/**
	 * lets you specify whether the given classType is representing an interface type
	 */
	public static void setClassIsInterface(ClassType classType, boolean isInterface) {
		binding_oo.oo_set_class_is_interface(classType.ptr, isInterface);
	}
}
