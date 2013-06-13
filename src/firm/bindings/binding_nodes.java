package firm.bindings;

/* WARNING: Automatically generated file */
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class binding_nodes {
	static {
		Native.register("oo");
	}

	public static enum ir_relation {
		ir_relation_false(0),
		ir_relation_equal((1 << 0)),
		ir_relation_less((1 << 1)),
		ir_relation_greater((1 << 2)),
		ir_relation_unordered((1 << 3)),
		ir_relation_less_equal((ir_relation.ir_relation_equal.val | ir_relation.ir_relation_less.val)),
		ir_relation_greater_equal((ir_relation.ir_relation_equal.val | ir_relation.ir_relation_greater.val)),
		ir_relation_less_greater((ir_relation.ir_relation_less.val | ir_relation.ir_relation_greater.val)),
		ir_relation_less_equal_greater(((ir_relation.ir_relation_equal.val | ir_relation.ir_relation_less.val) | ir_relation.ir_relation_greater.val)),
		ir_relation_unordered_equal((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_equal.val)),
		ir_relation_unordered_less((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_less.val)),
		ir_relation_unordered_less_equal(((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_less.val) | ir_relation.ir_relation_equal.val)),
		ir_relation_unordered_greater((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_greater.val)),
		ir_relation_unordered_greater_equal(((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_greater.val) | ir_relation.ir_relation_equal.val)),
		ir_relation_unordered_less_greater(((ir_relation.ir_relation_unordered.val | ir_relation.ir_relation_less.val) | ir_relation.ir_relation_greater.val)),
		ir_relation_true((((ir_relation.ir_relation_equal.val | ir_relation.ir_relation_less.val) | ir_relation.ir_relation_greater.val) | ir_relation.ir_relation_unordered.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_relation(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_relation() {
			this.val = C.next_val++;
		}

		public static ir_relation getEnum(int val) {
			for (ir_relation entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_cons_flags {
		cons_none(0),
		cons_volatile((1 << 0)),
		cons_unaligned((1 << 1)),
		cons_floats((1 << 2)),
		cons_throws_exception((1 << 3));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_cons_flags(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_cons_flags() {
			this.val = C.next_val++;
		}

		public static ir_cons_flags getEnum(int val) {
			for (ir_cons_flags entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum op_pin_state {
		op_pin_state_floats(0),
		op_pin_state_pinned(1),
		op_pin_state_exc_pinned(),
		op_pin_state_mem_pinned();
		public final int val;

		private static class C {
			static int next_val;
		}

		op_pin_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		op_pin_state() {
			this.val = C.next_val++;
		}

		public static op_pin_state getEnum(int val) {
			for (op_pin_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum cond_jmp_predicate {
		COND_JMP_PRED_NONE(),
		COND_JMP_PRED_TRUE(),
		COND_JMP_PRED_FALSE();
		public final int val;

		private static class C {
			static int next_val;
		}

		cond_jmp_predicate(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		cond_jmp_predicate() {
			this.val = C.next_val++;
		}

		public static cond_jmp_predicate getEnum(int val) {
			for (cond_jmp_predicate entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum mtp_additional_properties {
		mtp_no_property(0),
		mtp_property_const((1 << 0)),
		mtp_property_pure((1 << 1)),
		mtp_property_noreturn((1 << 2)),
		mtp_property_nothrow((1 << 3)),
		mtp_property_naked((1 << 4)),
		mtp_property_malloc((1 << 5)),
		mtp_property_returns_twice((1 << 6)),
		mtp_property_private((1 << 7)),
		mtp_property_has_loop((1 << 8)),
		mtp_property_always_inline((1 << 9)),
		mtp_property_noinline((1 << 10)),
		mtp_property_inline_recommended((1 << 11)),
		mtp_temporary((1 << 12));
		public final int val;

		private static class C {
			static int next_val;
		}

		mtp_additional_properties(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		mtp_additional_properties() {
			this.val = C.next_val++;
		}

		public static mtp_additional_properties getEnum(int val) {
			for (mtp_additional_properties entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum symconst_kind {
		symconst_type_size(),
		symconst_type_align(),
		symconst_addr_ent(),
		symconst_ofs_ent(),
		symconst_enum_const();
		public final int val;

		private static class C {
			static int next_val;
		}

		symconst_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		symconst_kind() {
			this.val = C.next_val++;
		}

		public static symconst_kind getEnum(int val) {
			for (symconst_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_builtin_kind {
		ir_bk_trap(),
		ir_bk_debugbreak(),
		ir_bk_return_address(),
		ir_bk_frame_address(),
		ir_bk_prefetch(),
		ir_bk_ffs(),
		ir_bk_clz(),
		ir_bk_ctz(),
		ir_bk_popcount(),
		ir_bk_parity(),
		ir_bk_bswap(),
		ir_bk_inport(),
		ir_bk_outport(),
		ir_bk_inner_trampoline(),
		ir_bk_saturating_increment(),
		ir_bk_compare_swap(),
		ir_bk_last(ir_builtin_kind.ir_bk_compare_swap.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_builtin_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_builtin_kind() {
			this.val = C.next_val++;
		}

		public static ir_builtin_kind getEnum(int val) {
			for (ir_builtin_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_volatility {
		volatility_non_volatile(),
		volatility_is_volatile();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_volatility(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_volatility() {
			this.val = C.next_val++;
		}

		public static ir_volatility getEnum(int val) {
			for (ir_volatility entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_align {
		align_is_aligned(0),
		align_non_aligned();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_align(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_align() {
			this.val = C.next_val++;
		}

		public static ir_align getEnum(int val) {
			for (ir_align entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum oo_opcode {
		ooo_Arraylength(),
		ooo_InstanceOf(),
		ooo_first(oo_opcode.ooo_Arraylength.val),
		ooo_last(oo_opcode.ooo_InstanceOf.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		oo_opcode(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		oo_opcode() {
			this.val = C.next_val++;
		}

		public static oo_opcode getEnum(int val) {
			for (oo_opcode entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Arraylength {
		n_Arraylength_mem(),
		n_Arraylength_ptr(),
		n_Arraylength_max(n_Arraylength.n_Arraylength_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Arraylength(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Arraylength() {
			this.val = C.next_val++;
		}

		public static n_Arraylength getEnum(int val) {
			for (n_Arraylength entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Arraylength {
		pn_Arraylength_M(),
		pn_Arraylength_res(),
		pn_Arraylength_max(pn_Arraylength.pn_Arraylength_res.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Arraylength(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Arraylength() {
			this.val = C.next_val++;
		}

		public static pn_Arraylength getEnum(int val) {
			for (pn_Arraylength entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_InstanceOf {
		n_InstanceOf_mem(),
		n_InstanceOf_ptr(),
		n_InstanceOf_max(n_InstanceOf.n_InstanceOf_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_InstanceOf(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_InstanceOf() {
			this.val = C.next_val++;
		}

		public static n_InstanceOf getEnum(int val) {
			for (n_InstanceOf entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_InstanceOf {
		pn_InstanceOf_M(),
		pn_InstanceOf_res(),
		pn_InstanceOf_max(pn_InstanceOf.pn_InstanceOf_res.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_InstanceOf(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_InstanceOf() {
			this.val = C.next_val++;
		}

		public static pn_InstanceOf getEnum(int val) {
			for (pn_InstanceOf entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}


	public static native int is_oo_node(Pointer node);

	public static native /* oo_opcode */int get_oo_irn_opcode(Pointer node);

	public static native Pointer new_rd_Arraylength(Pointer dbgi, Pointer block, Pointer irn_mem, Pointer irn_ptr);

	public static native Pointer new_r_Arraylength(Pointer block, Pointer irn_mem, Pointer irn_ptr);

	public static native Pointer new_d_Arraylength(Pointer dbgi, Pointer irn_mem, Pointer irn_ptr);

	public static native Pointer new_Arraylength(Pointer irn_mem, Pointer irn_ptr);

	public static native int is_Arraylength(Pointer node);

	public static native Pointer get_Arraylength_mem(Pointer node);

	public static native void set_Arraylength_mem(Pointer node, Pointer mem);

	public static native Pointer get_Arraylength_ptr(Pointer node);

	public static native void set_Arraylength_ptr(Pointer node, Pointer ptr);

	public static native Pointer get_op_Arraylength();

	public static native Pointer new_rd_InstanceOf(Pointer dbgi, Pointer block, Pointer irn_mem, Pointer irn_ptr, Pointer type);

	public static native Pointer new_r_InstanceOf(Pointer block, Pointer irn_mem, Pointer irn_ptr, Pointer type);

	public static native Pointer new_d_InstanceOf(Pointer dbgi, Pointer irn_mem, Pointer irn_ptr, Pointer type);

	public static native Pointer new_InstanceOf(Pointer irn_mem, Pointer irn_ptr, Pointer type);

	public static native int is_InstanceOf(Pointer node);

	public static native Pointer get_InstanceOf_mem(Pointer node);

	public static native void set_InstanceOf_mem(Pointer node, Pointer mem);

	public static native Pointer get_InstanceOf_ptr(Pointer node);

	public static native void set_InstanceOf_ptr(Pointer node, Pointer ptr);

	public static native Pointer get_InstanceOf_type(Pointer node);

	public static native void set_InstanceOf_type(Pointer node, Pointer type);

	public static native Pointer get_op_InstanceOf();
}
