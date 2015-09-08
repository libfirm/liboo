package firm;


import firm.bindings.binding_rta;

import com.sun.jna.Pointer;
import com.sun.jna.Callback;
import java.nio.Buffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;

import firm.nodes.Node;


public class RTA {

	public static interface DetectCallCallback extends Callback {
		Entity invoke(Node node);
	}

	public static void setCallback(DetectCallCallback callback) {
		binding_rta.rta_set_detection_callbacks(callback);
	}

	public static void runRTA(Entity[] entrypoints, Type[] initialLiveClasses) {
		binding_rta.rta_optimization(getBufferFromArray(entrypoints, true), getBufferFromArray(initialLiveClasses, true));
	}

	private static Buffer getBufferFromArray(JNAWrapper[] array, boolean nullterminate) {
		// taken from jFirm source code src/firm/nodes/Node.java and modified it
		// This should be placed in firm.JNAWrapper or similar!

		final int length = array.length;

		// There is no PointerBuffer so ...
		if (Pointer.SIZE == 4) {
			IntBuffer buffer = (nullterminate) ? IntBuffer.allocate(length+1) : IntBuffer.allocate(length);
			for (int i = 0; i < length; ++i) {
				buffer.put(i, (int) Pointer.nativeValue(array[i].ptr));
			}
			if (nullterminate)
				//buffer.put(length-1, (int)Pointer.nativeValue(Pointer.NULL));
				//buffer.put(length-1, (int)Pointer.NULL.getLong(0));
				buffer.put(length, 0);
			return buffer;
		} else if (Pointer.SIZE == 8) {
			LongBuffer buffer = (nullterminate) ? LongBuffer.allocate(length+1) : LongBuffer.allocate(length);
			for (int i = 0; i < length; ++i) {
				buffer.put(i, Pointer.nativeValue(array[i].ptr));
			}
			if (nullterminate)
				//buffer.put(length-1, (int)Pointer.nativeValue(Pointer.NULL));
				//buffer.put(length-1, Pointer.NULL.getLong(0));
				buffer.put(length, 0L);
			return buffer;
		} else {
			/* no 32- or 64-bit architecture */
			throw new RuntimeException("Unsupported architecture");
		}
	}
}
