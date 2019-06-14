package firm.bindings;

/* WARNING: Automatically generated file */
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class binding_oo {
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
		op_pin_state_exc_pinned();
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
		mtp_property_no_write((1 << 0)),
		mtp_property_pure((1 << 1)),
		mtp_property_noreturn((1 << 2)),
		mtp_property_terminates((1 << 3)),
		mtp_property_nothrow((1 << 4)),
		mtp_property_naked((1 << 5)),
		mtp_property_malloc((1 << 6)),
		mtp_property_returns_twice((1 << 7)),
		mtp_property_private((1 << 8)),
		mtp_property_always_inline((1 << 9)),
		mtp_property_noinline((1 << 10)),
		mtp_property_inline_recommended((1 << 11)),
		mtp_temporary((1 << 12)),
		mtp_property_is_constructor((1 << 13));
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
		ir_bk_saturating_increment(),
		ir_bk_compare_swap(),
		ir_bk_may_alias(),
		ir_bk_va_start(),
		ir_bk_va_arg(),
		ir_bk_last(ir_builtin_kind.ir_bk_va_arg.val);
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

	public static enum float_int_conversion_overflow_style_t {
		ir_overflow_indefinite(),
		ir_overflow_min_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		float_int_conversion_overflow_style_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		float_int_conversion_overflow_style_t() {
			this.val = C.next_val++;
		}

		public static float_int_conversion_overflow_style_t getEnum(int val) {
			for (float_int_conversion_overflow_style_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_visibility {
		ir_visibility_external(),
		ir_visibility_external_private(),
		ir_visibility_external_protected(),
		ir_visibility_local(),
		ir_visibility_private();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_visibility(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_visibility() {
			this.val = C.next_val++;
		}

		public static ir_visibility getEnum(int val) {
			for (ir_visibility entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_linkage {
		IR_LINKAGE_DEFAULT(0),
		IR_LINKAGE_CONSTANT((1 << 0)),
		IR_LINKAGE_WEAK((1 << 1)),
		IR_LINKAGE_GARBAGE_COLLECT((1 << 2)),
		IR_LINKAGE_MERGE((1 << 3)),
		IR_LINKAGE_HIDDEN_USER((1 << 4)),
		IR_LINKAGE_NO_CODEGEN((1 << 5)),
		IR_LINKAGE_NO_IDENTITY((1 << 6));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_linkage(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_linkage() {
			this.val = C.next_val++;
		}

		public static ir_linkage getEnum(int val) {
			for (ir_linkage entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_entity_usage {
		ir_usage_none(0),
		ir_usage_address_taken((1 << 0)),
		ir_usage_write((1 << 1)),
		ir_usage_read((1 << 2)),
		ir_usage_reinterpret_cast((1 << 3)),
		ir_usage_unknown((((ir_entity_usage.ir_usage_address_taken.val | ir_entity_usage.ir_usage_write.val) | ir_entity_usage.ir_usage_read.val) | ir_entity_usage.ir_usage_reinterpret_cast.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_entity_usage(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_entity_usage() {
			this.val = C.next_val++;
		}

		public static ir_entity_usage getEnum(int val) {
			for (ir_entity_usage entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_initializer_kind_t {
		IR_INITIALIZER_CONST(),
		IR_INITIALIZER_TARVAL(),
		IR_INITIALIZER_NULL(),
		IR_INITIALIZER_COMPOUND();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_initializer_kind_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_initializer_kind_t() {
			this.val = C.next_val++;
		}

		public static ir_initializer_kind_t getEnum(int val) {
			for (ir_initializer_kind_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ptr_access_kind {
		ptr_access_none(0),
		ptr_access_read(1),
		ptr_access_write(2),
		ptr_access_rw((ptr_access_kind.ptr_access_read.val | ptr_access_kind.ptr_access_write.val)),
		ptr_access_store(4),
		ptr_access_all((ptr_access_kind.ptr_access_rw.val | ptr_access_kind.ptr_access_store.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ptr_access_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ptr_access_kind() {
			this.val = C.next_val++;
		}

		public static ptr_access_kind getEnum(int val) {
			for (ptr_access_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum tp_opcode {
		tpo_uninitialized(0),
		tpo_struct(),
		tpo_union(),
		tpo_class(),
		tpo_segment(),
		tpo_method(),
		tpo_array(),
		tpo_pointer(),
		tpo_primitive(),
		tpo_code(),
		tpo_unknown(),
		tpo_last(tp_opcode.tpo_unknown.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		tp_opcode(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		tp_opcode() {
			this.val = C.next_val++;
		}

		public static tp_opcode getEnum(int val) {
			for (tp_opcode entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum inh_transitive_closure_state {
		inh_transitive_closure_none(),
		inh_transitive_closure_valid(),
		inh_transitive_closure_invalid(),
		inh_transitive_closure_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		inh_transitive_closure_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		inh_transitive_closure_state() {
			this.val = C.next_val++;
		}

		public static inh_transitive_closure_state getEnum(int val) {
			for (inh_transitive_closure_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_type_state {
		layout_undefined(),
		layout_fixed();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_type_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_type_state() {
			this.val = C.next_val++;
		}

		public static ir_type_state getEnum(int val) {
			for (ir_type_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum calling_convention {
		cc_reg_param(0x01000000),
		cc_last_on_top(0x02000000),
		cc_callee_clear_stk(0x04000000),
		cc_this_call(0x08000000),
		cc_compound_ret(0x10000000),
		cc_frame_on_caller_stk(0x20000000),
		cc_fpreg_param(0x40000000);
		public final int val;

		private static class C {
			static int next_val;
		}

		calling_convention(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		calling_convention() {
			this.val = C.next_val++;
		}

		public static calling_convention getEnum(int val) {
			for (calling_convention entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum __codecvt_result {
		__codecvt_ok(),
		__codecvt_partial(),
		__codecvt_error(),
		__codecvt_noconv();
		public final int val;

		private static class C {
			static int next_val;
		}

		__codecvt_result(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		__codecvt_result() {
			this.val = C.next_val++;
		}

		public static __codecvt_result getEnum(int val) {
			for (__codecvt_result entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum osr_flags {
		osr_flag_none(0),
		osr_flag_lftr_with_ov_check(1),
		osr_flag_ignore_x86_shift(2),
		osr_flag_keep_reg_pressure(4);
		public final int val;

		private static class C {
			static int next_val;
		}

		osr_flags(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		osr_flags() {
			this.val = C.next_val++;
		}

		public static osr_flags getEnum(int val) {
			for (osr_flags entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_mode_arithmetic {
		irma_none(1),
		irma_twos_complement(2),
		irma_ieee754(256),
		irma_x86_extended_float(),
		irma_last(ir_mode_arithmetic.irma_x86_extended_float.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_mode_arithmetic(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_mode_arithmetic() {
			this.val = C.next_val++;
		}

		public static ir_mode_arithmetic getEnum(int val) {
			for (ir_mode_arithmetic entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum asm_constraint_flags_t {
		ASM_CONSTRAINT_FLAG_NONE(0),
		ASM_CONSTRAINT_FLAG_SUPPORTS_REGISTER((1 << 0)),
		ASM_CONSTRAINT_FLAG_SUPPORTS_MEMOP((1 << 1)),
		ASM_CONSTRAINT_FLAG_SUPPORTS_IMMEDIATE((1 << 2)),
		ASM_CONSTRAINT_FLAG_SUPPORTS_ANY(((asm_constraint_flags_t.ASM_CONSTRAINT_FLAG_SUPPORTS_IMMEDIATE.val | asm_constraint_flags_t.ASM_CONSTRAINT_FLAG_SUPPORTS_MEMOP.val) | asm_constraint_flags_t.ASM_CONSTRAINT_FLAG_SUPPORTS_REGISTER.val)),
		ASM_CONSTRAINT_FLAG_NO_SUPPORT((1 << 3)),
		ASM_CONSTRAINT_FLAG_MODIFIER_WRITE((1 << 4)),
		ASM_CONSTRAINT_FLAG_MODIFIER_READ((1 << 5)),
		ASM_CONSTRAINT_FLAG_MODIFIER_EARLYCLOBBER((1 << 6)),
		ASM_CONSTRAINT_FLAG_MODIFIER_COMMUTATIVE((1 << 7)),
		ASM_CONSTRAINT_FLAG_INVALID((1 << 8));
		public final int val;

		private static class C {
			static int next_val;
		}

		asm_constraint_flags_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		asm_constraint_flags_t() {
			this.val = C.next_val++;
		}

		public static asm_constraint_flags_t getEnum(int val) {
			for (asm_constraint_flags_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum dwarf_source_language {
		DW_LANG_C89(0x0001),
		DW_LANG_C(0x0002),
		DW_LANG_Ada83(0x0003),
		DW_LANG_C_plus_plus(0x0004),
		DW_LANG_Cobol74(0x0005),
		DW_LANG_Cobol85(0x0006),
		DW_LANG_Fortran77(0x0007),
		DW_LANG_Fortran90(0x0008),
		DW_LANG_Pascal83(0x0009),
		DW_LANG_Modula2(0x000a),
		DW_LANG_Java(0x000b),
		DW_LANG_C99(0x000c),
		DW_LANG_Ada95(0x000d),
		DW_LANG_Fortran95(0x000e),
		DW_LANG_PLI(0x000f),
		DW_LANG_ObjC(0x0010),
		DW_LANG_ObjC_plus_plus(0x0011),
		DW_LANG_UPC(0x0012),
		DW_LANG_D(0x0013),
		DW_LANG_Python(0x0014),
		DW_LANG_Go(0x0016);
		public final int val;

		private static class C {
			static int next_val;
		}

		dwarf_source_language(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		dwarf_source_language() {
			this.val = C.next_val++;
		}

		public static dwarf_source_language getEnum(int val) {
			for (dwarf_source_language entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irp_callgraph_state {
		irp_callgraph_none(),
		irp_callgraph_consistent(),
		irp_callgraph_inconsistent(),
		irp_callgraph_and_calltree_consistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		irp_callgraph_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irp_callgraph_state() {
			this.val = C.next_val++;
		}

		public static irp_callgraph_state getEnum(int val) {
			for (irp_callgraph_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum loop_nesting_depth_state {
		loop_nesting_depth_none(),
		loop_nesting_depth_consistent(),
		loop_nesting_depth_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		loop_nesting_depth_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		loop_nesting_depth_state() {
			this.val = C.next_val++;
		}

		public static loop_nesting_depth_state getEnum(int val) {
			for (loop_nesting_depth_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum dbg_action {
		dbg_error(0),
		dbg_opt_ssa(),
		dbg_opt_auxnode(),
		dbg_const_eval(),
		dbg_opt_cse(),
		dbg_straightening(),
		dbg_if_simplification(),
		dbg_algebraic_simplification(),
		dbg_write_after_write(),
		dbg_write_after_read(),
		dbg_read_after_write(),
		dbg_read_after_read(),
		dbg_read_a_const(),
		dbg_dead_code(),
		dbg_opt_confirm(),
		dbg_gvn_pre(),
		dbg_combo(),
		dbg_jumpthreading(),
		dbg_backend(),
		dbg_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		dbg_action(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		dbg_action() {
			this.val = C.next_val++;
		}

		public static dbg_action getEnum(int val) {
			for (dbg_action entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum firm_kind {
		k_BAD(0),
		k_entity(),
		k_type(),
		k_ir_graph(),
		k_ir_node(),
		k_ir_mode(),
		k_tarval(),
		k_ir_loop(),
		k_ir_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		firm_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		firm_kind() {
			this.val = C.next_val++;
		}

		public static firm_kind getEnum(int val) {
			for (firm_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum op_arity {
		oparity_invalid(0),
		oparity_binary(),
		oparity_dynamic(),
		oparity_any();
		public final int val;

		private static class C {
			static int next_val;
		}

		op_arity(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		op_arity() {
			this.val = C.next_val++;
		}

		public static op_arity getEnum(int val) {
			for (op_arity entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irop_flags {
		irop_flag_none(0),
		irop_flag_commutative((1 << 0)),
		irop_flag_cfopcode((1 << 1)),
		irop_flag_fragile((1 << 2)),
		irop_flag_forking((1 << 3)),
		irop_flag_constlike((1 << 5)),
		irop_flag_keep((1 << 6)),
		irop_flag_start_block((1 << 7)),
		irop_flag_uses_memory((1 << 8)),
		irop_flag_dump_noblock((1 << 9)),
		irop_flag_unknown_jump((1 << 11)),
		irop_flag_const_memory((1 << 12));
		public final int val;

		private static class C {
			static int next_val;
		}

		irop_flags(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irop_flags() {
			this.val = C.next_val++;
		}

		public static irop_flags getEnum(int val) {
			for (irop_flags entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum dump_reason_t {
		dump_node_opcode_txt(),
		dump_node_mode_txt(),
		dump_node_nodeattr_txt(),
		dump_node_info_txt();
		public final int val;

		private static class C {
			static int next_val;
		}

		dump_reason_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		dump_reason_t() {
			this.val = C.next_val++;
		}

		public static dump_reason_t getEnum(int val) {
			for (dump_reason_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_opcode {
		iro_ASM(),
		iro_Add(),
		iro_Address(),
		iro_Align(),
		iro_Alloc(),
		iro_Anchor(),
		iro_And(),
		iro_Bad(),
		iro_Bitcast(),
		iro_Block(),
		iro_Builtin(),
		iro_Call(),
		iro_Cmp(),
		iro_Cond(),
		iro_Confirm(),
		iro_Const(),
		iro_Conv(),
		iro_CopyB(),
		iro_Deleted(),
		iro_Div(),
		iro_Dummy(),
		iro_End(),
		iro_Eor(),
		iro_Free(),
		iro_IJmp(),
		iro_Id(),
		iro_Jmp(),
		iro_Load(),
		iro_Member(),
		iro_Minus(),
		iro_Mod(),
		iro_Mul(),
		iro_Mulh(),
		iro_Mux(),
		iro_NoMem(),
		iro_Not(),
		iro_Offset(),
		iro_Or(),
		iro_Phi(),
		iro_Pin(),
		iro_Proj(),
		iro_Raise(),
		iro_Return(),
		iro_Sel(),
		iro_Shl(),
		iro_Shr(),
		iro_Shrs(),
		iro_Size(),
		iro_Start(),
		iro_Store(),
		iro_Sub(),
		iro_Switch(),
		iro_Sync(),
		iro_Tuple(),
		iro_Unknown(),
		iro_first(ir_opcode.iro_ASM.val),
		iro_last(ir_opcode.iro_Unknown.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_opcode(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_opcode() {
			this.val = C.next_val++;
		}

		public static ir_opcode getEnum(int val) {
			for (ir_opcode entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_ASM {
		n_ASM_mem(),
		n_ASM_max(n_ASM.n_ASM_mem.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_ASM(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_ASM() {
			this.val = C.next_val++;
		}

		public static n_ASM getEnum(int val) {
			for (n_ASM entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_ASM {
		pn_ASM_M(),
		pn_ASM_X_regular(),
		pn_ASM_first_out(),
		pn_ASM_max(pn_ASM.pn_ASM_first_out.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_ASM(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_ASM() {
			this.val = C.next_val++;
		}

		public static pn_ASM getEnum(int val) {
			for (pn_ASM entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Add {
		n_Add_left(),
		n_Add_right(),
		n_Add_max(n_Add.n_Add_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Add(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Add() {
			this.val = C.next_val++;
		}

		public static n_Add getEnum(int val) {
			for (n_Add entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Alloc {
		n_Alloc_mem(),
		n_Alloc_size(),
		n_Alloc_max(n_Alloc.n_Alloc_size.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Alloc(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Alloc() {
			this.val = C.next_val++;
		}

		public static n_Alloc getEnum(int val) {
			for (n_Alloc entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Alloc {
		pn_Alloc_M(),
		pn_Alloc_res(),
		pn_Alloc_max(pn_Alloc.pn_Alloc_res.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Alloc(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Alloc() {
			this.val = C.next_val++;
		}

		public static pn_Alloc getEnum(int val) {
			for (pn_Alloc entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Anchor {
		n_Anchor_end_block(),
		n_Anchor_start_block(),
		n_Anchor_end(),
		n_Anchor_start(),
		n_Anchor_frame(),
		n_Anchor_initial_mem(),
		n_Anchor_args(),
		n_Anchor_no_mem(),
		n_Anchor_max(n_Anchor.n_Anchor_no_mem.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Anchor(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Anchor() {
			this.val = C.next_val++;
		}

		public static n_Anchor getEnum(int val) {
			for (n_Anchor entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_And {
		n_And_left(),
		n_And_right(),
		n_And_max(n_And.n_And_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_And(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_And() {
			this.val = C.next_val++;
		}

		public static n_And getEnum(int val) {
			for (n_And entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Bitcast {
		n_Bitcast_op(),
		n_Bitcast_max(n_Bitcast.n_Bitcast_op.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Bitcast(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Bitcast() {
			this.val = C.next_val++;
		}

		public static n_Bitcast getEnum(int val) {
			for (n_Bitcast entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Builtin {
		n_Builtin_mem(),
		n_Builtin_max(n_Builtin.n_Builtin_mem.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Builtin(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Builtin() {
			this.val = C.next_val++;
		}

		public static n_Builtin getEnum(int val) {
			for (n_Builtin entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Builtin {
		pn_Builtin_M(),
		pn_Builtin_max(pn_Builtin.pn_Builtin_M.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Builtin(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Builtin() {
			this.val = C.next_val++;
		}

		public static pn_Builtin getEnum(int val) {
			for (pn_Builtin entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Call {
		n_Call_mem(),
		n_Call_ptr(),
		n_Call_max(n_Call.n_Call_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Call(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Call() {
			this.val = C.next_val++;
		}

		public static n_Call getEnum(int val) {
			for (n_Call entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Call {
		pn_Call_M(),
		pn_Call_T_result(),
		pn_Call_X_regular(),
		pn_Call_X_except(),
		pn_Call_max(pn_Call.pn_Call_X_except.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Call(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Call() {
			this.val = C.next_val++;
		}

		public static pn_Call getEnum(int val) {
			for (pn_Call entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Cmp {
		n_Cmp_left(),
		n_Cmp_right(),
		n_Cmp_max(n_Cmp.n_Cmp_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Cmp(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Cmp() {
			this.val = C.next_val++;
		}

		public static n_Cmp getEnum(int val) {
			for (n_Cmp entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Cond {
		n_Cond_selector(),
		n_Cond_max(n_Cond.n_Cond_selector.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Cond(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Cond() {
			this.val = C.next_val++;
		}

		public static n_Cond getEnum(int val) {
			for (n_Cond entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Cond {
		pn_Cond_false(),
		pn_Cond_true(),
		pn_Cond_max(pn_Cond.pn_Cond_true.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Cond(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Cond() {
			this.val = C.next_val++;
		}

		public static pn_Cond getEnum(int val) {
			for (pn_Cond entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Confirm {
		n_Confirm_value(),
		n_Confirm_bound(),
		n_Confirm_max(n_Confirm.n_Confirm_bound.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Confirm(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Confirm() {
			this.val = C.next_val++;
		}

		public static n_Confirm getEnum(int val) {
			for (n_Confirm entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Conv {
		n_Conv_op(),
		n_Conv_max(n_Conv.n_Conv_op.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Conv(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Conv() {
			this.val = C.next_val++;
		}

		public static n_Conv getEnum(int val) {
			for (n_Conv entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_CopyB {
		n_CopyB_mem(),
		n_CopyB_dst(),
		n_CopyB_src(),
		n_CopyB_max(n_CopyB.n_CopyB_src.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_CopyB(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_CopyB() {
			this.val = C.next_val++;
		}

		public static n_CopyB getEnum(int val) {
			for (n_CopyB entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Div {
		n_Div_mem(),
		n_Div_left(),
		n_Div_right(),
		n_Div_max(n_Div.n_Div_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Div(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Div() {
			this.val = C.next_val++;
		}

		public static n_Div getEnum(int val) {
			for (n_Div entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Div {
		pn_Div_M(),
		pn_Div_res(),
		pn_Div_X_regular(),
		pn_Div_X_except(),
		pn_Div_max(pn_Div.pn_Div_X_except.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Div(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Div() {
			this.val = C.next_val++;
		}

		public static pn_Div getEnum(int val) {
			for (pn_Div entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Eor {
		n_Eor_left(),
		n_Eor_right(),
		n_Eor_max(n_Eor.n_Eor_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Eor(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Eor() {
			this.val = C.next_val++;
		}

		public static n_Eor getEnum(int val) {
			for (n_Eor entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Free {
		n_Free_mem(),
		n_Free_ptr(),
		n_Free_max(n_Free.n_Free_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Free(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Free() {
			this.val = C.next_val++;
		}

		public static n_Free getEnum(int val) {
			for (n_Free entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_IJmp {
		n_IJmp_target(),
		n_IJmp_max(n_IJmp.n_IJmp_target.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_IJmp(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_IJmp() {
			this.val = C.next_val++;
		}

		public static n_IJmp getEnum(int val) {
			for (n_IJmp entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Id {
		n_Id_pred(),
		n_Id_max(n_Id.n_Id_pred.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Id(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Id() {
			this.val = C.next_val++;
		}

		public static n_Id getEnum(int val) {
			for (n_Id entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Load {
		n_Load_mem(),
		n_Load_ptr(),
		n_Load_max(n_Load.n_Load_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Load(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Load() {
			this.val = C.next_val++;
		}

		public static n_Load getEnum(int val) {
			for (n_Load entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Load {
		pn_Load_M(),
		pn_Load_res(),
		pn_Load_X_regular(),
		pn_Load_X_except(),
		pn_Load_max(pn_Load.pn_Load_X_except.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Load(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Load() {
			this.val = C.next_val++;
		}

		public static pn_Load getEnum(int val) {
			for (pn_Load entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Member {
		n_Member_ptr(),
		n_Member_max(n_Member.n_Member_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Member(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Member() {
			this.val = C.next_val++;
		}

		public static n_Member getEnum(int val) {
			for (n_Member entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Minus {
		n_Minus_op(),
		n_Minus_max(n_Minus.n_Minus_op.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Minus(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Minus() {
			this.val = C.next_val++;
		}

		public static n_Minus getEnum(int val) {
			for (n_Minus entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Mod {
		n_Mod_mem(),
		n_Mod_left(),
		n_Mod_right(),
		n_Mod_max(n_Mod.n_Mod_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Mod(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Mod() {
			this.val = C.next_val++;
		}

		public static n_Mod getEnum(int val) {
			for (n_Mod entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Mod {
		pn_Mod_M(),
		pn_Mod_res(),
		pn_Mod_X_regular(),
		pn_Mod_X_except(),
		pn_Mod_max(pn_Mod.pn_Mod_X_except.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Mod(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Mod() {
			this.val = C.next_val++;
		}

		public static pn_Mod getEnum(int val) {
			for (pn_Mod entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Mul {
		n_Mul_left(),
		n_Mul_right(),
		n_Mul_max(n_Mul.n_Mul_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Mul(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Mul() {
			this.val = C.next_val++;
		}

		public static n_Mul getEnum(int val) {
			for (n_Mul entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Mulh {
		n_Mulh_left(),
		n_Mulh_right(),
		n_Mulh_max(n_Mulh.n_Mulh_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Mulh(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Mulh() {
			this.val = C.next_val++;
		}

		public static n_Mulh getEnum(int val) {
			for (n_Mulh entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Mux {
		n_Mux_sel(),
		n_Mux_false(),
		n_Mux_true(),
		n_Mux_max(n_Mux.n_Mux_true.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Mux(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Mux() {
			this.val = C.next_val++;
		}

		public static n_Mux getEnum(int val) {
			for (n_Mux entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Not {
		n_Not_op(),
		n_Not_max(n_Not.n_Not_op.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Not(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Not() {
			this.val = C.next_val++;
		}

		public static n_Not getEnum(int val) {
			for (n_Not entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Or {
		n_Or_left(),
		n_Or_right(),
		n_Or_max(n_Or.n_Or_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Or(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Or() {
			this.val = C.next_val++;
		}

		public static n_Or getEnum(int val) {
			for (n_Or entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Pin {
		n_Pin_op(),
		n_Pin_max(n_Pin.n_Pin_op.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Pin(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Pin() {
			this.val = C.next_val++;
		}

		public static n_Pin getEnum(int val) {
			for (n_Pin entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Proj {
		n_Proj_pred(),
		n_Proj_max(n_Proj.n_Proj_pred.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Proj(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Proj() {
			this.val = C.next_val++;
		}

		public static n_Proj getEnum(int val) {
			for (n_Proj entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Raise {
		n_Raise_mem(),
		n_Raise_exo_ptr(),
		n_Raise_max(n_Raise.n_Raise_exo_ptr.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Raise(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Raise() {
			this.val = C.next_val++;
		}

		public static n_Raise getEnum(int val) {
			for (n_Raise entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Raise {
		pn_Raise_M(),
		pn_Raise_X(),
		pn_Raise_max(pn_Raise.pn_Raise_X.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Raise(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Raise() {
			this.val = C.next_val++;
		}

		public static pn_Raise getEnum(int val) {
			for (pn_Raise entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Return {
		n_Return_mem(),
		n_Return_max(n_Return.n_Return_mem.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Return(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Return() {
			this.val = C.next_val++;
		}

		public static n_Return getEnum(int val) {
			for (n_Return entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Sel {
		n_Sel_ptr(),
		n_Sel_index(),
		n_Sel_max(n_Sel.n_Sel_index.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Sel(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Sel() {
			this.val = C.next_val++;
		}

		public static n_Sel getEnum(int val) {
			for (n_Sel entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Shl {
		n_Shl_left(),
		n_Shl_right(),
		n_Shl_max(n_Shl.n_Shl_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Shl(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Shl() {
			this.val = C.next_val++;
		}

		public static n_Shl getEnum(int val) {
			for (n_Shl entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Shr {
		n_Shr_left(),
		n_Shr_right(),
		n_Shr_max(n_Shr.n_Shr_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Shr(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Shr() {
			this.val = C.next_val++;
		}

		public static n_Shr getEnum(int val) {
			for (n_Shr entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Shrs {
		n_Shrs_left(),
		n_Shrs_right(),
		n_Shrs_max(n_Shrs.n_Shrs_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Shrs(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Shrs() {
			this.val = C.next_val++;
		}

		public static n_Shrs getEnum(int val) {
			for (n_Shrs entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Start {
		pn_Start_M(),
		pn_Start_P_frame_base(),
		pn_Start_T_args(),
		pn_Start_max(pn_Start.pn_Start_T_args.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Start(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Start() {
			this.val = C.next_val++;
		}

		public static pn_Start getEnum(int val) {
			for (pn_Start entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Store {
		n_Store_mem(),
		n_Store_ptr(),
		n_Store_value(),
		n_Store_max(n_Store.n_Store_value.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Store(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Store() {
			this.val = C.next_val++;
		}

		public static n_Store getEnum(int val) {
			for (n_Store entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Store {
		pn_Store_M(),
		pn_Store_X_regular(),
		pn_Store_X_except(),
		pn_Store_max(pn_Store.pn_Store_X_except.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Store(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Store() {
			this.val = C.next_val++;
		}

		public static pn_Store getEnum(int val) {
			for (pn_Store entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Sub {
		n_Sub_left(),
		n_Sub_right(),
		n_Sub_max(n_Sub.n_Sub_right.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Sub(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Sub() {
			this.val = C.next_val++;
		}

		public static n_Sub getEnum(int val) {
			for (n_Sub entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum n_Switch {
		n_Switch_selector(),
		n_Switch_max(n_Switch.n_Switch_selector.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		n_Switch(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		n_Switch() {
			this.val = C.next_val++;
		}

		public static n_Switch getEnum(int val) {
			for (n_Switch entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Switch {
		pn_Switch_default(),
		pn_Switch_max(pn_Switch.pn_Switch_default.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Switch(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Switch() {
			this.val = C.next_val++;
		}

		public static pn_Switch getEnum(int val) {
			for (pn_Switch entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_dump_verbosity_t {
		dump_verbosity_onlynames(0x00000001),
		dump_verbosity_fields(0x00000002),
		dump_verbosity_methods(0x00000004),
		dump_verbosity_nostatic(0x00000040),
		dump_verbosity_typeattrs(0x00000008),
		dump_verbosity_entattrs(0x00000010),
		dump_verbosity_entconsts(0x00000020),
		dump_verbosity_accessStats(0x00000100),
		dump_verbosity_max(0x4FF00FBE);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_dump_verbosity_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_dump_verbosity_t() {
			this.val = C.next_val++;
		}

		public static ir_dump_verbosity_t getEnum(int val) {
			for (ir_dump_verbosity_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_dump_flags_t {
		ir_dump_flag_blocks_as_subgraphs((1 << 0)),
		ir_dump_flag_with_typegraph((1 << 1)),
		ir_dump_flag_disable_edge_labels((1 << 2)),
		ir_dump_flag_consts_local((1 << 3)),
		ir_dump_flag_idx_label((1 << 4)),
		ir_dump_flag_number_label((1 << 5)),
		ir_dump_flag_keepalive_edges((1 << 6)),
		ir_dump_flag_out_edges((1 << 7)),
		ir_dump_flag_dominance((1 << 8)),
		ir_dump_flag_loops((1 << 9)),
		ir_dump_flag_back_edges((1 << 10)),
		ir_dump_flag_iredges((1 << 11)),
		ir_dump_flag_all_anchors((1 << 12)),
		ir_dump_flag_show_marks((1 << 13)),
		ir_dump_flag_no_entity_values((1 << 14)),
		ir_dump_flag_ld_names((1 << 15)),
		ir_dump_flag_entities_in_hierarchy((1 << 16));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_dump_flags_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_dump_flags_t() {
			this.val = C.next_val++;
		}

		public static ir_dump_flags_t getEnum(int val) {
			for (ir_dump_flags_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_edge_kind_t {
		EDGE_KIND_NORMAL(),
		EDGE_KIND_FIRST(ir_edge_kind_t.EDGE_KIND_NORMAL.val),
		EDGE_KIND_BLOCK(),
		EDGE_KIND_LAST(ir_edge_kind_t.EDGE_KIND_BLOCK.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_edge_kind_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_edge_kind_t() {
			this.val = C.next_val++;
		}

		public static ir_edge_kind_t getEnum(int val) {
			for (ir_edge_kind_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_callee_info_state {
		irg_callee_info_none(),
		irg_callee_info_consistent(),
		irg_callee_info_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_callee_info_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_callee_info_state() {
			this.val = C.next_val++;
		}

		public static irg_callee_info_state getEnum(int val) {
			for (irg_callee_info_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_resources_t {
		IR_RESOURCE_NONE(0),
		IR_RESOURCE_BLOCK_VISITED((1 << 0)),
		IR_RESOURCE_BLOCK_MARK((1 << 1)),
		IR_RESOURCE_IRN_VISITED((1 << 2)),
		IR_RESOURCE_IRN_LINK((1 << 3)),
		IR_RESOURCE_LOOP_LINK((1 << 4)),
		IR_RESOURCE_PHI_LIST((1 << 5));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_resources_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_resources_t() {
			this.val = C.next_val++;
		}

		public static ir_resources_t getEnum(int val) {
			for (ir_resources_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_graph_constraints_t {
		IR_GRAPH_CONSTRAINT_ARCH_DEP((1 << 0)),
		IR_GRAPH_CONSTRAINT_MODEB_LOWERED((1 << 1)),
		IR_GRAPH_CONSTRAINT_NORMALISATION2((1 << 2)),
		IR_GRAPH_CONSTRAINT_OPTIMIZE_UNREACHABLE_CODE((1 << 3)),
		IR_GRAPH_CONSTRAINT_CONSTRUCTION((1 << 4)),
		IR_GRAPH_CONSTRAINT_TARGET_LOWERED((1 << 5)),
		IR_GRAPH_CONSTRAINT_BACKEND((1 << 6));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_graph_constraints_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_graph_constraints_t() {
			this.val = C.next_val++;
		}

		public static ir_graph_constraints_t getEnum(int val) {
			for (ir_graph_constraints_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_graph_properties_t {
		IR_GRAPH_PROPERTIES_NONE(0),
		IR_GRAPH_PROPERTY_NO_CRITICAL_EDGES((1 << 0)),
		IR_GRAPH_PROPERTY_NO_BADS((1 << 1)),
		IR_GRAPH_PROPERTY_NO_TUPLES((1 << 2)),
		IR_GRAPH_PROPERTY_NO_UNREACHABLE_CODE((1 << 3)),
		IR_GRAPH_PROPERTY_ONE_RETURN((1 << 4)),
		IR_GRAPH_PROPERTY_CONSISTENT_DOMINANCE((1 << 5)),
		IR_GRAPH_PROPERTY_CONSISTENT_POSTDOMINANCE((1 << 6)),
		IR_GRAPH_PROPERTY_CONSISTENT_DOMINANCE_FRONTIERS((1 << 7)),
		IR_GRAPH_PROPERTY_CONSISTENT_OUT_EDGES((1 << 8)),
		IR_GRAPH_PROPERTY_CONSISTENT_OUTS((1 << 9)),
		IR_GRAPH_PROPERTY_CONSISTENT_LOOPINFO((1 << 10)),
		IR_GRAPH_PROPERTY_CONSISTENT_ENTITY_USAGE((1 << 11)),
		IR_GRAPH_PROPERTY_MANY_RETURNS((1 << 12)),
		IR_GRAPH_PROPERTIES_CONTROL_FLOW(((((((ir_graph_properties_t.IR_GRAPH_PROPERTY_NO_CRITICAL_EDGES.val | ir_graph_properties_t.IR_GRAPH_PROPERTY_ONE_RETURN.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_NO_UNREACHABLE_CODE.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_LOOPINFO.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_DOMINANCE.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_POSTDOMINANCE.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_DOMINANCE_FRONTIERS.val)),
		IR_GRAPH_PROPERTIES_ALL(((((((ir_graph_properties_t.IR_GRAPH_PROPERTIES_CONTROL_FLOW.val | ir_graph_properties_t.IR_GRAPH_PROPERTY_NO_BADS.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_NO_TUPLES.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_OUT_EDGES.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_OUTS.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_CONSISTENT_ENTITY_USAGE.val) | ir_graph_properties_t.IR_GRAPH_PROPERTY_MANY_RETURNS.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_graph_properties_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_graph_properties_t() {
			this.val = C.next_val++;
		}

		public static ir_graph_properties_t getEnum(int val) {
			for (ir_graph_properties_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_alias_relation {
		ir_no_alias(),
		ir_may_alias(),
		ir_sure_alias();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_alias_relation(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_alias_relation() {
			this.val = C.next_val++;
		}

		public static ir_alias_relation getEnum(int val) {
			for (ir_alias_relation entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_entity_usage_computed_state {
		ir_entity_usage_not_computed(),
		ir_entity_usage_computed();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_entity_usage_computed_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_entity_usage_computed_state() {
			this.val = C.next_val++;
		}

		public static ir_entity_usage_computed_state getEnum(int val) {
			for (ir_entity_usage_computed_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_disambiguator_options {
		aa_opt_none(0),
		aa_opt_always_alias((1 << 0)),
		aa_opt_type_based((1 << 1)),
		aa_opt_byte_type_may_alias((1 << 2)),
		aa_opt_no_alias((1 << 3)),
		aa_opt_inherited((1 << 4));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_disambiguator_options(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_disambiguator_options() {
			this.val = C.next_val++;
		}

		public static ir_disambiguator_options getEnum(int val) {
			for (ir_disambiguator_options entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_segment_t {
		IR_SEGMENT_FIRST(),
		IR_SEGMENT_GLOBAL(ir_segment_t.IR_SEGMENT_FIRST.val),
		IR_SEGMENT_THREAD_LOCAL(),
		IR_SEGMENT_CONSTRUCTORS(),
		IR_SEGMENT_DESTRUCTORS(),
		IR_SEGMENT_JCR(),
		IR_SEGMENT_LAST(ir_segment_t.IR_SEGMENT_JCR.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_segment_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_segment_t() {
			this.val = C.next_val++;
		}

		public static ir_segment_t getEnum(int val) {
			for (ir_segment_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irp_resources_t {
		IRP_RESOURCE_NONE(0),
		IRP_RESOURCE_IRG_LINK((1 << 0)),
		IRP_RESOURCE_ENTITY_LINK((1 << 1)),
		IRP_RESOURCE_TYPE_VISITED((1 << 2)),
		IRP_RESOURCE_TYPE_LINK((1 << 3));
		public final int val;

		private static class C {
			static int next_val;
		}

		irp_resources_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irp_resources_t() {
			this.val = C.next_val++;
		}

		public static irp_resources_t getEnum(int val) {
			for (irp_resources_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ikind {
		INTRINSIC_CALL(0),
		INTRINSIC_INSTR();
		public final int val;

		private static class C {
			static int next_val;
		}

		ikind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ikind() {
			this.val = C.next_val++;
		}

		public static ikind getEnum(int val) {
			for (ikind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_platform_type_t {
		IR_TYPE_BOOL(),
		IR_TYPE_CHAR(),
		IR_TYPE_SHORT(),
		IR_TYPE_INT(),
		IR_TYPE_LONG(),
		IR_TYPE_LONG_LONG(),
		IR_TYPE_FLOAT(),
		IR_TYPE_DOUBLE(),
		IR_TYPE_LONG_DOUBLE();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_platform_type_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_platform_type_t() {
			this.val = C.next_val++;
		}

		public static ir_platform_type_t getEnum(int val) {
			for (ir_platform_type_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum range_types {
		VRP_UNDEFINED(),
		VRP_RANGE(),
		VRP_ANTIRANGE(),
		VRP_VARYING();
		public final int val;

		private static class C {
			static int next_val;
		}

		range_types(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		range_types() {
			this.val = C.next_val++;
		}

		public static range_types getEnum(int val) {
			for (range_types entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ddispatch_binding {
		bind_unknown(),
		bind_static(),
		bind_dynamic(),
		bind_interface();
		public final int val;

		private static class C {
			static int next_val;
		}

		ddispatch_binding(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ddispatch_binding() {
			this.val = C.next_val++;
		}

		public static ddispatch_binding getEnum(int val) {
			for (ddispatch_binding entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ddispatch_interface_call {
		call_runtime_lookup(0),
		call_searched_itable(1),
		call_itable_indexed(2),
		call_move2front(4);
		public final int val;

		private static class C {
			static int next_val;
		}

		ddispatch_interface_call(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ddispatch_interface_call() {
			this.val = C.next_val++;
		}

		public static ddispatch_interface_call getEnum(int val) {
			for (ddispatch_interface_call entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum idtype_t {
		P_ALL(),
		P_PID(),
		P_PGID();
		public final int val;

		private static class C {
			static int next_val;
		}

		idtype_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		idtype_t() {
			this.val = C.next_val++;
		}

		public static idtype_t getEnum(int val) {
			for (idtype_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}


	public static native void oo_init();

	public static native void oo_deinit();

	public static native void oo_lower();

	public static native void oo_set_interface_call_type(/* ddispatch_interface_call */int type);

	public static native /* ddispatch_interface_call */int oo_get_interface_call_type();

	public static native int oo_get_class_uid(Pointer classtype);

	public static native void oo_set_class_uid(Pointer classtype, int uid);

	public static native Pointer oo_get_class_vtable_entity(Pointer classtype);

	public static native void oo_set_class_vtable_entity(Pointer classtype, Pointer vtable);

	public static native int oo_get_class_vtable_size(Pointer classtype);

	public static native void oo_set_class_vtable_size(Pointer classtype, int vtable_size);

	public static native Pointer oo_get_class_vptr_entity(Pointer classtype);

	public static native void oo_set_class_vptr_entity(Pointer classtype, Pointer vptr);

	public static native Pointer oo_get_class_rtti_entity(Pointer classtype);

	public static native void oo_set_class_rtti_entity(Pointer classtype, Pointer rtti);

	public static native Pointer oo_get_class_itt_entity(Pointer classtype);

	public static native void oo_set_class_itt_entity(Pointer classtype, Pointer itt);

	public static native boolean oo_get_class_is_interface(Pointer classtype);

	public static native void oo_set_class_is_interface(Pointer classtype, boolean is_interface);

	public static native boolean oo_get_class_is_abstract(Pointer classtype);

	public static native void oo_set_class_is_abstract(Pointer classtype, boolean is_abstract);

	public static native boolean oo_get_class_is_final(Pointer classtype);

	public static native void oo_set_class_is_final(Pointer classtype, boolean is_final);

	public static native boolean oo_get_class_is_extern(Pointer classtype);

	public static native void oo_set_class_is_extern(Pointer classtype, boolean is_extern);

	public static native Pointer oo_get_type_link(Pointer type);

	public static native void oo_set_type_link(Pointer type, Pointer link);

	public static native boolean oo_get_method_exclude_from_vtable(Pointer method);

	public static native void oo_set_method_exclude_from_vtable(Pointer method, boolean exclude_from_vtable);

	public static native int oo_get_method_vtable_index(Pointer method);

	public static native void oo_set_method_vtable_index(Pointer method, int vtable_slot);

	public static native boolean oo_get_method_is_abstract(Pointer method);

	public static native void oo_set_method_is_abstract(Pointer method, boolean is_abstract);

	public static native boolean oo_get_method_is_final(Pointer method);

	public static native void oo_set_method_is_final(Pointer method, boolean is_final);

	public static native boolean oo_get_method_is_inherited(Pointer method);

	public static native void oo_set_method_is_inherited(Pointer method, boolean is_inherited);

	public static native boolean oo_get_field_is_transient(Pointer field);

	public static native void oo_set_field_is_transient(Pointer field, boolean is_transient);

	public static native /* ddispatch_binding */int oo_get_entity_binding(Pointer entity);

	public static native void oo_set_entity_binding(Pointer entity, /* ddispatch_binding */int binding);

	public static native Pointer oo_get_entity_link(Pointer entity);

	public static native void oo_set_entity_link(Pointer entity, Pointer link);

	public static native void oo_set_call_is_statically_bound(Pointer call, boolean bind_statically);

	public static native boolean oo_get_call_is_statically_bound(Pointer call);

	public static native Pointer oo_get_class_superclass(Pointer klass);

	public static native Pointer oo_get_entity_overwritten_superclass_entity(Pointer entity);

	public static native void oo_copy_entity_info(Pointer src, Pointer dest);
}
