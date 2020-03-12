dddd
bbbab
/* call.cpp -- implements method calls and returns, jumps, etc. */


#include "memory.h"
#include "flincl.h"
#include "assert.h"


void Mach::oper_frame(Symbol * sym)
{
	if (trace_flag) {
        trace("instr_frame:\n");
	    frame->trace();
    }
	Method * method = frame->get_method();
    FClass * fclass = method->get_fclass();
    Symbol * selector = sym;
	long var_index;
    //  now look up the method:
    Object * obj = read_tos().object;
    if (!obj) { // self == NULL, making a call from a global function
        method = selector->get_function();
    } else {
        obj->type_check(this);
        if (error_flag) return;
        if (!obj->get_fclass()->get_symboltable()->lookup(
				        selector, &method, &var_index) ||
			!method) {
			method = selector->get_function();
            if (method) obj = NULL;
        }
    }
	if (!method) {
		undefined_method(selector);
		return;
	}
    do_frame(obj, method);
}


// do_frame -- creates a new frame
//
// On entry:
//           top of stack is to be replaced by frame
//           zero (arg index) is pushed
void Mach::do_frame(Object * obj, Method * method)
{
    long frame_slots;
    long n_parms;
    // method may be builtin or ordinary
    if (method->has_tag(TAG_METHOD)) {
        frame_slots = method->get_frame_slots();
	    n_parms = method->get_parameters() +
                  method->get_keywords() + method->get_rest_flag() +
                  method->get_dict_flag();
    } else {
        Builtin * builtin = (Builtin *) method;
        assert(method->has_tag(TAG_BUILTIN));
        n_parms = builtin->get_parameters();
        frame_slots = (n_parms != 0xFFFF ? n_parms : 1);
    }
    Frame * frame = Frame::make(FRAME_LOCAL_BASE + frame_slots, this);

    // store object, method, and current frame in new frame:
    frame->init_self(obj, this); 
    frame->init_method(method, this); 
    frame->init_previous(frame, this);
    // replace tos with frame, overwriting object:
    (*(tos - 1)).frame = frame;
	if (n_parms == 0xFFFF) { // this mechanism is for builtins only
		frame->set_local(0, Array::make(0, 3, this), this);
	}
	// this is the argument index:
    push(FLong::make(0, this));
    if (method->has_tag(TAG_METHOD)) {
        Array * defaults = method->get_default();
        long parameters = method->get_parameters();
        if (defaults) {
            // copy default values into frame:
            long base = parameters - method->get_optionals();
            push(defaults);
            oper_print();
            for (int i = 0; i < defaults->get_array_len(); i++) {
                frame->set_local(base + i, defaults->fastref(i).node, this);
            }
        }
        parameters += method->get_keywords();
        if (method->get_rest_flag()) {
            // create rest array?
		    frame->set_local(parameters, Array::make(0, 3, this), this);
        }
        if (method->get_dict_flag()) {
            // create the dictionary
            frame->set_local(parameters + method->get_rest_flag(),
                             Dict::make(this), this);
        }
    }
}


/*
void Mach::oper_frame()
{
    int index = *pc++;  // fetch the method index
    Object * obj = read_tos().object->type_check(this);
    Method * method = obj->get_fclass()->get_methods()->method(index);
    do_frame(obj, method);
}
*/

/*
void Mach::oper_gframe()
{
    push(globals);
    oper_frame();
}
*/

void Mach::oper_param()
{
    /* on entry, stack (top down) contains:
     *  parameter, index, frame
     */
    FVal param = read_tos().node;
    int index = (int) (read_next_tos().flong->int64_data);
    Frame * frame = read_next_next_tos().frame;
    Method * method = frame->get_method();
	long parameters = method->get_parameters();
    if (index < parameters) {
        /* the first local is the first arg, etc. */
        frame->set_local(index, param, this);
		pop(); // pop the parameter
        replace(FLong::make(index + 1, this));
    } else if (parameters == 0xFFFF) { // special case for built-in
		// append param to array in frame local 0
		frame->local(0).array->append(param, this);
	    pop(); // pop the parameter
        replace(FLong::make(index + 1, this));
	} else if (method->has_tag(TAG_BUILTIN)) {
        too_many_parameters();
    } else if (method->get_rest_flag()) {
        frame->local(parameters + method->get_keywords()).array->
            append(param, this);
        pop(); // pop the parameter
	} else too_many_parameters();
}


void Mach::oper_keyparam()
{
    /* on entry, stack (top down) contains:
     * parameter, index, frame; keyword is next opcode
     */
    FVal param = read_tos().node;
    Frame * frame = read_next_next_tos().frame;
    Method * method = frame->get_method();
    Symbol * key = method->symbol(*pc++);
    if (trace_flag) trace("keyparam is %s\n", key->get_name()->get_string());
    if (method->has_tag(TAG_BUILTIN)) {
        invalid_keyword_parameter();
        return;
    }   
    // keyword should be in locals dictionary and should be between
    // parameters and parameters + keywords
	long parameters = method->get_parameters();
    long keywords = method->get_keywords();
    long index;
    if (!method->get_localtable()->lookup(key, &index)) {
        if (method->get_dict_flag()) {
            Dict_ptr dict = frame->local(parameters + keywords +
                                         method->get_rest_flag()).dict;
            dict->insert(key, param, this);
            pop();
            replace(FLong::make(index + 1, this));
            return;
        }
    } else if (index >= parameters && index < parameters + keywords) {
        frame->set_local(index, param, this);
        pop();
        return;
    }
    invalid_keyword_parameter();
}


void Mach::oper_call()
{
    Frame * nextframe = read_next_tos().frame;
	// check parameter count
	Method * method = nextframe->get_method();
    // get_signature works for both methods and builtins
	long parameters = method->get_parameters();
    long optionals = method->get_optionals();
	if ((parameters != 0xFFFF) && 
        (parameters - optionals > (int) (read_tos().flong->int64_data))) {
		parameter_count_mismatch();
		return;
	}
    poper_n(2); // get rid of the arg counter and nextframe
	// now copy the stack into the frame
	// the destination is stored in the method:
	long offset = FRAME_LOCAL_BASE + method->get_stack_offset();
	// the size is simply tos - expr_stack
	// trace("call frame->set_tos(%d)\n", tos - expr_stack);
	frame->set_tos(tos - expr_stack);
	FUnion *p = expr_stack;
	while (p < tos) {
		if (trace_flag) trace("call saving %x at %d\n", *p, offset);
		set_ptr_in(frame, offset++, (*p).node, this);
		p++;
	}
	tos = expr_stack;   // empty the expression stack
	frame->set_pc(pc);	// return address
    nextframe->init_previous(frame, this);
    frame = nextframe;
	::trace("in oper_call\n");
	frame->trace_stack();
    if (method->has_tag(TAG_METHOD)) {
	    if (trace_flag) {
            trace("oper_call method:\n");
	        method->trace();
        }
        pc = method->get_code()->initial_pc();
    } else if (method->has_tag(TAG_BUILTIN)) {
        Builtin * builtin = (Builtin *) method;
	    if (trace_flag) trace("oper_call builtin\n");
        Builtin_fn_ptr fnptr = builtin->get_fn();
        (*fnptr)(this);
        // after the call, restore the frame
        frame = frame->get_previous();
        // result is already on top of stack and
        // pc is already pointing to next instruction
        // I'm not sure if we need to test pc here
        if (!pc) running = false;
    } else {
        report_error("method or builtin expected");
        assert(false);
    }
}


void Mach::oper_return()
{
    // tos is the return value -- we will transfer it to the stack
    FVal result = pop().node;
    // move the reference to previous frame to frame:
    Frame * prev = frame->get_previous();
    frame = prev;

    // now restore the stack from frame:
    // first, get the from location
    long offset = FRAME_LOCAL_BASE + frame->get_method()->get_stack_offset();
    // the to location is simply the expr_stack,
    // the count is stored in the frame
    tos = expr_stack + frame->get_tos();
	if (trace_flag) trace("return frame->tos() is %d\n", frame->get_tos());
    FUnion *p = expr_stack;
    while(p < tos) {
		if (trace_flag) trace("return restore %x from %d\n", 
            frame->get_ptr(offset), offset);
		*p++ = frame->get_ptr(offset++);
	}
    // now clear the frame's copy:
    frame->set_tos(0);
    
    // now push the return value
    push(result);
	pc = frame->get_pc();
	if (trace_flag) trace("pc restored to %x\n", pc);
	if (!pc) running = false;
}


void Mach::oper_retnull()
{
    // move the reference to previous frame to frame:
    Frame * prev = frame->get_previous();
    frame = prev;
	// now restore the stack from frame:
	// first, get the from location
	long offset = FRAME_LOCAL_BASE + frame->get_method()->get_stack_offset();
	// the to location is simply the expr_stack,
	// the count is stored in the frame
	tos = expr_stack + frame->get_tos();
	if (trace_flag) trace("return frame->tos() is %d\n", frame->get_tos());
	FUnion *p = expr_stack;
	while(p < tos) {
		if (trace_flag) trace("return restore %x from %d\n", 
                              frame->get_ptr(offset), offset);
		*p++ = frame->get_ptr(offset++);
	}
	// now clear the frame's copy:
	frame->set_tos(0);
    // now push the return value
    push(NULL);
	pc = frame->get_pc();
	if (trace_flag) trace("pc restored to %x\n", pc);
	if (!pc) running = false;
}


void Mach::oper_pop2()
{
	pop(); pop();
}


void Mach::oper_pop()
{
	pop();
}


void Mach::oper_jump()
{
    // the short following the pc is a relative offset:
    pc += *(int16 *)pc;
}


void Mach::oper_jumpifnot()
{
	FVal v = pop().node;
	if (v == NULL) pc += *(int16 *)pc; // add the offset
	else pc++; // skip the offset
}


void Mach::oper_jumpif()
{
	FVal v = pop().node;
	if (v != NULL) pc += *(int16 *)pc; // add the offset
	else pc++; // skip the offset
}


void Mach::oper_cndand()
{
	FVal v = read_tos().node;
	if (v == NULL) pc += *(int16 *)pc; // add the offset
	else {
        pop();
        pc++; // skip the offset
    }
}


void Mach::oper_cndor()
{
	FVal v = read_tos().node;
	if (v != NULL) pc += *(int16 *)pc; // add the offset
	else {
        pc++; // skip the offset
        pop();
    }
}



// oper_send called from: send(object, selector, param1, param2, ...)
// and compiled as push object, push selector, send (creates frame), 
// param1, param, param2, param, call
//
void Mach::oper_send()
{
	Symbol * selector = pop().symbol->type_check(this);
	Object * obj = read_tos().object->type_check(this);
	if (error_flag) return;
	Method * method;
	long var_index;
	if (!obj->get_fclass()->get_symboltable()->lookup(
		    selector, &method, &var_index) || !method) {
	    method = selector->get_function();
		if (!method) {
			undefined_method(selector);
			return;
		}
		obj = NULL;
	}
	do_frame(obj, method);
}


void Mach::oper_super()
{
	Symbol * selector = pop().symbol->type_check(this);
	Object * obj = read_tos().object->type_check(this);
	if (error_flag) return;
	Method * method;
	long var_index;
	FClass * cl = obj->get_fclass()->get_super();
	if (!cl->get_symboltable()->lookup(
		    selector, &method, &var_index) || !method) {
	    method = selector->get_function();
		if (!method) {
			undefined_method(selector);
			return;
		}
		obj = NULL;
	}
	do_frame(obj, method);
}


void Mach::oper_xcall()
{
	union { uint16 op[2];
	        void (*fn)();
	} u;
	u.op[0] = *pc++;
	u.op[1] = *pc++; 
	(*(u.fn))();
}


// oper_loopinc -- loop variable increment and load
// 
// the loop var offset is in the next word (*pc)
// the top of stack is the increment
// add increment to the loop variable and leave
// the result on the stack for the next instruction
// as an optimization, we jump directly to the next
// instruction, which must be oper_looptst()
//
void Mach::oper_loopinc()
{
	short offset = *pc++;
	push(read_tos().node); // duplicate the increment
	push(frame->local(offset).node); // load the local
	oper_add();
	frame->set_local(offset, read_tos().node, this);
}


// oper_looptst -- test and branch
// 
// must follow oper_loopinc, assumes loop var is
// on top of stack, next is increment, and below
// that is the final value
//
void Mach::oper_looptst()
{
	push(read_next_next_tos().node); // copy final val to tos
	oper_less();
	oper_jumpif();
}


// oper_loopnxt -- load next array element and branch
//
// stack top has index, next is array
// pc points to address to branch to
//
void Mach::oper_loopnxt()
{
	short arg = *pc++;
	long i = (long) (read_tos().flong->must_get_long(this));
	if (error_flag) return; // error -- didn't find index
	Array * ap = read_next_tos().array->type_check(this);
	if (!ap) return; // error -- not an array
	long len = ap->get_array_len();
	if (i >= len) {
        poper_n(2);
        return; // normal return after last element
    }
	replace(FLong::make(i + 1, this));
	push(ap->fastref(i).node);
	pc += arg;
	return;
}


void Mach::oper_exit()
{
	running = false;
	exit_flag = true;
}






    


    
