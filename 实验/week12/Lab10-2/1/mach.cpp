/* mach.cpp -- execution machine
 */

#include "flincl.h"
//#include "classes.h"
#include "shortstack.h"
#include "semantics.h"
#include "compiler.h"

FVal s_true, s_false;
FVal s_symbol, s_long, s_double, s_string;
FVal s_array, s_dict, s_object, s_class;
FVal s_file;
FVal s_init;
Symbol * s_global;
Symbol * s_readline;
Symbol * s_token;
Symbol * s_close;
Symbol * s_write;
Symbol * s_readvalue;
Symbol * s_read;
Symbol * s_keys;
Symbol * s_values;
Symbol * s_clear;
Symbol * s_remove;
Symbol * s_copy;
Symbol * s_has_key;
Symbol * s_get;
Symbol * s_get2;
Symbol * s_chr;
Symbol * s_ord;
Symbol * s_intern;
Symbol * s_hex;
Symbol * s_oct;
Symbol * s_id;
Symbol * s_inst;
Symbol * s_subcl;



typedef void (Mach::*Primitive)(void);

Primitive instr_decode[4096] = {
/* 0 */        Mach::oper_noop,
/* 1 */        Mach::oper_newobj,
/* 2 */        Mach::oper_newarray,
/* 3 */        Mach::oper_aref,
/* 4 */        Mach::oper_dup,
/* 5 */        Mach::oper_unappend,
/* 6 */        Mach::oper_print,
/* 7 */        Mach::oper_append,
/* 8 */        Mach::oper_reverse,
/* 9 */        Mach::oper_last,
/* 10 */       Mach::oper_loopinc,
/* 11 */       Mach::oper_looptst,
/* 12 */       Mach::oper_member,
/* 13 */       Mach::oper_loopnxt,
/* 14 */       Mach::oper_notequal,
/* 15 */       Mach::oper_length,
/* 16 */       Mach::oper_cndand,
/* 17 */       Mach::oper_cndor,
/* 18 */       Mach::oper_isatom,
/* 19 */       Mach::oper_issymbol,
/* 20 */       Mach::oper_isnumber,
/* 21 */       Mach::oper_isinteger,
/* 22 */       Mach::oper_isfloat,
/* 23 */       Mach::oper_isstring,
/* 24 */       Mach::oper_isarray,
/* 25 */       Mach::oper_isobject,
/* 26 */       Mach::oper_isnull,
/* 27 */       Mach::oper_newdict,
/* 28 */       Mach::oper_isnegative,
/* 29 */       Mach::oper_iszero,
/* 30 */       Mach::oper_ispositive,
/* 31 */       Mach::oper_iseven,
/* 32 */       Mach::oper_isodd,
/* 33 */       Mach::oper_iseql,
/* 34 */       Mach::oper_isequal,
/* 35 */       Mach::oper_jump,
/* 36 */       Mach::oper_jumpifnot,
/* 37 */       Mach::oper_truncate,
/* 38 */       Mach::oper_float,
/* 39 */       Mach::oper_round,
/* 40 */       Mach::oper_add,
/* 41 */       Mach::oper_sub,
/* 42 */       Mach::oper_mul,
/* 43 */       Mach::oper_div,
/* 44 */       Mach::oper_inc,
/* 45 */       Mach::oper_dec,
/* 46 */       Mach::oper_rem,
/* 47 */       Mach::oper_min,
/* 48 */       Mach::oper_max,
/* 49 */       Mach::oper_abs,
/* 50 */       Mach::oper_sin,
/* 51 */       Mach::oper_cos,
/* 52 */       Mach::oper_tan,
/* 53 */       Mach::oper_exp,
/* 54 */       Mach::oper_sqrt,
/* 55 */       Mach::oper_random,
/* 56 */       Mach::oper_logand,
/* 57 */       Mach::oper_logior,
/* 58 */       Mach::oper_logxor,
/* 59 */       Mach::oper_lognot,
/* 60 */       Mach::oper_less,
/* 61 */       Mach::oper_lesseql,
/* 62 */       Mach::oper_noteql,
/* 63 */       Mach::oper_gtreql,
/* 64 */       Mach::oper_gtr,
/* 65 */       Mach::oper_strcat,
/* 66 */       Mach::oper_subseq,
/* 67 */       Mach::oper_keys,
/* 68 */       Mach::oper_power,
/* 69 */       Mach::oper_type,
/* 70 */       Mach::oper_is,
/* 71 */       Mach::oper_isnot,
/* 72 */       Mach::oper_send,
/* 73 */       Mach::oper_super,
/* 74 */       Mach::oper_noop,
/* 75 */       Mach::oper_noop,
/* 76 */       Mach::oper_isupper,
/* 77 */       Mach::oper_islower,
/* 78 */       Mach::oper_isalpha,
/* 79 */       Mach::oper_isdigit,
/* 80 */       Mach::oper_isalnum,
/* 81 */       Mach::oper_toupper,
/* 82 */       Mach::oper_tolower,
/* 83 */       Mach::oper_pop2,
/* 84 */       Mach::oper_pop,
/* 85 */       Mach::oper_param,
/* 86 */       Mach::oper_call,
/* 87 */       Mach::oper_return,
/* 88 */       Mach::oper_retnull,
/* 89 */       Mach::oper_setaref,
/* 90 */	   Mach::oper_xcall,
/* 91 */	   Mach::oper_endl,
/* 92 */       Mach::oper_setfield,
/* 93 */       Mach::oper_jumpif,
/* 94 */       Mach::oper_bool2int,
/* 95 */       Mach::oper_int2bool,
/* 96 */       Mach::oper_minus,
/* 97 */       Mach::oper_field,
/* 98 */       Mach::oper_readline,
/* 99 */       Mach::oper_repr,
/* 100 */      Mach::oper_flatten,
/* 101 */      Mach::oper_str,
/* 102 */      Mach::oper_token,
/* 103 */      Mach::oper_close,
/* 104 */      Mach::oper_write,
/* 105 */      Mach::oper_readval,
/* 106 */      Mach::oper_read,
/* 107 */      Mach::oper_notmember,
/* 108 */      Mach::oper_not,
/* 109 */      Mach::oper_rshift,
/* 110 */      Mach::oper_lshift,
/* 111 */      Mach::oper_index,
/* 112 */      Mach::oper_load,
/* 113 */      Mach::oper_sort,
/* 114 */      Mach::oper_values,
/* 115 */      Mach::oper_clear,
/* 116 */      Mach::oper_remove,
/* 117 */      Mach::oper_copy,
/* 118 */      Mach::oper_haskey,
/* 119 */      Mach::oper_get,
/* 120 */      Mach::oper_get2,
/* 121 */      Mach::oper_chr,
/* 122 */      Mach::oper_ord,
/* 123 */      Mach::oper_intern,
/* 124 */      Mach::oper_hex,
/* 125 */      Mach::oper_oct,
/* 126 */      Mach::oper_id,
/* 127 */      Mach::oper_inst,
/* 128 */      Mach::oper_subcl,
/* 129 */      Mach::oper_find,
/* 130 */      Mach::oper_keyparam,
/* 131 */      Mach::oper_space,
/* 132 */	   Mach::oper_exit,
/* 133 */	   Mach::oper_frame_previous,
/* 134 */	   Mach::oper_frame_variables,
/* 135 */	   Mach::oper_frame_pc,
/* 136 */	   Mach::oper_frame_method,
/* 137 */	   Mach::oper_frame_class,
/* 138 */	   Mach::oper_except_nest,
/* 139 */	   Mach::oper_get_frame,
/* 140 */	   Mach::oper_unread,
/* 141 */	   Mach::oper_value
};

Mach::Mach()
{
    tos = expr_stack;
    frame = NULL;
    call_stack = NULL;
    symbols = Dict::make(this);
    pc = NULL;
    running = true;
    error_flag = false;
	trace_flag = false;
	callin_method = NULL;
///	classes = Classes::make(this);
}


char *type_string[] = { "free  ", "symb  ", "long  ", "doub  ", "string", "array ", "data  ",
					    "dict  ", "frame ", "meth  ", "obj   ", "class ", "symbs ", "entry ",
						"code  ", "locs  ", "builtn", "file  " };

void Mach::execute()
{
	running = true;
	error_flag = false;
    while (running) {
        uint16 instr = *pc++;
        int arg = instr_arg(instr);
		if (trace_flag) {
			char text[64];
			// show the stack
			FUnion *s = tos - 1;
			while (s >= expr_stack) {
				if (s->node) {
				    trace(" %s", type_string[s->node->get_tag()]);
					if (s->node->get_tag() == TAG_LONG) {
						trace("%I64d", s->flong->int64_data);
					} else if (s->node->get_tag() == TAG_DOUBLE) {
                        trace("%g", s->fdouble->double_data);
                    }
				} else trace(" null  ");
				s--;
			}
			trace("\n");
			// disassemble the instruction
			Code_ptr c = frame->get_method()->get_code();
            int len;
            trace(c->disassemble(pc - c->initial_pc() - 1, 
                                 instr, *pc, text, &len, 
                                 frame->get_method()));
		}
        switch (instr_type(instr)) {
          case instr_op:
	        (this->*(instr_decode[arg]))();
            break;
          case instr_load_global:
            push(frame->get_method()->symbol(arg)->get_value().node);
            break;
          case instr_store_global:
            frame->get_method()->symbol(arg)->set_value(pop().node, this);
            break;
          case instr_load_instvar:
            push(frame->get_self()->instvar(arg).node);
            break;
          case instr_store_instvar:
            frame->get_self()->set_instvar(arg, pop().node, this);
            break;
          case instr_load_local:
            push(frame->local(arg).node);
            break;
          case instr_store_local:
            frame->set_local(arg, pop().node, this);
            break;
          case instr_load_int:
            push(FLong::make(arg_sign_extend(arg), this));
            break;
          case instr_load_lit:
            push(frame->get_method()->get_literals()->fastref(arg).node);
            break;
          case instr_load_atom:
            push(frame->get_method()->symbol(arg));
            break;
          case instr_local_frame:
			push(frame->get_self());
            // no break, fall through
		  case instr_frame: {
            oper_frame(frame->get_method()->symbol(arg));
            break;
			}
          case instr_load_field: {
            Object * obj = pop().object->type_check(this);
			if (!obj) break;
            FClass * fclass = obj->get_fclass();
 			Method * method = frame->get_method();
            FVal selector = method->symbol(arg);
			long var_index;
            //  now look up the slot:
            if (!fclass->get_symboltable()->lookup(selector, &method, 
												   &var_index) ||
				method) {
                undefined_field();
                break;
            }
            push(obj->instvar(var_index).node);
            break;
          }
          case instr_store_field: {
            Object * obj = pop().object->type_check(this);
			if (!obj) break;
            FClass * fclass = obj->get_fclass();
 			Method * method = frame->get_method();
            FVal selector = method->symbol(arg);
			long index;
            //  now look up the slot:
            if (!fclass->get_symboltable()->
				    lookup(selector, &method, &index) || method) {
                undefined_field();
                break;
            }
            obj->set_instvar(index, pop().node, this);
            break;
          }
          default:
            illegal_instruction();
            break;
        }
    }
}


FUnion Mach::node_alloc(Tag tag, size_t size)
{
	FUnion p;
	p.node = (Node_ptr) malloc(size);
    p.node->node_initialize(tag, Node::slots_from_size(size), this);
	p.node->clear();
	return p;
}


FUnion Mach::node_alloc_color(Tag tag, size_t size, Color color)
{
	FUnion p = node_alloc(tag, size);
	p.node->set_color(color);
    return p;
}


size_t Mach::bigger(size_t size)
{
	long big = size + (size >> 2); // try for 25% bigger, round up to mult of 8
	big = round_up_size(big);
	return big;
}


void Mach::initialize()
{
	exit_flag = false;
	exception_nesting = 0;
	for (int i = SMALL_INT_MIN; i < SMALL_INT_MAX; i++) {
		small_ints[i - SMALL_INT_MIN] = FLong::must_create(i, this);
	}
	for (i = 0; i < SHORT_STRINGS_LEN; i++) {
		char ss[2];
		ss[0] = i; 
		ss[1] = 0;
		// note that strlen(ss) is zero for i=0, otherwise 1
		short_strings[i] = FString::make_len(strlen(ss), this);
		short_strings[i]->set_string(ss);
	}
	s_false = NULL;
	s_true = Symbol::make("t", this);
	s_true->set_value(s_true, this);	// initialize value to itself
	s_symbol = Symbol::make("symbol", this);
	s_long = Symbol::make("long", this);
	s_double = Symbol::make("double", this);
	s_string = Symbol::make("string", this);
	s_array = Symbol::make("array", this);
	s_dict = Symbol::make("dict", this);
	s_object = Symbol::make("object", this);
	s_class = Symbol::make("class", this);
    s_file = Symbol::make("file", this);
	for (i = 0; i < TAG_COUNT; i++) tag_to_type[i] = NULL;
	tag_to_type[TAG_SYMBOL] = s_symbol;
	tag_to_type[TAG_LONG] = s_long;
	tag_to_type[TAG_DOUBLE] = s_double;
	tag_to_type[TAG_STRING] = s_string;
	tag_to_type[TAG_ARRAY] = s_array;
	tag_to_type[TAG_DICT] = s_dict;
	tag_to_type[TAG_OBJECT] = s_object;
	tag_to_type[TAG_CLASS] = s_class;
	tag_to_type[TAG_FILE] = s_file;

	s_init = Symbol::make("init", this);
	s_global = Symbol::make("global", this);
/*
    s_readline = Symbol::make("readline", this);
    s_close = Symbol::make("close", this);
    s_token = Symbol::make("token", this);
    s_write = Symbol::make("write", this);
    s_readvalue = Symbol::make("readvalue", this);
    s_read = Symbol::make("read", this);
    s_keys = Symbol::make("keys", this);
    s_values = Symbol::make("values", this);
    s_values = Symbol::make("clear", this);
    s_values = Symbol::make("remove", this);
    s_copy = Symbol::make("copy", this);
    s_has_key = Symbol::make("has_key", this);
    s_get = Symbol::make("get", this);
    s_get2 = Symbol::make("get2", this);
    s_chr = Symbol::make("chr", this);
    s_ord = Symbol::make("ord", this);
    s_intern = Symbol::make("intern", this);
    s_hex = Symbol::make("hex", this);
    s_oct = Symbol::make("oct", this);
    s_id = Symbol::make("id", this);
    s_inst = Symbol::make("inst", this);
    s_subcl = Symbol::make("subcl", this);
*/
	s_runtime_exception = Symbol::make("runtime_exception", this);

    builtinfn_init(); // install builtin functions

	// create an initial frame and associated method so calls will work:
    frame = Frame::make(FRAME_LOCAL_BASE + EXPR_STACK_MAX, this);
	Method * method = Method::make(s_global, this);
	s_global->set_function(method, this); // save for others to use
	push(method); // protect from GC
	frame->init_method(method, this);
	pop();
	method->set_stack_offset(0);
	method->set_frame_slots(EXPR_STACK_MAX);
	// create callin_method to aid in debugging
	callin_method = Method::make(Symbol::make("<callin_method>", this), this);
	callin_method->set_stack_offset(0);
	callin_method->set_frame_slots(EXPR_STACK_MAX);
}



void Mach::mark_white()
{
	for (int i = SMALL_INT_MIN; i < SMALL_INT_MAX; i++) {
		small_ints[i - SMALL_INT_MIN]->set_color(GC_WHITE);
	}
}


/* 1 */

FClass * Mach::class_lookup(Symbol * s)
{
	FClass * fclass = s->get_value().fclass;
	if (fclass && fclass->has_tag(TAG_CLASS)) 
		return fclass;
	return NULL;
}


void Mach::class_add(Symbol * s, FClass * fclass)
{
	s->set_value(fclass, this);
}


void Mach::file_load(char *filename)
{
    Comp compiler;
    compiler.init(this);
    compiler.compile(filename);
}

