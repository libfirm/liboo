package firm.bindings;

/* WARNING: Automatically generated file */
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class binding_mangle {
	static {
		Native.register("oo");
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
		mtp_no_property(0x00000000),
		mtp_property_const(0x00000001),
		mtp_property_pure(0x00000002),
		mtp_property_noreturn(0x00000004),
		mtp_property_nothrow(0x00000008),
		mtp_property_naked(0x00000010),
		mtp_property_malloc(0x00000020),
		mtp_property_returns_twice(0x00000040),
		mtp_property_intrinsic(0x00000080),
		mtp_property_runtime(0x00000100),
		mtp_property_private(0x00000200),
		mtp_property_has_loop(0x00000400),
		mtp_property_inherited((1 << 31));
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
		symconst_type_tag(),
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

	public static enum ir_where_alloc {
		stack_alloc(),
		heap_alloc();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_where_alloc(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_where_alloc() {
			this.val = C.next_val++;
		}

		public static ir_where_alloc getEnum(int val) {
			for (ir_where_alloc entry : values()) {
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
		ir_bk_inner_trampoline();
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

	public static enum pn_generic {
		pn_Generic_M(0),
		pn_Generic_X_regular(1),
		pn_Generic_X_except(2),
		pn_Generic_other(3);
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_generic(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_generic() {
			this.val = C.next_val++;
		}

		public static pn_generic getEnum(int val) {
			for (pn_generic entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_value_classify_sign {
		value_classified_unknown(0),
		value_classified_positive(1),
		value_classified_negative(-1);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_value_classify_sign(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_value_classify_sign() {
			this.val = C.next_val++;
		}

		public static ir_value_classify_sign getEnum(int val) {
			for (ir_value_classify_sign entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_phase_state {
		phase_building(),
		phase_high(),
		phase_low(),
		phase_backend();
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_phase_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_phase_state() {
			this.val = C.next_val++;
		}

		public static irg_phase_state getEnum(int val) {
			for (irg_phase_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_outs_state {
		outs_none(),
		outs_consistent(),
		outs_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_outs_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_outs_state() {
			this.val = C.next_val++;
		}

		public static irg_outs_state getEnum(int val) {
			for (irg_outs_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_extblk_info_state {
		ir_extblk_info_none(0),
		ir_extblk_info_valid(1),
		ir_extblk_info_invalid(2);
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_extblk_info_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_extblk_info_state() {
			this.val = C.next_val++;
		}

		public static irg_extblk_info_state getEnum(int val) {
			for (irg_extblk_info_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_dom_state {
		dom_none(),
		dom_consistent(),
		dom_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_dom_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_dom_state() {
			this.val = C.next_val++;
		}

		public static irg_dom_state getEnum(int val) {
			for (irg_dom_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_loopinfo_state {
		loopinfo_none(0),
		loopinfo_constructed(1),
		loopinfo_valid(2),
		loopinfo_cf(4),
		loopinfo_inter(8),
		loopinfo_consistent((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_valid.val)),
		loopinfo_inconsistent(irg_loopinfo_state.loopinfo_constructed.val),
		loopinfo_ip_consistent(((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_inter.val) | irg_loopinfo_state.loopinfo_valid.val)),
		loopinfo_ip_inconsistent((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_inter.val)),
		loopinfo_cf_consistent(((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_cf.val) | irg_loopinfo_state.loopinfo_valid.val)),
		loopinfo_cf_inconsistent((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_cf.val)),
		loopinfo_cf_ip_consistent((((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_cf.val) | irg_loopinfo_state.loopinfo_inter.val) | irg_loopinfo_state.loopinfo_valid.val)),
		loopinfo_cf_ip_inconsistent(((irg_loopinfo_state.loopinfo_constructed.val | irg_loopinfo_state.loopinfo_cf.val) | irg_loopinfo_state.loopinfo_inter.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_loopinfo_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_loopinfo_state() {
			this.val = C.next_val++;
		}

		public static irg_loopinfo_state getEnum(int val) {
			for (irg_loopinfo_state entry : values()) {
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

	public static enum irg_inline_property {
		irg_inline_any(),
		irg_inline_forbidden(),
		irg_inline_recomended(),
		irg_inline_forced(),
		irg_inline_forced_no_body();
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_inline_property(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_inline_property() {
			this.val = C.next_val++;
		}

		public static irg_inline_property getEnum(int val) {
			for (irg_inline_property entry : values()) {
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
		IR_RESOURCE_PHI_LIST((1 << 5)),
		IR_RESOURCE_IRG_LINK((1 << 6)),
		IR_RESOURCE_ENTITY_LINK((1 << 8)),
		IR_RESOURCE_TYPE_VISITED((1 << 9)),
		IR_RESOURCE_LOCAL_MASK(0x00FF),
		IR_RESOURCE_GLOBAL_MASK(0xFF00);
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

	public static enum ir_graph_state_t {
		IR_GRAPH_STATE_KEEP_MUX((1 << 0)),
		IR_GRAPH_STATE_ARCH_DEP((1 << 1)),
		IR_GRAPH_STATE_BCONV_ALLOWED((1 << 2));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_graph_state_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_graph_state_t() {
			this.val = C.next_val++;
		}

		public static ir_graph_state_t getEnum(int val) {
			for (ir_graph_state_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_visibility {
		ir_visibility_default(),
		ir_visibility_local(),
		ir_visibility_external(),
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
		IR_LINKAGE_HIDDEN_USER((1 << 4));
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
		align_non_aligned(),
		align_is_aligned();
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
		tpo_class(),
		tpo_struct(),
		tpo_method(),
		tpo_union(),
		tpo_array(),
		tpo_enumeration(),
		tpo_pointer(),
		tpo_primitive(),
		tpo_code(),
		tpo_none(),
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

	public static enum ir_class_cast_state {
		ir_class_casts_any(0),
		ir_class_casts_transitive(1),
		ir_class_casts_normalized(2),
		ir_class_casts_state_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_class_cast_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_class_cast_state() {
			this.val = C.next_val++;
		}

		public static ir_class_cast_state getEnum(int val) {
			for (ir_class_cast_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum trverify_error_codes {
		no_error(0),
		error_ent_not_cont(),
		error_null_mem(),
		error_const_on_wrong_irg(),
		error_existent_entity_without_irg(),
		error_wrong_ent_overwrites(),
		error_inherited_ent_without_const(),
		error_glob_ent_allocation(),
		error_ent_const_mode(),
		error_ent_wrong_owner();
		public final int val;

		private static class C {
			static int next_val;
		}

		trverify_error_codes(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		trverify_error_codes() {
			this.val = C.next_val++;
		}

		public static trverify_error_codes getEnum(int val) {
			for (trverify_error_codes entry : values()) {
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

	public static enum ir_variadicity {
		variadicity_non_variadic(),
		variadicity_variadic();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_variadicity(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_variadicity() {
			this.val = C.next_val++;
		}

		public static ir_variadicity getEnum(int val) {
			for (ir_variadicity entry : values()) {
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
		cc_fpreg_param(0x40000000),
		cc_bits((0xFF << 24));
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

	public static enum ir_allocation {
		allocation_automatic(),
		allocation_parameter(),
		allocation_dynamic(),
		allocation_static();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_allocation(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_allocation() {
			this.val = C.next_val++;
		}

		public static ir_allocation getEnum(int val) {
			for (ir_allocation entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_peculiarity {
		peculiarity_existent(),
		peculiarity_description(),
		peculiarity_inherited();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_peculiarity(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_peculiarity() {
			this.val = C.next_val++;
		}

		public static ir_peculiarity getEnum(int val) {
			for (ir_peculiarity entry : values()) {
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

	public static enum insn_kind {
		LEA(),
		SHIFT(),
		SUB(),
		ADD(),
		ZERO(),
		MUL(),
		ROOT();
		public final int val;

		private static class C {
			static int next_val;
		}

		insn_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		insn_kind() {
			this.val = C.next_val++;
		}

		public static insn_kind getEnum(int val) {
			for (insn_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum arch_dep_opts_t {
		arch_dep_none(0),
		arch_dep_mul_to_shift(1),
		arch_dep_div_by_const(2),
		arch_dep_mod_by_const(4);
		public final int val;

		private static class C {
			static int next_val;
		}

		arch_dep_opts_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		arch_dep_opts_t() {
			this.val = C.next_val++;
		}

		public static arch_dep_opts_t getEnum(int val) {
			for (arch_dep_opts_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum add_hidden {
		ADD_HIDDEN_ALWAYS_IN_FRONT(0),
		ADD_HIDDEN_ALWAYS_LAST(1),
		ADD_HIDDEN_SMART(2);
		public final int val;

		private static class C {
			static int next_val;
		}

		add_hidden(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		add_hidden() {
			this.val = C.next_val++;
		}

		public static add_hidden getEnum(int val) {
			for (add_hidden entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum lowering_flags {
		LF_NONE(0),
		LF_COMPOUND_PARAM(1),
		LF_COMPOUND_RETURN(2),
		LF_RETURN_HIDDEN(4),
		LF_SMALL_CMP_IN_REGS(8);
		public final int val;

		private static class C {
			static int next_val;
		}

		lowering_flags(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		lowering_flags() {
			this.val = C.next_val++;
		}

		public static lowering_flags getEnum(int val) {
			for (lowering_flags entry : values()) {
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

	public static enum pn_Alloc {
		pn_Alloc_M(pn_generic.pn_Generic_M.val),
		pn_Alloc_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Alloc_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Alloc_res(pn_generic.pn_Generic_other.val),
		pn_Alloc_max();
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

	public static enum pn_Bound {
		pn_Bound_M(pn_generic.pn_Generic_M.val),
		pn_Bound_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Bound_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Bound_res(pn_generic.pn_Generic_other.val),
		pn_Bound_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Bound(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Bound() {
			this.val = C.next_val++;
		}

		public static pn_Bound getEnum(int val) {
			for (pn_Bound entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Builtin {
		pn_Builtin_M(pn_generic.pn_Generic_M.val),
		pn_Builtin_1_result(pn_generic.pn_Generic_other.val),
		pn_Builtin_max();
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

	public static enum pn_Call {
		pn_Call_M(pn_generic.pn_Generic_M.val),
		pn_Call_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Call_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Call_T_result(pn_generic.pn_Generic_other.val),
		pn_Call_P_value_res_base(),
		pn_Call_max();
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

	public static enum pn_Cmp {
		pn_Cmp_False(0),
		pn_Cmp_Eq(1),
		pn_Cmp_Lt(2),
		pn_Cmp_Le((pn_Cmp.pn_Cmp_Eq.val | pn_Cmp.pn_Cmp_Lt.val)),
		pn_Cmp_Gt(4),
		pn_Cmp_Ge((pn_Cmp.pn_Cmp_Eq.val | pn_Cmp.pn_Cmp_Gt.val)),
		pn_Cmp_Lg((pn_Cmp.pn_Cmp_Lt.val | pn_Cmp.pn_Cmp_Gt.val)),
		pn_Cmp_Leg(((pn_Cmp.pn_Cmp_Lt.val | pn_Cmp.pn_Cmp_Eq.val) | pn_Cmp.pn_Cmp_Gt.val)),
		pn_Cmp_Uo(8),
		pn_Cmp_Ue((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Eq.val)),
		pn_Cmp_Ul((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Lt.val)),
		pn_Cmp_Ule(((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Lt.val) | pn_Cmp.pn_Cmp_Eq.val)),
		pn_Cmp_Ug((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Gt.val)),
		pn_Cmp_Uge(((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Gt.val) | pn_Cmp.pn_Cmp_Eq.val)),
		pn_Cmp_Ne(((pn_Cmp.pn_Cmp_Uo.val | pn_Cmp.pn_Cmp_Lt.val) | pn_Cmp.pn_Cmp_Gt.val)),
		pn_Cmp_True(15),
		pn_Cmp_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Cmp(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Cmp() {
			this.val = C.next_val++;
		}

		public static pn_Cmp getEnum(int val) {
			for (pn_Cmp entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Cond {
		pn_Cond_false(),
		pn_Cond_true(),
		pn_Cond_max();
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

	public static enum pn_CopyB {
		pn_CopyB_M(pn_generic.pn_Generic_M.val),
		pn_CopyB_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_CopyB_X_except(pn_generic.pn_Generic_X_except.val),
		pn_CopyB_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_CopyB(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_CopyB() {
			this.val = C.next_val++;
		}

		public static pn_CopyB getEnum(int val) {
			for (pn_CopyB entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Div {
		pn_Div_M(pn_generic.pn_Generic_M.val),
		pn_Div_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Div_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Div_res(pn_generic.pn_Generic_other.val),
		pn_Div_max();
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

	public static enum pn_DivMod {
		pn_DivMod_M(pn_generic.pn_Generic_M.val),
		pn_DivMod_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_DivMod_X_except(pn_generic.pn_Generic_X_except.val),
		pn_DivMod_res_div(pn_generic.pn_Generic_other.val),
		pn_DivMod_res_mod(),
		pn_DivMod_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_DivMod(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_DivMod() {
			this.val = C.next_val++;
		}

		public static pn_DivMod getEnum(int val) {
			for (pn_DivMod entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_InstOf {
		pn_InstOf_M(pn_generic.pn_Generic_M.val),
		pn_InstOf_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_InstOf_X_except(pn_generic.pn_Generic_X_except.val),
		pn_InstOf_res(pn_generic.pn_Generic_other.val),
		pn_InstOf_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_InstOf(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_InstOf() {
			this.val = C.next_val++;
		}

		public static pn_InstOf getEnum(int val) {
			for (pn_InstOf entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Load {
		pn_Load_M(pn_generic.pn_Generic_M.val),
		pn_Load_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Load_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Load_res(pn_generic.pn_Generic_other.val),
		pn_Load_max();
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

	public static enum pn_Mod {
		pn_Mod_M(pn_generic.pn_Generic_M.val),
		pn_Mod_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Mod_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Mod_res(pn_generic.pn_Generic_other.val),
		pn_Mod_max();
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

	public static enum pn_Quot {
		pn_Quot_M(pn_generic.pn_Generic_M.val),
		pn_Quot_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Quot_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Quot_res(pn_generic.pn_Generic_other.val),
		pn_Quot_max();
		public final int val;

		private static class C {
			static int next_val;
		}

		pn_Quot(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		pn_Quot() {
			this.val = C.next_val++;
		}

		public static pn_Quot getEnum(int val) {
			for (pn_Quot entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum pn_Raise {
		pn_Raise_M(pn_generic.pn_Generic_M.val),
		pn_Raise_X(pn_generic.pn_Generic_X_regular.val),
		pn_Raise_max();
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

	public static enum pn_Start {
		pn_Start_X_initial_exec(),
		pn_Start_M(),
		pn_Start_P_frame_base(),
		pn_Start_P_tls(),
		pn_Start_T_args(),
		pn_Start_max();
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

	public static enum pn_Store {
		pn_Store_M(pn_generic.pn_Generic_M.val),
		pn_Store_X_regular(pn_generic.pn_Generic_X_regular.val),
		pn_Store_X_except(pn_generic.pn_Generic_X_except.val),
		pn_Store_max();
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

	public static enum asm_constraint_flags_t {
		ASM_CONSTRAINT_FLAG_NONE(0),
		ASM_CONSTRAINT_FLAG_SUPPORTS_REGISTER((1 << 0)),
		ASM_CONSTRAINT_FLAG_SUPPORTS_MEMOP((1 << 1)),
		ASM_CONSTRAINT_FLAG_SUPPORTS_IMMEDIATE((1 << 2)),
		ASM_CONSTRAINT_FLAG_NO_SUPPORT((1 << 3)),
		ASM_CONSTRAINT_FLAG_MODIFIER_WRITE((1 << 4)),
		ASM_CONSTRAINT_FLAG_MODIFIER_NO_WRITE((1 << 5)),
		ASM_CONSTRAINT_FLAG_MODIFIER_READ((1 << 6)),
		ASM_CONSTRAINT_FLAG_MODIFIER_NO_READ((1 << 7)),
		ASM_CONSTRAINT_FLAG_MODIFIER_EARLYCLOBBER((1 << 8)),
		ASM_CONSTRAINT_FLAG_MODIFIER_COMMUTATIVE((1 << 9)),
		ASM_CONSTRAINT_FLAG_INVALID((1 << 10));
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
		dbg_rem_poly_call(),
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

	public static enum exec_freq_state {
		exec_freq_none(),
		exec_freq_consistent(),
		exec_freq_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		exec_freq_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		exec_freq_state() {
			this.val = C.next_val++;
		}

		public static exec_freq_state getEnum(int val) {
			for (exec_freq_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irp_temperature_state {
		temperature_none(),
		temperature_consistent(),
		temperature_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		irp_temperature_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irp_temperature_state() {
			this.val = C.next_val++;
		}

		public static irp_temperature_state getEnum(int val) {
			for (irp_temperature_state entry : values()) {
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
		k_ir_op(),
		k_tarval(),
		k_ir_loop(),
		k_ir_compound_graph_path(),
		k_ir_extblk(),
		k_ir_prog(),
		k_ir_region(),
		k_ir_graph_pass(),
		k_ir_prog_pass(),
		k_ir_graph_pass_mgr(),
		k_ir_prog_pass_mgr(),
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

	public static enum ir_opcode {
		iro_ASM(),
		iro_Add(),
		iro_Alloc(),
		iro_Anchor(),
		iro_And(),
		iro_Bad(),
		iro_Block(),
		iro_Borrow(),
		iro_Bound(),
		iro_Builtin(),
		iro_Call(),
		iro_Carry(),
		iro_Cast(),
		iro_Cmp(),
		iro_Cond(),
		iro_Confirm(),
		iro_Const(),
		iro_Conv(),
		iro_CopyB(),
		iro_Deleted(),
		iro_Div(),
		iro_DivMod(),
		iro_Dummy(),
		iro_End(),
		iro_Eor(),
		iro_Free(),
		iro_IJmp(),
		iro_Id(),
		iro_InstOf(),
		iro_Jmp(),
		iro_Load(),
		iro_Minus(),
		iro_Mod(),
		iro_Mul(),
		iro_Mulh(),
		iro_Mux(),
		iro_NoMem(),
		iro_Not(),
		iro_Or(),
		iro_Phi(),
		iro_Pin(),
		iro_Proj(),
		iro_Quot(),
		iro_Raise(),
		iro_Return(),
		iro_Rotl(),
		iro_Sel(),
		iro_Shl(),
		iro_Shr(),
		iro_Shrs(),
		iro_Start(),
		iro_Store(),
		iro_Sub(),
		iro_SymConst(),
		iro_Sync(),
		iro_Tuple(),
		iro_Unknown(),
		iro_First(ir_opcode.iro_ASM.val),
		iro_Last(ir_opcode.iro_Unknown.val),
		beo_First(),
		beo_Spill(ir_opcode.beo_First.val),
		beo_Reload(),
		beo_Perm(),
		beo_MemPerm(),
		beo_Copy(),
		beo_Keep(),
		beo_CopyKeep(),
		beo_Call(),
		beo_Return(),
		beo_AddSP(),
		beo_SubSP(),
		beo_IncSP(),
		beo_Start(),
		beo_FrameAddr(),
		beo_Barrier(),
		beo_Last(ir_opcode.beo_Barrier.val),
		iro_MaxOpcode();
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

	public static enum op_arity {
		oparity_invalid(0),
		oparity_unary(),
		oparity_binary(),
		oparity_trinary(),
		oparity_zero(),
		oparity_variable(),
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
		irop_flag_labeled((1 << 0)),
		irop_flag_commutative((1 << 1)),
		irop_flag_cfopcode((1 << 2)),
		irop_flag_fragile((1 << 3)),
		irop_flag_forking((1 << 4)),
		irop_flag_highlevel((1 << 5)),
		irop_flag_constlike((1 << 6)),
		irop_flag_always_opt((1 << 7)),
		irop_flag_keep((1 << 8)),
		irop_flag_start_block((1 << 9)),
		irop_flag_uses_memory((1 << 10)),
		irop_flag_dump_noblock((1 << 11)),
		irop_flag_dump_noinput((1 << 12)),
		irop_flag_machine((1 << 13)),
		irop_flag_machine_op((1 << 14)),
		irop_flag_cse_neutral((1 << 15)),
		irop_flag_user((1 << 16));
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

	public static enum ir_mode_sort_helper {
		irmsh_is_num(0x10),
		irmsh_is_data(0x20),
		irmsh_is_datab(0x40),
		irmsh_is_dataM(0x80);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_mode_sort_helper(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_mode_sort_helper() {
			this.val = C.next_val++;
		}

		public static ir_mode_sort_helper getEnum(int val) {
			for (ir_mode_sort_helper entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_mode_sort {
		irms_auxiliary(0),
		irms_control_flow(1),
		irms_memory((2 | ir_mode_sort_helper.irmsh_is_dataM.val)),
		irms_internal_boolean((3 | ir_mode_sort_helper.irmsh_is_datab.val)),
		irms_reference((((4 | ir_mode_sort_helper.irmsh_is_data.val) | ir_mode_sort_helper.irmsh_is_datab.val) | ir_mode_sort_helper.irmsh_is_dataM.val)),
		irms_int_number(((((5 | ir_mode_sort_helper.irmsh_is_data.val) | ir_mode_sort_helper.irmsh_is_datab.val) | ir_mode_sort_helper.irmsh_is_dataM.val) | ir_mode_sort_helper.irmsh_is_num.val)),
		irms_float_number(((((6 | ir_mode_sort_helper.irmsh_is_data.val) | ir_mode_sort_helper.irmsh_is_datab.val) | ir_mode_sort_helper.irmsh_is_dataM.val) | ir_mode_sort_helper.irmsh_is_num.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_mode_sort(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_mode_sort() {
			this.val = C.next_val++;
		}

		public static ir_mode_sort getEnum(int val) {
			for (ir_mode_sort entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_mode_arithmetic {
		irma_uninitialized(0),
		irma_none(1),
		irma_twos_complement(2),
		irma_ones_complement(),
		irma_int_BCD(),
		irma_ieee754(256),
		irma_float_BCD(),
		irma_max();
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

	public static enum hook_opt_kind {
		HOOK_OPT_DEAD_BLOCK(),
		HOOK_OPT_STG(),
		HOOK_OPT_IFSIM(),
		HOOK_OPT_CONST_EVAL(),
		HOOK_OPT_ALGSIM(),
		HOOK_OPT_PHI(),
		HOOK_OPT_SYNC(),
		HOOK_OPT_WAW(),
		HOOK_OPT_WAR(),
		HOOK_OPT_RAW(),
		HOOK_OPT_RAR(),
		HOOK_OPT_RC(),
		HOOK_OPT_TUPLE(),
		HOOK_OPT_ID(),
		HOOK_OPT_CSE(),
		HOOK_OPT_STRENGTH_RED(),
		HOOK_OPT_ARCH_DEP(),
		HOOK_OPT_REASSOC(),
		HOOK_OPT_POLY_CALL(),
		HOOK_OPT_IF_CONV(),
		HOOK_OPT_FUNC_CALL(),
		HOOK_OPT_CONFIRM(),
		HOOK_OPT_CONFIRM_C(),
		HOOK_OPT_CONFIRM_E(),
		HOOK_OPT_EXC_REM(),
		HOOK_OPT_NORMALIZE(),
		HOOK_LOWERED(),
		HOOK_BACKEND(),
		HOOK_OPT_LAST();
		public final int val;

		private static class C {
			static int next_val;
		}

		hook_opt_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		hook_opt_kind() {
			this.val = C.next_val++;
		}

		public static hook_opt_kind getEnum(int val) {
			for (hook_opt_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum if_result_t {
		IF_RESULT_SUCCESS(0),
		IF_RESULT_SIDE_EFFECT(1),
		IF_RESULT_SIDE_EFFECT_PHI(2),
		IF_RESULT_TOO_DEEP(3),
		IF_RESULT_BAD_CF(4),
		IF_RESULT_DENIED(5),
		IF_RESULT_LAST();
		public final int val;

		private static class C {
			static int next_val;
		}

		if_result_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		if_result_t() {
			this.val = C.next_val++;
		}

		public static if_result_t getEnum(int val) {
			for (if_result_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum hook_type_t {
		hook_new_ir_op(),
		hook_free_ir_op(),
		hook_new_node(),
		hook_set_irn_n(),
		hook_replace(),
		hook_turn_into_id(),
		hook_normalize(),
		hook_new_graph(),
		hook_free_graph(),
		hook_irg_walk(),
		hook_irg_walk_blkwise(),
		hook_irg_block_walk(),
		hook_merge_nodes(),
		hook_reassociate(),
		hook_lower(),
		hook_inline(),
		hook_tail_rec(),
		hook_strength_red(),
		hook_dead_node_elim(),
		hook_dead_node_elim_subst(),
		hook_if_conversion(),
		hook_func_call(),
		hook_arch_dep_replace_mul_with_shifts(),
		hook_arch_dep_replace_division_by_const(),
		hook_new_mode(),
		hook_new_entity(),
		hook_new_type(),
		hook_node_info(),
		hook_last();
		public final int val;

		private static class C {
			static int next_val;
		}

		hook_type_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		hook_type_t() {
			this.val = C.next_val++;
		}

		public static hook_type_t getEnum(int val) {
			for (hook_type_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum firmstat_options_t {
		FIRMSTAT_ENABLED(0x00000001),
		FIRMSTAT_PATTERN_ENABLED(0x00000002),
		FIRMSTAT_COUNT_STRONG_OP(0x00000004),
		FIRMSTAT_COUNT_DAG(0x00000008),
		FIRMSTAT_COUNT_DELETED(0x00000010),
		FIRMSTAT_COUNT_SELS(0x00000020),
		FIRMSTAT_COUNT_CONSTS(0x00000040),
		FIRMSTAT_COUNT_EXTBB(0x00000080),
		FIRMSTAT_CSV_OUTPUT(0x10000000);
		public final int val;

		private static class C {
			static int next_val;
		}

		firmstat_options_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		firmstat_options_t() {
			this.val = C.next_val++;
		}

		public static firmstat_options_t getEnum(int val) {
			for (firmstat_options_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum firmstat_optimizations_t {
		FS_OPT_NEUTRAL_0(hook_opt_kind.HOOK_OPT_LAST.val),
		FS_OPT_NEUTRAL_1(),
		FS_OPT_ADD_A_A(),
		FS_OPT_ADD_A_MINUS_B(),
		FS_OPT_ADD_SUB(),
		FS_OPT_ADD_MUL_A_X_A(),
		FS_OPT_SUB_0_A(),
		FS_OPT_MINUS_SUB(),
		FS_OPT_SUB_MINUS(),
		FS_OPT_SUB_MUL_A_X_A(),
		FS_OPT_SUB_SUB_X_Y_Z(),
		FS_OPT_SUB_C_NOT_X(),
		FS_OPT_SUB_TO_ADD(),
		FS_OPT_SUB_TO_NOT(),
		FS_OPT_SUB_TO_CONV(),
		FS_OPT_MUL_MINUS(),
		FS_OPT_MUL_MINUS_1(),
		FS_OPT_MINUS_MUL_C(),
		FS_OPT_MUL_MINUS_MINUS(),
		FS_OPT_OR(),
		FS_OPT_AND(),
		FS_OPT_TO_EOR(),
		FS_OPT_EOR_A_A(),
		FS_OPT_EOR_A_B_A(),
		FS_OPT_EOR_TO_NOT_BOOL(),
		FS_OPT_EOR_TO_NOT(),
		FS_OPT_NOT_CMP(),
		FS_OPT_OR_SHFT_TO_ROTL(),
		FS_OPT_REASSOC_SHIFT(),
		FS_OPT_SHIFT_AND(),
		FS_OPT_SHIFT_OR(),
		FS_OPT_SHIFT_EOR(),
		FS_OPT_CONV(),
		FS_OPT_CAST(),
		FS_OPT_MIN_MAX_EQ(),
		FS_OPT_MUX_COMBINE(),
		FS_OPT_MUX_CONV(),
		FS_OPT_MUX_BOOL(),
		FS_OPT_MUX_NOT_BOOL(),
		FS_OPT_MUX_OR_BOOL(),
		FS_OPT_MUX_ORNOT_BOOL(),
		FS_OPT_MUX_AND_BOOL(),
		FS_OPT_MUX_ANDNOT_BOOL(),
		FS_OPT_MUX_C(),
		FS_OPT_MUX_EQ(),
		FS_OPT_MUX_TRANSFORM(),
		FS_OPT_MUX_TO_MIN(),
		FS_OPT_MUX_TO_MAX(),
		FS_OPT_MUX_TO_BITOP(),
		FS_OPT_IDEM_UNARY(),
		FS_OPT_MINUS_NOT(),
		FS_OPT_NOT_MINUS_1(),
		FS_OPT_NOT_PLUS_1(),
		FS_OPT_ADD_X_NOT_X(),
		FS_OPT_FP_INV_MUL(),
		FS_OPT_CONST_PHI(),
		FS_OPT_PREDICATE(),
		FS_OPT_DEMORGAN(),
		FS_OPT_CMP_OP_OP(),
		FS_OPT_CMP_OP_C(),
		FS_OPT_CMP_CONV_CONV(),
		FS_OPT_CMP_CONV(),
		FS_OPT_CMP_TO_BOOL(),
		FS_OPT_CMP_CNST_MAGN(),
		FS_OPT_CMP_SHF_TO_AND(),
		FS_OPT_CMP_MOD_TO_AND(),
		FS_OPT_NOP(),
		FS_OPT_GVN_FOLLOWER(),
		FS_OPT_GVN_FULLY(),
		FS_OPT_GVN_PARTLY(),
		FS_OPT_COMBO_CONST(),
		FS_OPT_COMBO_CF(),
		FS_OPT_COMBO_FOLLOWER(),
		FS_OPT_COMBO_CONGRUENT(),
		FS_OPT_JUMPTHREADING(),
		FS_OPT_RTS_ABS(),
		FS_OPT_RTS_ALLOCA(),
		FS_OPT_RTS_SQRT(),
		FS_OPT_RTS_CBRT(),
		FS_OPT_RTS_POW(),
		FS_OPT_RTS_EXP(),
		FS_OPT_RTS_LOG(),
		FS_OPT_RTS_SIN(),
		FS_OPT_RTS_COS(),
		FS_OPT_RTS_TAN(),
		FS_OPT_RTS_ASIN(),
		FS_OPT_RTS_ACOS(),
		FS_OPT_RTS_ATAN(),
		FS_OPT_RTS_SINH(),
		FS_OPT_RTS_COSH(),
		FS_OPT_RTS_TANH(),
		FS_OPT_RTS_SYMMETRIC(),
		FS_OPT_RTS_STRCMP(),
		FS_OPT_RTS_STRNCMP(),
		FS_OPT_RTS_STRCPY(),
		FS_OPT_RTS_STRLEN(),
		FS_OPT_RTS_MEMCPY(),
		FS_OPT_RTS_MEMPCPY(),
		FS_OPT_RTS_MEMMOVE(),
		FS_OPT_RTS_MEMSET(),
		FS_OPT_RTS_MEMCMP(),
		FS_BE_IA32_LEA(),
		FS_BE_IA32_LOAD_LEA(),
		FS_BE_IA32_STORE_LEA(),
		FS_BE_IA32_AM_S(),
		FS_BE_IA32_AM_D(),
		FS_BE_IA32_CJMP(),
		FS_BE_IA32_2ADDRCPY(),
		FS_BE_IA32_SPILL2ST(),
		FS_BE_IA32_RELOAD2LD(),
		FS_BE_IA32_SUB2NEGADD(),
		FS_BE_IA32_LEA2ADD(),
		FS_OPT_MAX();
		public final int val;

		private static class C {
			static int next_val;
		}

		firmstat_optimizations_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		firmstat_optimizations_t() {
			this.val = C.next_val++;
		}

		public static firmstat_optimizations_t getEnum(int val) {
			for (firmstat_optimizations_t entry : values()) {
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
		cons_floats((1 << 2));
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

	public static enum ir_dump_verbosity_t {
		dump_verbosity_onlynames(0x00000001),
		dump_verbosity_fields(0x00000002),
		dump_verbosity_methods(0x00000004),
		dump_verbosity_nostatic(0x00000040),
		dump_verbosity_typeattrs(0x00000008),
		dump_verbosity_entattrs(0x00000010),
		dump_verbosity_entconsts(0x00000020),
		dump_verbosity_accessStats(0x00000100),
		dump_verbosity_noClassTypes(0x00001000),
		dump_verbosity_noStructTypes(0x00002000),
		dump_verbosity_noUnionTypes(0x00004000),
		dump_verbosity_noArrayTypes(0x00008000),
		dump_verbosity_noPointerTypes(0x00010000),
		dump_verbosity_noMethodTypes(0x00020000),
		dump_verbosity_noPrimitiveTypes(0x00040000),
		dump_verbosity_noEnumerationTypes(0x00080000),
		dump_verbosity_onlyClassTypes(0x000FE000),
		dump_verbosity_onlyStructTypes(0x000FD000),
		dump_verbosity_onlyUnionTypes(0x000FB000),
		dump_verbosity_onlyArrayTypes(0x000F7000),
		dump_verbosity_onlyPointerTypes(0x000EF000),
		dump_verbosity_onlyMethodTypes(0x000DF000),
		dump_verbosity_onlyPrimitiveTypes(0x000BF000),
		dump_verbosity_onlyEnumerationTypes(0x0007F000),
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
		ir_dump_flag_group_extbb((1 << 1)),
		ir_dump_flag_with_typegraph((1 << 2)),
		ir_dump_flag_disable_edge_labels((1 << 3)),
		ir_dump_flag_consts_local((1 << 4)),
		ir_dump_flag_idx_label((1 << 5)),
		ir_dump_flag_number_label((1 << 6)),
		ir_dump_flag_keepalive_edges((1 << 7)),
		ir_dump_flag_out_edges((1 << 8)),
		ir_dump_flag_dominance((1 << 9)),
		ir_dump_flag_loops((1 << 10)),
		ir_dump_flag_back_edges((1 << 11)),
		ir_dump_flag_analysed_types((1 << 12)),
		ir_dump_flag_iredges((1 << 13)),
		ir_dump_flag_node_addresses((1 << 14)),
		ir_dump_flag_all_anchors((1 << 15)),
		ir_dump_flag_show_marks((1 << 16)),
		ir_dump_flag_no_entity_values((1 << 20)),
		ir_dump_flag_ld_names((1 << 21)),
		ir_dump_flag_entities_in_hierarchy((1 << 22));
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
		EDGE_KIND_FIRST(),
		EDGE_KIND_NORMAL(ir_edge_kind_t.EDGE_KIND_FIRST.val),
		EDGE_KIND_BLOCK(),
		EDGE_KIND_DEP(),
		EDGE_KIND_LAST();
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

	public static enum firm_verification_t {
		FIRM_VERIFICATION_OFF(0),
		FIRM_VERIFICATION_ON(1),
		FIRM_VERIFICATION_REPORT(2),
		FIRM_VERIFICATION_ERROR_ONLY(3);
		public final int val;

		private static class C {
			static int next_val;
		}

		firm_verification_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		firm_verification_t() {
			this.val = C.next_val++;
		}

		public static firm_verification_t getEnum(int val) {
			for (firm_verification_t entry : values()) {
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

	public static enum ir_disambuigator_options {
		aa_opt_no_opt(0),
		aa_opt_type_based(1),
		aa_opt_byte_type_may_alias(2),
		aa_opt_no_alias_args(4),
		aa_opt_no_alias_args_global(8),
		aa_opt_no_alias(16),
		aa_opt_inherited(128);
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_disambuigator_options(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_disambuigator_options() {
			this.val = C.next_val++;
		}

		public static ir_disambuigator_options getEnum(int val) {
			for (ir_disambuigator_options entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_storage_class_class_t {
		ir_sc_pointer(0x0),
		ir_sc_globalvar(0x1),
		ir_sc_localvar(0x2),
		ir_sc_tls(0x3),
		ir_sc_malloced(0x4),
		ir_sc_globaladdr(0x5),
		ir_sc_modifier_nottaken(0x80),
		ir_sc_modifier_argument(0x40),
		ir_sc_modifiers((ir_storage_class_class_t.ir_sc_modifier_nottaken.val | ir_storage_class_class_t.ir_sc_modifier_argument.val));
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_storage_class_class_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_storage_class_class_t() {
			this.val = C.next_val++;
		}

		public static ir_storage_class_class_t getEnum(int val) {
			for (ir_storage_class_class_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum fp_model_t {
		fp_explicit_rounding((1 << 0)),
		fp_strict_algebraic((1 << 1)),
		fp_contradictions((1 << 2)),
		fp_strict_eval_order((1 << 3)),
		fp_exceptions((1 << 4)),
		fp_environment_access((1 << 5)),
		fp_model_precise(((fp_model_t.fp_explicit_rounding.val | fp_model_t.fp_strict_algebraic.val) | fp_model_t.fp_contradictions.val)),
		fp_model_strict(((((fp_model_t.fp_explicit_rounding.val | fp_model_t.fp_strict_algebraic.val) | fp_model_t.fp_strict_eval_order.val) | fp_model_t.fp_exceptions.val) | fp_model_t.fp_environment_access.val)),
		fp_model_fast(fp_model_t.fp_contradictions.val);
		public final int val;

		private static class C {
			static int next_val;
		}

		fp_model_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		fp_model_t() {
			this.val = C.next_val++;
		}

		public static fp_model_t getEnum(int val) {
			for (fp_model_t entry : values()) {
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
		IR_SEGMENT_LAST(ir_segment_t.IR_SEGMENT_DESTRUCTORS.val);
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

	public static enum ir_typeinfo_state {
		ir_typeinfo_none(),
		ir_typeinfo_consistent(),
		ir_typeinfo_inconsistent();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_typeinfo_state(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_typeinfo_state() {
			this.val = C.next_val++;
		}

		public static ir_typeinfo_state getEnum(int val) {
			for (ir_typeinfo_state entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum irg_verify_flags_t {
		VERIFY_NORMAL(0),
		VERIFY_ENFORCE_SSA(1);
		public final int val;

		private static class C {
			static int next_val;
		}

		irg_verify_flags_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		irg_verify_flags_t() {
			this.val = C.next_val++;
		}

		public static irg_verify_flags_t getEnum(int val) {
			for (irg_verify_flags_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum verify_bad_flags_t {
		BAD_CF(1),
		BAD_DF(2),
		BAD_BLOCK(4),
		TUPLE(8);
		public final int val;

		private static class C {
			static int next_val;
		}

		verify_bad_flags_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		verify_bad_flags_t() {
			this.val = C.next_val++;
		}

		public static verify_bad_flags_t getEnum(int val) {
			for (verify_bad_flags_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum ir_region_kind {
		ir_rk_Unknown(),
		ir_rk_BasicBlock(),
		ir_rk_Sequence(),
		ir_rk_IfThen(),
		ir_rk_IfThenElse(),
		ir_rk_Case(),
		ir_rk_Switch(),
		ir_rk_Proper(),
		ir_rk_TryCatch(),
		ir_rk_TryCatchFinally(),
		ir_rk_SelfLoop(),
		ir_rk_RepeatLoop(),
		ir_rk_WhileLoop(),
		ir_rk_NaturalLoop(),
		ir_rk_Improper();
		public final int val;

		private static class C {
			static int next_val;
		}

		ir_region_kind(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		ir_region_kind() {
			this.val = C.next_val++;
		}

		public static ir_region_kind getEnum(int val) {
			for (ir_region_kind entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum tarval_int_overflow_mode_t {
		TV_OVERFLOW_BAD(),
		TV_OVERFLOW_WRAP(),
		TV_OVERFLOW_SATURATE();
		public final int val;

		private static class C {
			static int next_val;
		}

		tarval_int_overflow_mode_t(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		tarval_int_overflow_mode_t() {
			this.val = C.next_val++;
		}

		public static tarval_int_overflow_mode_t getEnum(int val) {
			for (tarval_int_overflow_mode_t entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}

	public static enum tv_output_mode {
		TVO_NATIVE(),
		TVO_HEX(),
		TVO_DECIMAL(),
		TVO_OCTAL(),
		TVO_BINARY(),
		TVO_FLOAT(),
		TVO_HEXFLOAT();
		public final int val;

		private static class C {
			static int next_val;
		}

		tv_output_mode(int val) {
			this.val = val;
			C.next_val = val + 1;
		}

		tv_output_mode() {
			this.val = C.next_val++;
		}

		public static tv_output_mode getEnum(int val) {
			for (tv_output_mode entry : values()) {
				if (val == entry.val)
					return entry;
			}
			return null;
		}
	}


	public static native void mangle_ct_init(Pointer ct);

	public static native void mangle_ct_flush(Pointer ct);

	public static native int mangle_ct_find(Pointer ct, String prefix);

	public static native void mangle_ct_insert(Pointer ct, String prefix);

	public static native void mangle_emit_substitution(int match, Pointer obst);

	public static native void mangle_pointer_type(Pointer type, Pointer obst, Pointer ct);

	public static native void mangle_primitive_type(Pointer type, Pointer obst);

	public static native boolean mangle_qualified_class_name(Pointer type, Pointer obst, Pointer ct);

	public static native void mangle_type(Pointer type, Pointer obst, Pointer ct);

	public static native void mangle_type_for_compression_table(Pointer type, Pointer obst);

	public static native Pointer mangle_entity_name(Pointer entity);

	public static native Pointer mangle_vtable_name(Pointer clazz);

	public static native void mangle_init();

	public static native void mangle_set_primitive_type_name(Pointer type, String name);

	public static native void mangle_add_name_substitution(String name, String mangled);

	public static native void mangle_deinit();
}
