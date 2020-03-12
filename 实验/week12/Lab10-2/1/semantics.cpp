/* semantics -- */

#include "flincl.h"
#include "shortstack.h"
#include "semantics.h"
#include "stdio.h" // for compiler.h
#include "compiler.h"
#include "assert.h"

#define must(emit) if (!(emit)) return false;

typedef struct {
	char *name;
	short params;
	short op;
} Builtin_desc;

// built_ins are "global functions" that cannot be
// overridden or overloaded
Builtin_desc built_ins[] = {
	{ "array", 1, OPCODE_NEWARRAY },
    { "dict", 1, OPCODE_NEWDICT },
    { "flatten", 1, OPCODE_FLATTEN },
    { "repr", 1, OPCODE_REPR },
    { "str", 1, OPCODE_STR },
    { "type", 1, OPCODE_TYPE },
    { "len", 1, OPCODE_LENGTH },
    { "cos", 1, OPCODE_COS },
    { "sin", 1, OPCODE_SIN },
    { "tan", 1, OPCODE_TAN },
    { "exp", 1, OPCODE_EXP },
    { "sqrt", 1, OPCODE_SQRT },
    { "rem", 1, OPCODE_REM },
    { "min", 2, OPCODE_MIN },
    { "max", 2, OPCODE_MAX },
    { "random", 0, OPCODE_RANDOM },
    { "abs", 1, OPCODE_ABS },
    { "int", 1, OPCODE_TRUNCATE },
    { "round", 1, OPCODE_ROUND },
    { "float", 1, OPCODE_FLOAT },
    { "chr", 1, OPCODE_CHR },
    { "ord", 1, OPCODE_ORD },
    { "hex", 1, OPCODE_HEX },
    { "id", 1, OPCODE_ID },
    { "intern", 1, OPCODE_INTERN },
    { "isinstance", 2, OPCODE_INST },
    { "issubclass", 2, OPCODE_SUBCL },
    { "oct", 1, OPCODE_OCT },
    { "strcat", 2, OPCODE_STRCAT },
    { "subseq", 3, OPCODE_SUBSEQ },
    { "isupper", 1, OPCODE_ISUPPER },
    { "islower", 1, OPCODE_ISLOWER },
    { "isalpha", 1, OPCODE_ISALPHA },
    { "isdigit", 1, OPCODE_ISDIGIT },
    { "isalnum", 1, OPCODE_ISALNUM },
    { "toupper", 1, OPCODE_TOUPPER },
    { "tolower", 1, OPCODE_TOLOWER },
    { "find", 4, OPCODE_FIND },
	{ "exit", 0, OPCODE_EXIT },
	{ "frame_previous", 1, OPCODE_FRMPREV },
	{ "frame_variables", 1, OPCODE_FRMVARS },
	{ "frame_pc", 1, OPCODE_FRMPC },
	{ "frame_method", 1, OPCODE_FRMMETH },
	{ "frame_class", 1, OPCODE_FRMCLASS },
	{ "runtime_exception_nesting", 0, OPCODE_NEST },
	{ "frame_get", 0, OPCODE_FRMGET },
	{ NULL, 0, 0 }
};

// builtin_methods are implemented by oper_codes,
// but if the first operand's type is object,
// a method lookup is performed. Builtin_methods
// allow us to implement builtin methods on builtin
// types, and run more efficiently than a full
// method call which allocates a frame.
Builtin_desc builtin_methods[] = {
    { "readline", 0, OPCODE_READLINE },
    { "write", 1, OPCODE_WRITE },
    { "read", 1, OPCODE_READ },
	{ "unread", 1, OPCODE_UNREAD },
    { "readvalue", 0, OPCODE_READVAL },
    { "close", 0, OPCODE_CLOSE },
    { "token", 0, OPCODE_TOKEN },
	{ "append", 1, OPCODE_APPEND },
	{ "unappend", 0, OPCODE_UNAPPEND },
    { "last", 0, OPCODE_LAST },
    { "keys", 0, OPCODE_KEYS },
    { "values", 0, OPCODE_VALUES },
    { "clear", 0, OPCODE_CLEAR },
    { "index", 1, OPCODE_INDEX },
    { "reverse", 0, OPCODE_REVERSE },
    { "sort", 0, OPCODE_SORT },
    { "remove", 1, OPCODE_REMOVE },
    { "copy", 0, OPCODE_COPY },
    { "has_key", 1, OPCODE_HASKEY },
    { "get", 2, OPCODE_GET },
	{ "value", 0, OPCODE_VALUE },
    // { "get2", 2, OPCODE_GET2 },
    { NULL, 0, 0 }
};



// builtin -- searches for id in built-in list
//
// if found, return op-code required to execute the built-in
// and set actuals to the number of parameters required by
// the built-in function
// otherwise, return 0 and set actuals to -1
//
short Sem::builtin(char *id, short *actuals)
{
    int i = 0;
	while (built_ins[i].name) {
		if (streql(built_ins[i].name, id)) {
			*actuals = built_ins[i].params;
			return instr_compose(instr_op, built_ins[i].op);
		}
		i++;
	}
	*actuals = -1;
	return 0;
}


short Sem::builtin_method(char *id, short *actuals)
{
    int i = 0;
    while (builtin_methods[i].name) {
        if (streql(builtin_methods[i].name, id)) {
            *actuals = builtin_methods[i].params;
            return instr_compose(instr_op, builtin_methods[i].op);
        }
        i++;
    }
    *actuals = -1;
    return 0;
}


bool Sem::init(Mach * m, Comp_ptr c)
{
	mach = m;
	comp = c;
	instructions = NULL;
	if_stack = Short_stack::make(3, mach);
	looper_stack = Short_stack::make(2, mach);
	assign_stack = Short_stack::make(2, mach);
	indent_stack = Short_stack::make(8, mach);
	array_stack = Short_stack::make(2, mach);
	call_stack = Short_stack::make(2, mach);
    return if_stack && looper_stack && assign_stack && 
        indent_stack && array_stack && call_stack;
    // BUG: need error report here if returning false
}


Sem::Sem()
{
}


bool Sem::reset()
{
    if (instructions) {
        //FLSys::free(instructions, max_instr);
    }
	class_started = false;
	method_started = false;
	local_offset = 0;
	member_offset = 0;
    next_instr = 0;
    max_instr = 0;
	method = NULL;
	fclass = NULL;
	method_name = NULL;
    class_name = NULL;
    var_name = NULL;
	array_index = 0;
	opcode = 0;
    actuals = 0;
	// initially has room for 16 symbols
	symbols = Array::make(0, 16, mach);
    return symbols != NULL;
}


bool Sem::emit(short instruction)
{
    if (next_instr >= max_instr) {
        if (!grow()) return false;
    }
    if (next_instr < max_instr) {
        instructions->set_instr(next_instr++, instruction, 
                                mach->trace_flag);
    }
    return true;
}


bool Sem::emit_at(long location, long instruction)
{
    assert(location < next_instr);
	if (instruction > 0x7FFF || instruction < -0x10000) {
		comp->report_error("virtual machine limitation: branch out of bounds");
		return false;
	}
    instructions->set_instr(location, (short) instruction, 
                            mach->trace_flag);
	return true;
}


bool Sem::for_var(char *id)
{
	long index;  // variable index in frame
	// is this a local? look in method symbol table
	Symbol * s = Symbol::make(id, mach);
    if (!method->get_localtable()->lookup(s, &index)) {
		comp->report_error("undeclared loop variable");
		return false;
	}
	// now index is the loop variable offset
	looper_stack->push((short) index, mach);
	return true;
}


bool Sem::for_init()
{
	short index = looper_stack->pop(mach);
	// store the initial value
	must(emit(instr_compose(instr_store_local, (short) index)));
	looper_stack->push(index, mach);  // save for later
	return true;
}


bool Sem::for_to()
{
	return true;
}


bool Sem::for_by(bool by_flag)
{
	if (!by_flag) { // no BY expression
		emit(instr_compose(instr_load_int, 1));
	}
	// generate instruction to push the loopcounter
	short index = looper_stack->pop(mach);
	looper_stack->push(index, mach);
	emit(instr_compose(instr_load_local, (short) index));
	emit(OPCODE_JUMP);
	looper_stack->push(next_instr, mach);
	emit(0);
	// machine stack is to_value, by_value, loopcount (top)
	return true;
}


// for_end_range -- called after for-=-to-by-:
//
bool Sem::for_end_range()
{
	long looptop = looper_stack->pop(mach);
	long loopvar = looper_stack->pop(mach);
	return emit(OPCODE_LOOPINC) &&
	       emit((short) loopvar) &&
		   // jump instruction is pc += *pc,
		   //    next_instr = looptop + *looptop,
		   //    *looptop = next_instr - looptop
		   emit_at(looptop, next_instr - looptop) &&
		   emit(OPCODE_LOOPTST) &&
		   // jump instruction is pc += *pc
		   //    looptop + 1 = next_instr + *next_instr
		   //    *next_instr = looptop + 1 - next_instr
		   emit(looptop + 1 - next_instr) &&
	       // break instructions should branch to here
		   emit(OPCODE_POP2);
}


bool Sem::for_array()
{
	short index = looper_stack->pop(mach);
	must(emit(instr_compose(instr_load_int, 0)));
	must(emit(OPCODE_JUMP));
	looper_stack->push(next_instr, mach);
	return emit(0) &&
		   // store the initial value
		   emit(instr_compose(instr_store_local, index));
}


bool Sem::for_end_elements()
{
	// stack has looptop-1
	// machine stack has array index, then array
	long looptop = looper_stack->pop(mach);
	return emit_at(looptop, next_instr - looptop) &&
		   emit(OPCODE_LOOPNXT) &&
		   emit(looptop - next_instr);
}


bool Sem::return_null()
{
    emit(OPCODE_RETNULL);
    return true;
}


// unemit -- undo the previous emit, and return opcode
//
short Sem::unemit()
{
    if (next_instr <= 0) return OPCODE_NOOP;
    next_instr--;
    return instructions->get_instr(next_instr);
}

    
bool Sem::grow()
{
    short newlen = 128;
    if (newlen <= max_instr) newlen = max_instr * 2;
    assert(max_instr < newlen);
    Code_ptr newinstr = Code::make(newlen, mach);
    if (!newinstr) return false;
	if (next_instr > 0) {
        for (long i = 0; i < next_instr; i++) {
            newinstr->set_instr(i, instructions->get_instr(i), false);
		}
	}
    instructions = newinstr;
    max_instr = newlen;
    return true;
}


// call prepares to invoke a function.
// The current opcode,actuals pair is pushed
// The new opcode is looked up and saved until
// after parameters are ready
//
bool Sem::call(char *id)
{
	call_stack->push(opcode, mach);
	call_stack->push(actuals, mach);
	opcode = builtin(id, &actuals);
	if (opcode == 0) {
	    long index = insert_symbol(id);
	    assert(index >= 0 && index < 2048);
	    emit(instr_compose(instr_local_frame, (short) index));
	}
	return true;
}


bool Sem::method_call(char *id)
{
	call_stack->push(opcode, mach);
	call_stack->push(actuals, mach);
    opcode = builtin_method(id, &actuals);
    if (opcode == 0) {
	    long index = insert_symbol(id);
	    assert(index >= 0 && index < 2048);
	    opcode = instr_compose(instr_frame, (short) index);
    }
    return true;
}


bool Sem::class_start(char *class_name, char *super_name)
{
	fclass = FClass::make(mach);
	if (!fclass) return false;
	Symbol * s = Symbol::make(class_name, mach);
	mach->push(s);
	FClass * probe = mach->class_lookup(s);
	if (probe) {
		comp->report_error("class is already defined");
		mach->pop();
		fclass = NULL;
		return false;
	}
	mach->class_add(s, fclass);
	mach->pop();
	if (super_name[0]) {
		s = Symbol::make(super_name, mach);
		FClass * super = mach->class_lookup(s);
		if (!super) {
			comp->report_error("super class is undefined");
			fclass = NULL;
			return false;
		}
		fclass->init_super(super);
		member_offset = super->get_inst_slots();
	} else {
		member_offset = 1; // all objects have class pointer
	}
	class_started = true;
	return true;
}


bool Sem::method_start()
{
    method_started = true;
    local_offset = 0;
    return true;
}


bool Sem::method_end()
{
	emit(instr_compose(instr_op, OPCODE_RETNULL));
	// copy the code vector to the method
    Code_ptr newinstr = Code::make(next_instr, mach);
    if (!newinstr) return false;
    for (long i = 0; i < next_instr; i++) {
        newinstr->set_instr(i, instructions->get_instr(i), false);
	}
    int len = 1;
    if (mach->trace_flag) {
        for (i = 0; i < next_instr; i += len) {
            char text[64];
            trace(newinstr->disassemble(i, newinstr->get_instr(i), 
                                        newinstr->get_instr(i + 1), text, 
                                        &len, method));
        }
    }
	method->set_code(newinstr, mach);
	method = NULL;
	method_started = false;
	next_instr = 0;
	return true;
}


bool Sem::method_name_is(char *name)
{
	method_name = Symbol::make(name, mach);
    method = Method::make(method_name, mach);
    if (class_started) { // insert method in the class
		fclass->enter_method(method_name, method, mach);
		method->set_fclass(fclass, mach);
	} else { // attach method to symbol
        method_name->set_function(method, mach);
        method->set_fclass(NULL, mach);
    }
    // add symbols to method:
   	method->init_symbols(symbols, mach);
	if (mach->trace_flag)
        trace("method_name_is: symbol name is %s\n", 
		      method_name->get_name()->get_string());
    return true;
}


bool Sem::begin_formals()
{
    formal_type = FORMAL_REQUIRED;
    return true;
}


bool Sem::set_formal_type(short ft)
{
    if (ft < formal_type) {
    	comp->report_error("formal type out of order");
        return false;
    }
    formal_type = ft;
    return true;
}


bool Sem::formal_is(char *name)
{
	// convert the name to a symbol in this class
	var_name = Symbol::make(name, mach);
    // position the stack above the parameter:
	method->set_stack_offset(1 + method->get_stack_offset());
    switch (formal_type) {
      case FORMAL_OPTIONAL:
        method->set_optionals(1 + method->get_optionals());
        // no break, fall through to increment parameters too!
      case FORMAL_REQUIRED:
        method->set_parameters(1 + method->get_parameters());
        break;
      case FORMAL_KEYWORD:
        method->set_keywords(1 + method->get_keywords());
        break;
      case FORMAL_REST:
        method->set_rest_flag(true);
        break;
      case FORMAL_DICTIONARY:
        method->set_dict_flag(true);
        break;
    }
        
    return method->get_localtable()->insert_var(var_name, local_offset++, mach);
}


bool Sem::default_value(FVal v)
{
    // first see if there is an optionals array:
    Array * defaults = method->get_default();
    if (!defaults) {
        defaults = Array::make(1, 1, mach);
        method->set_default(defaults);
    }
    // this new value is for local_offset-1, but the defaults start
    // at parameters - optionals
    long index = local_offset - 1 - 
        (method->get_parameters() - method->get_optionals());
    // make sure array is long enough:
    while (defaults->get_array_len() <= index) {
        defaults->append(NULL, mach);
    }
    defaults->set_fastref(index, v, mach);
    return true;
}


bool Sem::end_of_formals()
{
    // signature is number of formals
    // stack_offset is number of formals + locals
	// frame_slots is stack_offset + maximum stack size
	method->set_frame_slots(method->get_stack_offset() + 
							EXPR_STACK_MAX);
    return true;
}


bool Sem::id(char *id)
{
	Symbol * s = Symbol::make(id, mach);
	long index; // variable index in frame
	Method * mp;
	mach->push(s);
	// is this a local? look in method symbol table
    if (method->get_localtable()->lookup(s, &index)) {
		must(emit(instr_compose(instr_load_local, (short) index)));
	} else if (fclass &&
               fclass->get_symboltable()->lookup(s, &mp, &index) &&
			   mp == NULL) {
		must(emit(instr_compose(instr_load_instvar, (short) index)));
	} else { // load value from a global
		index = insert_symbol(id);
		must(emit(instr_compose(instr_load_global, (short) index)));
	}
	mach->pop();
	return true;
}


bool Sem::assign_id(char *id)
{
	Symbol * s = Symbol::make(id, mach);
	long index; // variable index in frame
	Method * mp;
	mach->push(s);
	// is this a local? look in method symbol table
    if (method->get_localtable()->lookup(s, &index)) {
		must(emit(instr_compose(instr_store_local, (short) index)));
	} else if (fclass && 
               fclass->get_symboltable()->lookup(s, &mp, &index) &&
			   mp == NULL) {
		trace("assign_id got instance index of %d\n", index);
		must(emit(instr_compose(instr_store_instvar, (short) index)));
	} else { // load value from a global
		index = insert_symbol(id);
		must(emit(instr_compose(instr_store_global, (short) index)));
	}
	mach->pop();
	return true;
}


bool Sem::field(char *id)
{
	trace("field not implemented\n");	// not implemented
	return true;
}


bool Sem::aref()
{
	return emit(OPCODE_AREF);
}

bool Sem::if_true()
{
    // emit a test to branch over true part
    must(emit(OPCODE_JUMPIFNOT));
    // save PC for the branch location
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Sem::if_else()
{
    // this is the target for the if:
	long target = if_stack->pop(mach);
    // emit a jump around else part
    must(emit(OPCODE_JUMP));
    // save PC for the branch location
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    emit(0);
    return emit_at(target, next_instr - target);
}


bool Sem::if_end()
{
    // this is the target for whatever is on the stack
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
}


bool Sem::while_begin()
{
    // remember where to branch back to
    looper_stack->push(next_instr, mach);
    return true;
}

bool Sem::while_true()
{
    // emit a test to branch over true part
    must(emit(OPCODE_JUMPIFNOT));
    // save PC for the branch location
    looper_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Sem::while_end()
{
    // top of stack is branch out of loop
    long end_of_looper_branch = looper_stack->pop(mach);
    // now emit loop branch
    must(emit(OPCODE_JUMP));
    must(emit(looper_stack->pop(mach) - next_instr));
    // fix up previous branch to next instr (after loop)
    return emit_at(end_of_looper_branch, next_instr - end_of_looper_branch);
}


bool Sem::return_end()
{
    return emit(OPCODE_RETURN);
}


bool Sem::load_end()
{
    return emit(OPCODE_LOAD);
}


bool Sem::print_expr()
{
	return emit(OPCODE_PRINT);
}


bool Sem::print_comma()
{
    return emit(OPCODE_SPACE);
}


bool Sem::print_end()
{
    return emit(OPCODE_ENDL);
}


// assign_end -- called after rval is on stack
// 
// emit pushed instruction to finish the job:
// possibilities are:
//   assign to array: stack has array, index, value
//   assign to head: stack has node, value
//   assign to tail: stack has node, value
//   assign to variable: stack has value
//   assign to field of object: stack has object, index, value
//
bool Sem::assign_end()
{
    short op = assign_stack->pop(mach);
    return emit(op);
}


// rval2lval -- prepare for an assignment statement
//
// when you encounter an '=', the expression on the left has
// just emitted an instruction to load the value to the stack
// The method "undoes" the emit, and converts the instruction
// from one that loads a value to one that stores a value.
// Error is returned if the instruction cannot be converted.
// E.g. if the instruction is an array reference, convert to 
// a store.  If the instruction is addition, return false.
//
bool Sem::rval2lval()
{
    // first get previous instruction
    short op = unemit();
    short newop = OPCODE_NOOP;
    
    // now compute the assignment version of op
    short typ = instr_type(op);
    short arg = instr_arg(op);
    if (typ == instr_op) {
        if (op == OPCODE_AREF) newop = OPCODE_SETAREF;
        else if (op == OPCODE_FIELD) newop = OPCODE_SETFIELD;
    } else if (typ == instr_load_global) {
        newop = instr_compose(instr_store_global, arg);
    } else if (typ == instr_load_instvar) {
        newop = instr_compose(instr_store_instvar, arg);
    } else if (typ == instr_load_local) {
        newop = instr_compose(instr_store_local, arg);
    }
    if (newop != OPCODE_NOOP) {
        assign_stack->push(newop, mach);
        return true;
    }
    return false;
}
        

bool Sem::unary_minus()
{
    return emit(OPCODE_MINUS);
}


bool Sem::not()
{
    return emit(OPCODE_NULL);
}


bool Sem::lognot()
{
    return emit(OPCODE_LOGNOT);
}


bool Sem::push_long(int64 value)
{
	if (mach->arg_sign_extend(instr_arg((long) value)) == value) {
		return emit(instr_compose(instr_load_int, instr_arg((short) value)));
	} else {
		FVal lit = FLong::make(value, mach);
		mach->push(lit);
		assert(method);
		short arg = method->literal_add(lit, mach);
		mach->pop();
		return emit(instr_compose(instr_load_lit, arg));
	}
}

bool Sem::push_double(double value)
{
	FVal lit = FDouble::make(value, mach);
	return push_fval(lit);
}


bool Sem::push_string(char *s)
{
    FVal lit = FString::make(s, mach);
	return push_fval(lit);
}


bool Sem::push_symbol(char *s)
{
    FVal lit = Symbol::make(s, mach);
    return push_fval(lit);
}


bool Sem::push_fval(FVal v)
{
	mach->push(v);
	assert(method);
	short arg = method->literal_add(v, mach);
	mach->pop();
	return emit(instr_compose(instr_load_lit, arg));
}


bool Sem::var_decl(char * id)
{
	if (method_started) { // local variable in method
		assert(method);
		Symbol * s = Symbol::make(id, mach);
		assert(s);
		mach->push(s);
		trace("var_decl: before new method variable, locals are:\n");
		method->get_localtable()->trace_locals();
		if (!method->get_localtable()->
			insert_var(s, local_offset++, mach)) {
			comp->report_error("local variable already declared");
			mach->pop();
			return false;
		}
		method->set_stack_offset(1 + method->get_stack_offset());
		mach->pop();
	} else if (class_started) {
		assert(fclass);
		Symbol * s = Symbol::make(id, mach);
		mach->push(s);
		trace("instance var at index %d\n", member_offset);
		if (!fclass->get_symboltable()->
			insert_var(s, member_offset++, mach)) {
			comp->report_error("instance variable already declared");
			mach->pop();
			return false;
		}
		mach->pop();
		fclass->set_inst_slots(member_offset);
	}
	// undeclared variables are globals, the value is stored on the
	// symbol itself, so we do not need to enter it into a table

	// show current state of things
	if (method_started) {
		trace("current locals are:\n");
		method->get_localtable()->trace_locals();
	} else if (class_started) {
		trace("current members are:\n");
		fclass->get_symboltable()->trace_members();
	} else trace("global variable declared\n");
	return true;		
}


// Here's how array code generation works:
//    the current array_index is pushed on array_stack
//     and array_index is set to 0
//     (in case we are dealing with nested arrays)
//    emit placeholder for the size of the array
//     (so there is an upper bound on the size of a 
//     literal array: the maximum literal integer,
//     which is 2047)
//    the location of the literal is pushed on array_stack
//    the create array instruction is emitted
//    for each expression, we duplicate the top of stack,
//     emit a literal index, and emit a store into array op
//    at array_end, we know the size of the array, so we
//     pop the stack and emit the size at that location
//    finally, pop the previous array_index value
bool Sem::array_start()
{
	array_stack->push(array_index, mach);
	array_index = 0;
	array_stack->push(next_instr, mach);
	return emit(0) && emit(OPCODE_NEWARRAY);
}

bool Sem::dict_start()
{
	array_stack->push(array_index, mach);
	array_index = 0;
	array_stack->push(next_instr, mach);
	return emit(0) && emit(OPCODE_NEWDICT);
}



bool Sem::before_array_expr()
{
	short instr = instr_compose(instr_load_int, array_index);
	if (instr_arg(instr) != array_index) {
		comp->report_error("array expression too long");
		return false;
	}
	array_index++;
    return emit(OPCODE_DUP) && emit(instr);
}


bool Sem::before_dict_key()
{
	array_index++;
    return emit(OPCODE_DUP);
}


bool Sem::before_dict_val()
{
    return true;
}



bool Sem::array_expr()
{
    return emit(OPCODE_SETAREF);
}

bool Sem::dict_key()
{
    return true;
}


bool Sem::dict_val()
{
    return emit(OPCODE_SETAREF);
}



bool Sem::array_end()
{
	must(emit_at(array_stack->pop(mach), 
			     instr_compose(instr_load_int, array_index)));
	array_index = array_stack->pop(mach);
	return true;
}


bool Sem::dict_end()
{
	must(emit_at(array_stack->pop(mach), 
			     instr_compose(instr_load_int, array_index)));
	array_index = array_stack->pop(mach);
	return true;
}


// here's how calls work: 
// there are many built-in functions that compile to instructions
// which must be handled here rather than at run-time. When call()
// or method_call() is called, we look up the function to see if it
// is built-in. If so, we push the corresponding opcode and then
// the required number of parameters onto the call_stack (a 
// compile-time structure). If the function is not built-in, we
// push zero and -1. Whenever we get an actual, we look at the top
// of stack. If opcode = 0 (not built-in), we generate a param opcode, 
// otherwise we decrement
// the top of stack. (At runtime the parameter will be left on the
// expression stack, not copied to a frame.) If the top of stack 
// would go to -1, we return an error (too many parameters).
// When we get an actual_end() we look at the opcode:
// If non-zero, emit it. If zero, emit the
// call opcode. If actuals is greater than zero, return an error
// (too few actuals).
// FOR CONVENIENCE, we will keep the top of the stack in member vars,
// opcode and actuals.

bool Sem::actual()
{
    if (opcode == 0) return emit(OPCODE_PARAM);
	if (actuals == 0) {
		comp->report_error("too many parameters");
		return false;
	}
	actuals--;
	return true;
}


bool Sem::keyword(long index)
{
    assert(index < 0x10000);
    if (opcode == 0) {
        return emit(OPCODE_KEYPARAM) && emit((short) index);
    } else {
        comp->report_error("unexpected keyword parameter");
        return false;
    }
}


bool Sem::actual_end()
{
    // if optional start, stop parameters are missing, fill
    // them in now
    if (opcode == instr_compose(instr_op, OPCODE_FIND)) {
        if (actuals == 2) {
            push_long(0);
            actuals--;
        }
        if (actuals == 1) {
            push_long(-1);
            actuals--;
        }
    // this is another style for variable argument lists
    // if get(key) use OPCODE_GET, if get(key, default) use OPCODE_GET2:
    } else if (opcode == instr_compose(instr_op, OPCODE_GET)) {
        if (actuals == 1) {
            actuals--; // OPCODE_GET really takes one parameter
        } else if (actuals == 0) {
            // 2 parameters, so use OPCODE_GET2:
            opcode = instr_compose(instr_op, OPCODE_GET2);
        }
    }
	if (actuals > 0) {
		comp->report_error("too few parameters");
		return false;
	}
	if (opcode != 0) emit(opcode);
	else emit(instr_compose(instr_op, OPCODE_CALL));
    actuals = call_stack->pop(mach);  // restore actuals
	opcode = call_stack->pop(mach);   // restore opcode
	return true;
}


bool Sem::end_id_expr()
{
	emit(instr_compose(instr_op, OPCODE_POP));
	return true;
}

int Sem::insert_symbol(char *str)
{
	Symbol * s = Symbol::make(str, mach);
	mach->push(s);
	long len = symbols->get_array_len();
	for (int i = 0; i < len; i++) {
		if (symbols->fastref(i).symbol == s) {
			mach->pop();
			return i;
		}
	}
	symbols->append(s, mach);
	mach->pop();
	return i;
}


bool Sem::relop(char *op)
{
    if (streql(op, "<")) return emit(OPCODE_LESS);
    if (streql(op, ">")) return emit(OPCODE_GTR);
    if (streql(op, "<=")) return emit(OPCODE_LESSEQL);
    if (streql(op, ">=")) return emit(OPCODE_GTREQL);
    if (streql(op, "==")) return emit(OPCODE_ISEQUAL);
    if (streql(op, "!=")) return emit(OPCODE_NOTEQUAL);
    if (streql(op, "is")) return emit(OPCODE_IS);
    if (streql(op, "isnot")) return emit(OPCODE_ISNOT);
    if (streql(op, "in")) return emit(OPCODE_MEMBER);
    if (streql(op, "notin")) return emit(OPCODE_NOTMEMBER);
    return false;
}


bool Sem::addop(char *op)
{
    if (streql(op, "+")) return emit(OPCODE_ADD);
    if (streql(op, "-")) return emit(OPCODE_SUB);
    return false;
}


bool Sem::mulop(char *op)
{
    if (streql(op, "*")) return emit(OPCODE_MUL);
    if (streql(op, "/")) return emit(OPCODE_DIV);
    if (streql(op, "%")) return emit(OPCODE_REM);
    return false;
}


bool Sem::power()
{
    return emit(OPCODE_POWER);
}


bool Sem::begin_and(char *op)
{
    if (op[0] == '&') return true; // logical and
    must(emit(OPCODE_CNDAND));
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Sem::end_and(char *op)
{
    if (op[0] == '&') return emit(OPCODE_LOGAND);
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
    // emit a jump around 2nd operand to here
}

bool Sem::begin_or(char *op)
{
    if (op[0] == '|' ||
        op[0] == '^') return true; // logical or
    must(emit(OPCODE_CNDOR));
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Sem::end_or(char *op)
{
    if (op[0] == '|') return emit(OPCODE_LOGIOR);
    if (op[0] == '^') return emit(OPCODE_LOGXOR);
    if (op[0] == '<') return emit(OPCODE_LSHIFT);
    if (op[0] == '>') return emit(OPCODE_RSHIFT);
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
    // emit a jump around 2nd operand to here
}



