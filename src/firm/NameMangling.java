package firm;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class NameMangling {
	public void beginMangling() {
		libOOBindings.mangle_init();
	}
	
	public void endMangling() {
		libOOBindings.mangle_deinit();
	}
	
	public Ident mangle(Entity entity) {
		return new Ident(libOOBindings.mangle_entity_name(entity.ptr));
	}
	
	public Ident getVTableName(ClassType type) {
		return new Ident(libOOBindings.mangle_vtable_name(type.ptr));
	}
	
	public void lowerOO() {
		libOOBindings.lower_oo();
	}
	
	public void setPrimitiveTypeName(Type type, String name) {
		libOOBindings.mangle_set_primitive_type_name(type.ptr, name);
	}
	
	public void addNameSubstitution(String name, String mangled) {
		libOOBindings.mangle_add_name_substitution(name, mangled);
	}
	
	private static class libOOBindings {
		static { Native.register("oo"); }
		public static native void mangle_init();
		public static native void mangle_deinit();
		public static native Pointer mangle_entity_name(Pointer entity);
		public static native Pointer mangle_vtable_name(Pointer clazz);
		public static native void mangle_set_primitive_type_name(Pointer type, String name);
		public static native void mangle_add_name_substitution(String name, String mangled);
		
		public static native void lower_oo();
	}
}
