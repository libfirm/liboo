package firm;

import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class DebugInfo {
	static { Native.register("oo"); }
	public static native void debuginfo_init();
	public static native void debuginfo_deinit();
	public static native Pointer create_debuginfo(Pointer filename_ident,
	                                              int line, int column,
	                                              int length);
}