#include "CGRpch.h"

/*
 * Copyright (C)2015-2016 Haxe Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define OP(_,n) n,
#define OP_BEGIN static int hl_op_nargs[] = {
#define OP_END };
#include "HashlinkOpcode.h"

#define OP(n,_) #n,
#define OP_BEGIN static const char *hl_op_names[] = {
#define OP_END };
#include "HashlinkOpcode.h"
#include "HashlinkHelper.h"

typedef struct {
	const unsigned char* b;
	int size;
	int pos;
	const char* error;
	hl_code* code;
} hl_reader;

#undef ERROR
#define READ() hl_read_b(r)
#define INDEX() hl_read_index(r)
#define UINDEX() hl_read_uindex(r)
#define ERROR(msg) if( !r->error ) { r->error = msg; hl_debug_break(); }
#define CHK_ERROR() if( r->error ) return

static int setjmp_wrapper(jmp_buf env) {
	return _setjmp(env);
}

static unsigned char hl_read_b(hl_reader* r) {
	if (r->pos >= r->size) {
		ERROR("No more data");
		return 0;
	}
	return r->b[r->pos++];
}

static void hl_read_bytes(hl_reader* r, void* data, int size) {
	if (size < 0) {
		ERROR("Invalid size");
		return;
	}
	if (r->pos + size > r->size) {
		ERROR("No more data");
		return;
	}
	memcpy(data, r->b + r->pos, size);
	r->pos += size;
}

static double hl_read_double(hl_reader* r) {
	double d = 0.;
	hl_read_bytes(r, &d, 8);
	return d;
}

static int hl_read_i32(hl_reader* r) {
	unsigned char a, b, c, d;
	if (r->pos + 4 > r->size) {
		ERROR("No more data");
		return 0;
	}
	a = r->b[r->pos++];
	b = r->b[r->pos++];
	c = r->b[r->pos++];
	d = r->b[r->pos++];
	return a | (b << 8) | (c << 16) | (d << 24);
}

static int hl_read_index(hl_reader* r) {
	unsigned char b = READ();
	if ((b & 0x80) == 0)
		return b & 0x7F;
	if ((b & 0x40) == 0) {
		int v = READ() | ((b & 31) << 8);
		return (b & 0x20) == 0 ? v : -v;
	}
	{
		int c = READ();
		int d = READ();
		int e = READ();
		int v = ((b & 31) << 24) | (c << 16) | (d << 8) | e;
		return (b & 0x20) == 0 ? v : -v;
	}
}

static int hl_read_uindex(hl_reader* r) {
	int i = hl_read_index(r);
	if (i < 0) {
		ERROR("Negative index");
		return 0;
	}
	return i;
}

static hl_type* hl_get_type(hl_reader* r) {
	int i = INDEX();
	if (i < 0 || i >= r->code->ntypes) {
		ERROR("Invalid type index");
		i = 0;
	}
	return r->code->types + i;
}

static const char* hl_read_string(hl_reader* r) {
	int i = INDEX();
	if (i < 0 || i >= r->code->nstrings) {
		ERROR("Invalid string index");
		return "";
	}
	return r->code->strings[i];
}

const uchar* hl_get_ustring(hl_code* code, int index) {
	uchar* str = code->ustrings[index];
	if (str == NULL) {
		int size = hl_utf8_length((vbyte*)code->strings[index], 0);
        str = (uchar*)hl_malloc(&code->alloc, (size + 1) << 1);
		hl_from_utf8(str, size, code->strings[index]);
		code->ustrings[index] = str;
	}
	return str;
}

static const uchar* hl_read_ustring(hl_reader* r) {
	int i = INDEX();
	if (i < 0 || i >= r->code->nstrings) {
		ERROR("Invalid string index");
		i = 0;
	}
	return hl_get_ustring(r->code, i);
}

static void hl_read_type(hl_reader* r, hl_type* t) {
    // Cast the result of READ() to hl_type_kind to ensure type compatibility  
    t->kind = static_cast<hl_type_kind>(READ());
	switch ((int)t->kind) {
	case HFUN:
	case HMETHOD:
	{
		int i;
		int nargs = READ();
		t->fun = (hl_type_fun*)hl_zalloc(&r->code->alloc, sizeof(hl_type_fun));
		t->fun->nargs = nargs;
		t->fun->args = (hl_type**)hl_malloc(&r->code->alloc, sizeof(hl_type*) * nargs);
		for (i = 0;i < nargs;i++)
			t->fun->args[i] = hl_get_type(r);
		t->fun->ret = hl_get_type(r);
	}
	break;
	case HOBJ:
	case HSTRUCT:
	{
		int i;
		const uchar* name = hl_read_ustring(r);
		int super = INDEX();
		int global = UINDEX();
		int nfields = UINDEX();
		int nproto = UINDEX();
		int nbindings = UINDEX();
		t->obj = (hl_type_obj*)hl_malloc(&r->code->alloc, sizeof(hl_type_obj));
		t->obj->name = name;
		t->obj->super = super < 0 ? NULL : r->code->types + super;
		t->obj->global_value = (void**)(int_val)global;
		t->obj->nfields = nfields;
		t->obj->nproto = nproto;
		t->obj->nbindings = nbindings;
		t->obj->fields = (hl_obj_field*)hl_malloc(&r->code->alloc, sizeof(hl_obj_field) * nfields);
		t->obj->proto = (hl_obj_proto*)hl_malloc(&r->code->alloc, sizeof(hl_obj_proto) * nproto);
		t->obj->bindings = (int*)hl_malloc(&r->code->alloc, sizeof(int) * nbindings * 2);
		t->obj->rt = NULL;
		for (i = 0;i < nfields;i++) {
			hl_obj_field* f = t->obj->fields + i;
			f->name = hl_read_ustring(r);
			f->hashed_name = hl_hash_gen(f->name, true);
			f->t = hl_get_type(r);
		}
		for (i = 0;i < nproto;i++) {
			hl_obj_proto* p = t->obj->proto + i;
			p->name = hl_read_ustring(r);
			p->hashed_name = hl_hash_gen(p->name, true);
			p->findex = UINDEX();
			p->pindex = INDEX();
		}
		for (i = 0;i < nbindings;i++) {
			t->obj->bindings[i << 1] = UINDEX();
			t->obj->bindings[(i << 1) | 1] = UINDEX();
		}
	}
	break;
	case HREF:
		t->tparam = hl_get_type(r);
		break;
	case HVIRTUAL:
	{
		int i;
		int nfields = UINDEX();
		t->virt = (hl_type_virtual*)hl_malloc(&r->code->alloc, sizeof(hl_type_virtual));
		t->virt->nfields = nfields;
		t->virt->fields = (hl_obj_field*)hl_malloc(&r->code->alloc, sizeof(hl_obj_field) * nfields);
		for (i = 0;i < nfields;i++) {
			hl_obj_field* f = t->virt->fields + i;
			f->name = hl_read_ustring(r);
			f->hashed_name = hl_hash_gen(f->name, true);
			f->t = hl_get_type(r);
		}
	}
	break;
	case HABSTRACT:
		t->abs_name = hl_read_ustring(r);
		break;
	case HENUM:
	{
		int i, j;
        t->tenum = (hl_type_enum*)hl_malloc(&r->code->alloc, sizeof(hl_type_enum));
		t->tenum->name = hl_read_ustring(r);
		t->tenum->global_value = (void**)(int_val)UINDEX();
		t->tenum->nconstructs = UINDEX();
		t->tenum->constructs = (hl_enum_construct*)hl_malloc(&r->code->alloc, sizeof(hl_enum_construct) * t->tenum->nconstructs);
		for (i = 0;i < t->tenum->nconstructs;i++) {
			hl_enum_construct* c = t->tenum->constructs + i;
			c->name = hl_read_ustring(r);
			c->nparams = UINDEX();
			c->params = (hl_type**)hl_malloc(&r->code->alloc, sizeof(hl_type*) * c->nparams);
			c->offsets = (int*)hl_malloc(&r->code->alloc, sizeof(int) * c->nparams);
			for (j = 0;j < c->nparams;j++)
				c->params[j] = hl_get_type(r);
		}
	}
	break;
	case HNULL:
	case HPACKED:
		t->tparam = hl_get_type(r);
		break;
	default:
		if (t->kind >= HLAST) ERROR("Invalid type");
		break;
	}
}

static void hl_read_opcode(hl_reader* r, hl_function* f, hl_opcode* o) {
	o->op = (hl_op)READ();
	if (o->op >= OLast) {
		ERROR("Invalid opcode");
		return;
	}
	switch (hl_op_nargs[o->op]) {
	case 0:
		break;
	case 1:
		o->p1 = INDEX();
		break;
	case 2:
		o->p1 = INDEX();
		o->p2 = INDEX();
		break;
	case 3:
		o->p1 = INDEX();
		o->p2 = INDEX();
		o->p3 = INDEX();
		break;
	case 4:
		o->p1 = INDEX();
		o->p2 = INDEX();
		o->p3 = INDEX();
		o->extra = (int*)(int_val)INDEX();
		break;
	case -1:
		switch (o->op) {
		case OCallN:
		case OCallClosure:
		case OCallMethod:
		case OCallThis:
		case OMakeEnum:
		{
			int i;
			o->p1 = INDEX();
			o->p2 = INDEX();
			o->p3 = READ();
			o->extra = (int*)hl_malloc(&r->code->falloc, sizeof(int) * o->p3);
			for (i = 0;i < o->p3;i++)
				o->extra[i] = INDEX();
		}
		break;
		case OSwitch:
		{
			int i;
			o->p1 = UINDEX();
			o->p2 = UINDEX();
			o->extra = (int*)hl_malloc(&r->code->falloc, sizeof(int) * o->p2);
			for (i = 0;i < o->p2;i++)
				o->extra[i] = UINDEX();
			o->p3 = UINDEX();
		}
		break;
		default:
			ERROR("Don't know how to process opcode");
			break;
		}
		break;
	default:
	{
		int i, size = hl_op_nargs[o->op] - 3;
		o->p1 = INDEX();
		o->p2 = INDEX();
		o->p3 = INDEX();
		o->extra = (int*)hl_malloc(&r->code->falloc, sizeof(int) * size);
		for (i = 0;i < size;i++)
			o->extra[i] = INDEX();
	}
	break;
	}
}

static void hl_read_function(hl_reader* r, hl_function* f) {
	int i;
	f->type = hl_get_type(r);
	f->findex = UINDEX();
	f->nregs = UINDEX();
	f->nops = UINDEX();
	f->regs = (hl_type**)hl_malloc(&r->code->falloc, f->nregs * sizeof(hl_type*));
	for (i = 0;i < f->nregs;i++)
		f->regs[i] = hl_get_type(r);
	CHK_ERROR();
	f->ops = (hl_opcode*)hl_malloc(&r->code->falloc, f->nops * sizeof(hl_opcode));
	for (i = 0;i < f->nops;i++)
		hl_read_opcode(r, f, f->ops + i);
}

#undef CHK_ERROR
#define CHK_ERROR() if( r->error ) { if( c ) hl_free(&c->alloc); *error_msg = (char*)r->error; return NULL; }
#define EXIT(msg) { ERROR(msg); CHK_ERROR(); }
#define ALLOC(v,ptr,count) v = (ptr *)hl_zalloc(&c->alloc,(count)*sizeof(ptr))

const char* hl_op_name(int op) {
	if (op < 0 || op >= OLast)
		return "UnknownOp";
	return hl_op_names[op];
}

static char** hl_read_strings(hl_reader* r, int nstrings, int** out_lens) {
	int size = hl_read_i32(r);
	hl_code* c = r->code;
	char* sbase = (char*)hl_malloc(&c->alloc, sizeof(char) * size);
	char* sdata = sbase;
	char** strings;
	int* lens;
	int i;
	hl_read_bytes(r, sdata, size);
	ALLOC(strings, char*, nstrings);
	ALLOC(lens, int, nstrings);
	for (i = 0;i < nstrings;i++) {
		int sz = UINDEX();
		strings[i] = sdata;
		lens[i] = sz;
		sdata += sz;
		if (sdata >= sbase + size || *sdata) {
			ERROR("Invalid string");
			return NULL;
		}
		sdata++;
	}
	*out_lens = lens;
	return strings;
}

static int* hl_read_debug_infos(hl_reader* r, int nops) {
	int curfile = -1, curline = 0;
	hl_code* code = r->code;
	int* debug = (int*)hl_malloc(&code->alloc, sizeof(int) * nops * 2);
	int i = 0;
	while (i < nops) {
		int c = READ();
		if (c & 1) {
			c >>= 1;
			curfile = (c << 8) | READ();
			if (curfile >= code->ndebugfiles)
				ERROR("Invalid debug file");
		}
		else if (c & 2) {
			int delta = c >> 6;
			int count = (c >> 2) & 15;
			if (i + count > nops)
				ERROR("Outside range");
			while (count--) {
				debug[i << 1] = curfile;
				debug[(i << 1) | 1] = curline;
				i++;
			}
			curline += delta;
		}
		else if (c & 4) {
			curline += c >> 3;
			debug[i << 1] = curfile;
			debug[(i << 1) | 1] = curline;
			i++;
		}
		else {
			unsigned char b2 = READ();
			unsigned char b3 = READ();
			curline = (c >> 3) | (b2 << 5) | (b3 << 13);
			debug[i << 1] = curfile;
			debug[(i << 1) | 1] = curline;
			i++;
		}
	}
	return debug;
}

hl_code* hl_code_read(const unsigned char* data, int size, char** error_msg) {
	hl_reader _r = { data, size, 0, 0, NULL };
	hl_reader* r = &_r;
	hl_code* c;
	hl_alloc alloc;
	int i;
	int flags;
	int max_version = 5;
	hl_alloc_init(&alloc);
	c = static_cast<hl_code*>(hl_zalloc(&alloc, sizeof(hl_code)));
	c->alloc = alloc;
	hl_alloc_init(&c->falloc);
	if (READ() != 'H' || READ() != 'L' || READ() != 'B')
		EXIT("Invalid HL bytecode header");
	r->code = reinterpret_cast<hl_code*>(c);
	c->version = READ();
	if (c->version <= 1 || c->version > max_version) {
		printf("Found version %d while HL %d.%d supports up to %d\n", c->version, HL_VERSION >> 16, (HL_VERSION >> 8) & 0xFF, max_version);
		EXIT("Unsupported bytecode version");
	}
	flags = UINDEX();
	c->nints = UINDEX();
	c->nfloats = UINDEX();
	c->nstrings = UINDEX();
	if (c->version >= 5)
		c->nbytes = UINDEX();
	c->ntypes = UINDEX();
	c->nglobals = UINDEX();
	c->nnatives = UINDEX();
	c->nfunctions = UINDEX();
	c->nconstants = c->version >= 4 ? UINDEX() : 0;
	c->entrypoint = UINDEX();
	c->hasdebug = flags & 1;
	CHK_ERROR();
	ALLOC(c->ints, int, c->nints);
	for (i = 0;i < c->nints;i++)
		c->ints[i] = hl_read_i32(r);
	CHK_ERROR();
	ALLOC(c->floats, double, c->nfloats);
	for (i = 0;i < c->nfloats;i++)
		c->floats[i] = hl_read_double(r);
	CHK_ERROR();
	c->strings = hl_read_strings(r, c->nstrings, &c->strings_lens);
	ALLOC(c->ustrings, uchar*, c->nstrings);
	CHK_ERROR();
	if (c->version >= 5) {
		int size = hl_read_i32(r);
		c->bytes = static_cast<char*>(hl_malloc(&c->alloc, size));
		hl_read_bytes(r, c->bytes, size);
		ALLOC(c->bytes_pos, int, c->nbytes);
		CHK_ERROR();
		for (i = 0;i < c->nbytes;i++)
			c->bytes_pos[i] = UINDEX();
		CHK_ERROR();
	}
	if (c->hasdebug) {
		c->ndebugfiles = UINDEX();
		c->debugfiles = hl_read_strings(r, c->ndebugfiles, &c->debugfiles_lens);
		CHK_ERROR();
	}
	ALLOC(c->types, hl_type, c->ntypes);
	for (i = 0;i < c->ntypes;i++) {
		hl_read_type(r, c->types + i);
		CHK_ERROR();
	}
	ALLOC(c->globals, hl_type*, c->nglobals);
	for (i = 0;i < c->nglobals;i++)
		c->globals[i] = hl_get_type(r);
	CHK_ERROR();
	ALLOC(c->natives, hl_native, c->nnatives);
	for (i = 0;i < c->nnatives;i++) {
		hl_native* n = c->natives + i;
		n->lib = hl_read_string(r);
		n->name = hl_read_string(r);
		n->t = hl_get_type(r);
		n->findex = UINDEX();
	}
	CHK_ERROR();
	ALLOC(c->functions, hl_function, c->nfunctions);
	for (i = 0;i < c->nfunctions;i++) {
		hl_read_function(r, c->functions + i);
		CHK_ERROR();
		if (c->hasdebug) {
			c->functions[i].debug = hl_read_debug_infos(r, c->functions[i].nops);
			if (c->version >= 3) {
				// skip assigns (no need here)
				int nassigns = UINDEX();
				int j;
				for (j = 0;j < nassigns;j++) {
					UINDEX();
					INDEX();
				}
			}
		}
	}
	CHK_ERROR();
	ALLOC(c->constants, hl_constant, c->nconstants);
	for (i = 0; i < c->nconstants; i++) {
		int j;
		hl_constant* k = c->constants + i;
		k->global = UINDEX();
		k->nfields = UINDEX();
		ALLOC(k->fields, int, k->nfields);
		for (j = 0; j < k->nfields; j++)
			k->fields[j] = UINDEX();
		CHK_ERROR();
	}
	return c;
}

void hl_code_free(hl_code* c) {
	hl_free(&c->falloc);
}

static const unsigned int crc32_table[] =
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

#define H(b) hash = (hash >> 8) ^ crc32_table[(hash ^ (b)) & 0xFF]
#define H32(i) { H(i&0xFF); H((i>>8)&0xFF); H((i>>16)&0xFF); H(((unsigned int)i)>>24); }
#define HFUN(idx) H32(h->functions_signs[h->functions_indexes[idx]]);
#define HSTR(s) { const char *_c = s; while( *_c ) H(*_c++); }
#define HUSTR(s) { const uchar *_c = s; while( *_c ) H(*_c++); }
#define HTYPE(t) if( !isrec ) H32(hash_type_first(t,true))

// hash with only partial recursion
static int hash_type_first(hl_type* t, bool isrec) {
	int hash = -1;
	int i;
	H(t->kind);
	switch (t->kind) {
	case HFUN:
	case HMETHOD:
		H(t->fun->nargs);
		for (i = 0;i < t->fun->nargs;i++)
			HTYPE(t->fun->args[i]);
		HTYPE(t->fun->ret);
		break;
	case HOBJ:
	case HSTRUCT:
		HUSTR(t->obj->name);
		H32(t->obj->nfields);
		H32(t->obj->nproto);
		for (i = 0;i < t->obj->nfields;i++) {
			hl_obj_field* f = t->obj->fields + i;
			H32(f->hashed_name);
			HTYPE(f->t);
		}
		break;
	case HREF:
	case HNULL:
		HTYPE(t->tparam);
		break;
	case HVIRTUAL:
		H32(t->virt->nfields);
		for (i = 0;i < t->virt->nfields;i++) {
			hl_obj_field* f = t->virt->fields + i;
			H32(f->hashed_name);
			HTYPE(f->t);
		}
		break;
	case HENUM:
		HUSTR(t->tenum->name);
		for (i = 0;i < t->tenum->nconstructs;i++) {
			hl_enum_construct* c = t->tenum->constructs + i;
			int k;
			H(c->nparams);
			HUSTR(c->name);
			for (k = 0;k < c->nparams;k++)
				HTYPE(c->params[k]);
		}
		break;
	case HABSTRACT:
		HUSTR(t->abs_name);
		break;
	default:
		break;
	}
	return hash;
}

#undef HTYPE
#define HTYPE(t) H32(h->types_hashes[t - h->code->types])

static int hash_type_rec(hl_code_hash* h, hl_type* t) {
	int hash = -1;
	int i;
	switch (t->kind) {
	case HFUN:
	case HMETHOD:
		for (i = 0;i < t->fun->nargs;i++)
			HTYPE(t->fun->args[i]);
		HTYPE(t->fun->ret);
		break;
	case HOBJ:
	case HSTRUCT:
		for (i = 0;i < t->obj->nfields;i++) {
			hl_obj_field* f = t->obj->fields + i;
			HTYPE(f->t);
		}
		break;
	case HREF:
	case HNULL:
		HTYPE(t->tparam);
		break;
	case HVIRTUAL:
		for (i = 0;i < t->virt->nfields;i++) {
			hl_obj_field* f = t->virt->fields + i;
			HTYPE(f->t);
		}
		break;
	case HENUM:
		for (i = 0;i < t->tenum->nconstructs;i++) {
			hl_enum_construct* c = t->tenum->constructs + i;
			int k;
			for (k = 0;k < c->nparams;k++)
				HTYPE(c->params[k]);
		}
		break;
	default:
		break;
	}
	return hash;
}

static int hash_native(hl_code_hash* h, hl_native* n) {
	int hash = -1;
	HSTR(n->lib);
	HSTR(n->name);
	HTYPE(n->t);
	return hash;
}

static int hash_fun_sign(hl_code_hash* h, hl_function* f) {
	int hash = -1;
	HTYPE(f->type);
	if (f->obj) {
		HUSTR(f->obj->name);
		HUSTR(f->field.name);
	}
	else if (f->field.ref) {
		HUSTR(f->field.ref->obj->name);
		HUSTR(f->field.ref->field.name);
		H32(f->ref);
	}
	return hash;
}

static int hash_fun(hl_code_hash* h, hl_function* f) {
	int hash = -1;
	hl_code* c = h->code;
	int i, k;
	for (i = 0;i < f->nregs;i++)
		HTYPE(f->regs[i]);
	for (k = 0;k < f->nops;k++) {
		hl_opcode* o = f->ops + k;
		H(o->op);
		switch (o->op) {
		case OInt:
			H32(o->p1);
			H32(c->ints[o->p2]);
			break;
		case OFloat:
			H32(o->p1);
			H32(((int*)c->floats)[o->p2 << 1]);
			H32(((int*)c->floats)[(o->p2 << 1) | 1]);
			break;
		case OString:
			H32(o->p1);
			HSTR(c->strings[o->p2]);
			break;
			//case OBytes:
		case OType:
			H32(o->p1);
			HTYPE(c->types + o->p2);
			break;
		case OCall0:
			H32(o->p1);
			HFUN(o->p2);
			break;
		case OCall1:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			break;
		case OCall2:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			H32((int)(int_val)o->extra);
			break;
		case OCall3:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			H32(o->extra[0]);
			H32(o->extra[1]);
			break;
		case OCall4:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			H32(o->extra[0]);
			H32(o->extra[1]);
			H32(o->extra[2]);
			break;
		case OCallN:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			for (i = 0;i < o->p3;i++)
				H32(o->extra[i]);
			break;
		case OStaticClosure:
			H32(o->p1);
			HFUN(o->p2);
			break;
		case OInstanceClosure:
			H32(o->p1);
			HFUN(o->p2);
			H32(o->p3);
			break;
		case ODynGet:
			H32(o->p1);
			H32(o->p2);
			HSTR(c->strings[o->p3]);
			break;
		case ODynSet:
			H32(o->p1);
			HSTR(c->strings[o->p2]);
			H32(o->p3);
			break;
		default:
			switch (hl_op_nargs[o->op]) {
			case 0:
				break;
			case 1:
				H32(o->p1);
				break;
			case 2:
				H32(o->p1);
				H32(o->p2);
				break;
			case 3:
				H32(o->p1);
				H32(o->p2);
				H32(o->p3);
				break;
			case 4:
				H32(o->p1);
				H32(o->p2);
				H32(o->p3);
				H32((int)(int_val)o->extra);
				break;
			case -1:
				switch (o->op) {
				case OCallN:
				case OCallClosure:
				case OCallMethod:
				case OCallThis:
				case OMakeEnum:
					H32(o->p1);
					H32(o->p2);
					H32(o->p3);
					for (i = 0;i < o->p3;i++)
						H32(o->extra[i]);
					break;
				case OSwitch:
					H32(o->p1);
					H32(o->p2);
					for (i = 0;i < o->p2;i++)
						H32(o->extra[i]);
					H32(o->p3);
					break;
				default:
					printf("Don't know how to process opcode %d", o->op);
					break;
				}
				break;
			default:
			{
				int size = hl_op_nargs[o->op] - 3;
				H32(o->p1);
				H32(o->p2);
				H32(o->p3);
				for (i = 0;i < size;i++)
					H32(o->extra[i]);
			}
			break;
			}
		}
	}
	return hash;
}

int hl_code_hash_type(hl_code_hash* h, hl_type* t) {
	int hash = -1;
	HTYPE(t);
	return hash;
}

hl_code_hash* hl_code_hash_alloc(hl_code* c) {
	int i;
    hl_code_hash* h = (hl_code_hash*)malloc(sizeof(hl_code_hash));
	memset(h, 0, sizeof(hl_code_hash));
	h->code = c;

	h->functions_indexes = static_cast<int*>(malloc(sizeof(int) * (c->nfunctions + c->nnatives)));
	for (i = 0;i < c->nfunctions;i++) {
		hl_function* f = c->functions + i;
		h->functions_indexes[f->findex] = i;
	}
	for (i = 0;i < c->nnatives;i++) {
		hl_native* n = c->natives + i;
		h->functions_indexes[n->findex] = i + c->nfunctions;
	}

	h->types_hashes = static_cast<int*>(malloc(sizeof(int) * c->ntypes));
	for (i = 0;i < c->ntypes;i++)
		h->types_hashes[i] = hash_type_first(c->types + i, false);
	int* types_hashes = static_cast<int*>(malloc(sizeof(int) * c->ntypes)); // use a second buffer for order-indepedent
	for (i = 0;i < c->ntypes;i++)
		types_hashes[i] = h->types_hashes[i] ^ hash_type_rec(h, c->types + i);
	free(h->types_hashes);
	h->types_hashes = types_hashes;

	h->globals_signs = static_cast<int*>(malloc(sizeof(int) * c->nglobals));
	for (i = 0;i < c->nglobals;i++) {
		hl_type* t = c->globals[i];
		h->globals_signs[i] = i | 0x80000000;
		if (t->kind == HABSTRACT)
			h->globals_signs[i] = hl_code_hash_type(h, t); // some global abstracts allocated by compiler
	}
	for (i = 0;i < c->ntypes;i++) {
		hl_type* t = c->types + i;
		switch (t->kind) {
		case HOBJ:
		case HSTRUCT:
			if (t->obj->global_value)
				h->globals_signs[(int)(int_val)t->obj->global_value - 1] = hl_code_hash_type(h, t);
			break;
		case HENUM:
			if (t->tenum->global_value)
				h->globals_signs[(int)(int_val)t->tenum->global_value - 1] = hl_code_hash_type(h, t);
			break;
		default:
			break;
		}
	}
	for (i = 0;i < c->nconstants;i++) {
		hl_constant* k = c->constants + i;
		hl_type* t = c->globals[k->global];
		int hash = -1;
		int j;
		for (j = 0;j < k->nfields;j++) {
			int index = k->fields[j];
			switch (t->obj->fields[j].t->kind) {
			case HI32:
				H32(c->ints[index]);
				break;
			case HBYTES:
				HSTR(c->strings[index]);
				break;
			default:
				break;
			}
		}
		h->globals_signs[k->global] = hash;
	}

	// look into boot code to identify globals that are constant enum constructors
	// this is a bit hackish but we need them for remap and there's no metatada
	hl_function* f = c->functions + h->functions_indexes[c->entrypoint];
	for (i = 4;i < f->nops;i++) {
		hl_opcode* op = f->ops + i;
		hl_type* t;
		switch (op->op) {
		case OSetGlobal:
			t = c->globals[op->p1];
			if (t->kind == HENUM && f->ops[i - 2].op == OGetArray && f->ops[i - 3].op == OInt)
				h->globals_signs[op->p1] = c->ints[f->ops[i - 3].p2];
			break;
		default:
			break;
		}
	}

	for (i = 0;i < c->nglobals;i++)
		h->globals_signs[i] ^= hl_code_hash_type(h, c->globals[i]);
	return h;
}


void hl_code_hash_remap_globals(hl_code_hash* hnew, hl_code_hash* hold) {
	hl_code* c = hnew->code;
	int i;
	int old_start = 0;

	int count = c->nglobals;
	int old_count = hold->code->nglobals;
	int extra = old_count - count;
	if (extra < 0) extra = 0;
	int* remap = static_cast<int*>(malloc(sizeof(int) * count));

	for (i = 0;i < count;i++) {
		int k;
		int h = hnew->globals_signs[i];
		remap[i] = -1;
		for (k = old_start;k < old_count;k++) {
			if (hold->globals_signs[k] == h) {
				if (k == old_start) old_start++;
				remap[i] = k;
				break;
			}
		}
	}

	// new globals
	for (i = 0;i < count;i++)
		if (remap[i] == -1)
			remap[i] = old_count + extra++;

	hl_type** nglobals;
	int new_count = old_count + extra;
	ALLOC(nglobals, hl_type*, new_count);
	for (i = 0;i < new_count;i++)
		nglobals[i] = i < old_count ? hold->code->globals[i] : &hlt_void;
	for (i = 0;i < count;i++)
		nglobals[remap[i]] = c->globals[i];
	c->globals = nglobals;
	c->nglobals = new_count;

#	ifdef HL_DEBUG
	for (i = old_count;i < new_count;i++) {
		hl_type* t = c->globals[i];
		uprintf(USTR("New global %s\n"), hl_type_str(t));
	}
#	endif

	int* nsigns = static_cast<int*>(malloc(sizeof(int) * c->nglobals));
	for (i = 0;i < new_count;i++)
		nsigns[i] = i < old_count ? hold->globals_signs[i] : -1;
	for (i = 0;i < count;i++)
		nsigns[remap[i]] = hnew->globals_signs[i];
	free(hnew->globals_signs);
	hnew->globals_signs = nsigns;

	for (i = 0;i < c->ntypes;i++) {
		hl_type* t = c->types + i;
		switch (t->kind) {
		case HSTRUCT:
		case HOBJ:
			if (t->obj->global_value)
				t->obj->global_value = reinterpret_cast<void**>((int_val)(remap[(int)(int_val)t->obj->global_value - 1] + 1));
			break;
		case HENUM:
			if (t->tenum->global_value)
				t->tenum->global_value = reinterpret_cast<void**>((int_val)(remap[(int)(int_val)t->tenum->global_value - 1] + 1));
			break;
		default:
			break;
		}
	}
	for (i = 0;i < c->nconstants;i++)
		c->constants[i].global = remap[c->constants[i].global];

	for (i = 0;i < c->nfunctions;i++) {
		hl_function* f = c->functions + i;
		int k;
		for (k = 0;k < f->nops;k++) {
			hl_opcode* op = f->ops + k;
			switch (op->op) {
			case OGetGlobal:
				op->p2 = remap[op->p2];
				break;
			case OSetGlobal:
				op->p1 = remap[op->p1];
				break;
			default:
				break;
			}
		}
	}

	free(remap);
}

void hl_code_hash_finalize(hl_code_hash* h) {
	hl_code* c = h->code;
	int i;
	h->functions_signs = static_cast<int*>(malloc(sizeof(int) * (c->nfunctions + c->nnatives)));
	for (i = 0;i < c->nfunctions;i++) {
		hl_function* f = c->functions + i;
		h->functions_signs[i] = hash_fun_sign(h, f);
	}
	for (i = 0;i < c->nnatives;i++) {
		hl_native* n = c->natives + i;
		h->functions_signs[i + c->nfunctions] = hash_native(h, n);
	}
	h->functions_hashes = static_cast<int*>(malloc(sizeof(int) * c->nfunctions));
	for (i = 0;i < c->nfunctions;i++) {
		hl_function* f = c->functions + i;
		h->functions_hashes[i] = hash_fun(h, f);
	}
}

void hl_code_hash_free(hl_code_hash* h) {
	free(h->functions_hashes);
	free(h->functions_indexes);
	free(h->functions_signs);
	free(h->globals_signs);
	free(h->types_hashes);
	free(h);
}

#ifdef HL_WIN
#	include <windows.h>
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#	define dlopen(l,p)		(void*)( (l) ? LoadLibraryA(l) : (HMODULE)&__ImageBase)
#	define dlsym(h,n)		GetProcAddress((HMODULE)h,n)
#else
#	include <dlfcn.h>
#endif

#define HOT_RELOAD_EXTRA_GLOBALS	4096

void hl_prim_not_loaded() {
	hl_error("Primitive or library is missing");
}

static hl_module** cur_modules = NULL;
static int modules_count = 0;

static bool module_resolve_pos(hl_module* m, void* addr, int* fidx, int* fpos) {
	int code_pos = ((int)(int_val)((unsigned char*)addr - (unsigned char*)m->jit_code));
	int min, max;
	hl_debug_infos* dbg;
	hl_function* fdebug;
	if (m->jit_debug == NULL)
		return false;
	// lookup function from code pos
	min = 0;
	max = m->code->nfunctions;
	while (min < max) {
		int mid = (min + max) >> 1;
		hl_debug_infos* p = m->jit_debug + mid;
		if (p->start <= code_pos)
			min = mid + 1;
		else
			max = mid;
	}
	if (min == 0)
		return false; // hl_callback
	do {
		min--;
		*fidx = min;
		dbg = m->jit_debug + min;
		fdebug = m->code->functions + min;
	} while (!dbg->offsets);
	// lookup inside function
	min = 0;
	max = fdebug->nops;
	code_pos -= dbg->start;
	while (min < max) {
		int mid = (min + max) >> 1;
		int offset = dbg->large ? ((int*)dbg->offsets)[mid] : ((unsigned short*)dbg->offsets)[mid];
		if (offset <= code_pos)
			min = mid + 1;
		else
			max = mid;
	}
	if (min == 0)
		return false; // ???
	*fpos = min - 1;
	return true;
}

uchar* hl_module_resolve_symbol_full(void* addr, uchar* out, int* outSize, int** r_debug_addr) {
	int* debug_addr;
	int file, line;
	int pos = 0;
	int fidx, fpos;
	hl_function* fdebug;
	int i;
	hl_module* m = NULL;
	for (i = 0;i < modules_count;i++) {
		m = reinterpret_cast<hl_module*>(cur_modules[i]);
		if (addr >= m->jit_code && addr <= (void*)((char*)m->jit_code + m->codesize)) break;
	}
	if (i == modules_count)
		return NULL;
	if (!module_resolve_pos(m, addr, &fidx, &fpos))
		return NULL;
	// extract debug info
	fdebug = m->code->functions + fidx;
	debug_addr = fdebug->debug + ((fpos & 0xFFFF) * 2);
	file = debug_addr[0];
	line = debug_addr[1];
	if (r_debug_addr) {
		*r_debug_addr = debug_addr;
		if (file < 0) return NULL; // already cached
	}
	if (!out)
		return NULL;
	int size = *outSize;
	if (fdebug->obj)
		pos += usprintf(out, size - pos, USTR("%s.%s("), fdebug->obj->name, fdebug->field.name);
	else if (fdebug->field.ref)
		pos += usprintf(out, size - pos, USTR("%s.~%s.%d("), fdebug->field.ref->obj->name, fdebug->field.ref->field.name, fdebug->ref);
	else
		pos += usprintf(out, size - pos, USTR("fun$%d("), fdebug->findex);
	pos += hl_from_utf8(out + pos, size - pos, m->code->debugfiles[file & 0x7FFFFFFF]);
	pos += usprintf(out + pos, size - pos, USTR(":%d)"), line);
	*outSize = pos;
	return out;
}

static uchar* module_resolve_symbol(void* addr, uchar* out, int* outSize) {
	return hl_module_resolve_symbol_full(addr, out, outSize, NULL);
}

int hl_module_capture_stack_range(void* stack_top, void** stack_ptr, void** out, int size) {
#if defined(HL_64) && defined(HL_WIN)
#else
	void* stack_bottom = stack_ptr;
#endif
	int count = 0;
	if (modules_count == 1) {
		hl_module* m = reinterpret_cast<hl_module*>(cur_modules[0]);
		unsigned char* code = static_cast<unsigned char*>(m->jit_code);
		int code_size = m->codesize;
		if (m->jit_debug) {
			int s = m->jit_debug[0].start;
			code += s;
			code_size -= s;
		}
		while (stack_ptr < (void**)stack_top) {
#if defined(HL_64) && defined(HL_WIN)
			void* module_addr = *stack_ptr++; // EIP
			if (module_addr >= (void*)code && module_addr < (void*)(code + code_size)) {
				if (out) {
					if (count == size) break;
					out[count++] = module_addr;
				}
				else
					count++;
			}
#else
			void* stack_addr = *stack_ptr++; // EBP
			if (stack_addr > stack_bottom && stack_addr < stack_top) {
				void* module_addr = *stack_ptr; // EIP
				if (module_addr >= (void*)code && module_addr < (void*)(code + code_size)) {
					if (out) {
						if (count == size) break;
						out[count++] = module_addr;
					}
					else {
						count++;
					}
				}
			}
#endif
		}
	}
	else {
		while (stack_ptr < (void**)stack_top) {
#if defined(HL_64) && defined(HL_WIN)
			void* module_addr = *stack_ptr++; // EIP
			{
#else
			void* stack_addr = *stack_ptr++; // EBP
			if (stack_addr > stack_bottom && stack_addr < stack_top) {
				void* module_addr = *stack_ptr; // EIP
#endif
				int i;
				for (i = 0;i < modules_count;i++) {
					hl_module* m = reinterpret_cast<hl_module*>(cur_modules[i]);
					unsigned char* code = static_cast<unsigned char*>(m->jit_code);
					int code_size = m->codesize;
					if (module_addr >= (void*)code && module_addr < (void*)(code + code_size)) {
						if (out && count == size) {
							stack_ptr = reinterpret_cast<void**>(stack_top);
							break;
						}
						if (m->jit_debug) {
							int s = m->jit_debug[0].start;
							code += s;
							code_size -= s;
							if (module_addr < (void*)code || module_addr >= (void*)(code + code_size)) continue;
						}
						if (out)
							out[count++] = module_addr;
						else
							count++;
						break;
					}
				}
			}
			}
		}
	return count;
	}

static int module_capture_stack(void** stack, int size) {
	return hl_module_capture_stack_range(hl_get_thread()->stack_top, (void**)&stack, stack, size);
}

static void hl_module_types_dump(void (*fdump)(void*, int)) {
	int ntypes = 0;
	int i, j, fcount = 0;
	for (i = 0;i < modules_count;i++)
		ntypes += cur_modules[i]->code->ntypes;
	fdump(&ntypes, 4);
	for (i = 0;i < modules_count;i++) {
		hl_module* m = reinterpret_cast<hl_module*>(cur_modules[i]);
		for (j = 0;j < m->code->ntypes;j++) {
			hl_type* t = m->code->types + j;
			fdump(&t, sizeof(void*));
			if (t->kind == HFUN) fcount++;
		}
	}
	fdump(&fcount, 4);
	for (i = 0;i < modules_count;i++) {
		hl_module* m = reinterpret_cast<hl_module*>(cur_modules[i]);
		for (j = 0;j < m->code->ntypes;j++) {
			hl_type* t = m->code->types + j;
			if (t->kind == HFUN) {
				hl_type* ct = (hl_type*)&t->fun->closure_type;
				fdump(&ct, sizeof(void*));
			}
		}
	}
}

hl_module* hl_module_alloc(hl_code * c) {
	int i;
	int gsize = 0;
	hl_module* m = (hl_module*)malloc(sizeof(hl_module));
	if (m == NULL)
		return NULL;
	memset(m, 0, sizeof(hl_module));
	m->code = c;
	m->globals_indexes = (int*)malloc(sizeof(int) * c->nglobals);
	if (m->globals_indexes == NULL) {
		hl_module_free(m);
		return NULL;
	}
	for (i = 0;i < c->nglobals;i++) {
		gsize += hl_pad_size(gsize, c->globals[i]);
		m->globals_indexes[i] = gsize;
		gsize += hl_type_size(c->globals[i]);
	}
	m->globals_size = gsize;
	m->globals_data = (unsigned char*)malloc(gsize);
	if (m->globals_data == NULL) {
		hl_module_free(m);
		return NULL;
	}
	memset(m->globals_data, 0, gsize);
	m->functions_ptrs = (void**)malloc(sizeof(void*) * (c->nfunctions + c->nnatives));
	m->functions_indexes = (int*)malloc(sizeof(int) * (c->nfunctions + c->nnatives));
	m->ctx.functions_types = (hl_type**)malloc(sizeof(void*) * (c->nfunctions + c->nnatives));
	if (m->functions_ptrs == NULL || m->functions_indexes == NULL || m->ctx.functions_types == NULL) {
		hl_module_free(m);
		return NULL;
	}
	memset(m->functions_ptrs, 0, sizeof(void*) * (c->nfunctions + c->nnatives));
	memset(m->functions_indexes, 0xFF, sizeof(int) * (c->nfunctions + c->nnatives));
	memset(m->ctx.functions_types, 0, sizeof(void*) * (c->nfunctions + c->nnatives));
	hl_alloc_init(&m->ctx.alloc);
	m->ctx.functions_ptrs = m->functions_ptrs;
	return m;
}

static void null_function() {
	hl_error("Null function ptr");
}

static void append_fields(char** p, hl_type * t);

static void append_type(char** p, hl_type * t) {
	*(*p)++ = TYPE_STR[t->kind];
	switch (t->kind) {
	case HFUN:
	{
		int i;
		for (i = 0;i < t->fun->nargs;i++)
			append_type(p, t->fun->args[i]);
		*(*p)++ = '_';
		append_type(p, t->fun->ret);
		break;
	}
	case HREF:
	case HNULL:
		append_type(p, t->tparam);
		break;
	case HOBJ:
	{
		append_fields(p, t);
		*(*p)++ = '_';
	}
	break;
	case HABSTRACT:
		*p += utostr(*p, 100, t->abs_name);
		*(*p)++ = '_';
		break;
	default:
		break;
	}
}

static void append_fields(char** p, hl_type * t) {
	int i;
	if (t->obj->super)
		append_fields(p, t->obj->super);
	for (i = 0;i < t->obj->nfields;i++)
		append_type(p, t->obj->fields[i].t);
}

#define DISABLED_LIB_PTR ((void*)(int_val)2)

static void* resolve_library(const char* lib, bool is_opt) {
	char tmp[256];
	void* h;

#	ifndef HL_CONSOLE
	static char* DISABLED_LIBS = NULL;
	if (!DISABLED_LIBS) {
		DISABLED_LIBS = getenv("HL_DISABLED_LIBS");
		if (!DISABLED_LIBS) DISABLED_LIBS = const_cast<char*>("");
	}
	char* disPart = strstr(DISABLED_LIBS, lib);
	if (disPart) {
		disPart += strlen(lib);
		if (*disPart == 0 || *disPart == ',')
			return DISABLED_LIB_PTR;
	}
#	endif

	if (strcmp(lib, "builtin") == 0)
		return dlopen(NULL, RTLD_LAZY);

	if (strcmp(lib, "std") == 0) {
#	ifdef HL_WIN
#		ifdef HL_64
		h = dlopen("libhl64.dll", RTLD_LAZY);
		if (h == NULL) h = dlopen("libhl.dll", RTLD_LAZY);
#		else
		h = dlopen("libhl.dll", RTLD_LAZY);
#		endif
		if (h == NULL && !is_opt) hl_fatal1("Failed to load library %s", "libhl.dll");
		return h;
#	else
		return RTLD_DEFAULT;
#	endif
	}

	strcpy(tmp, lib);

#	ifdef HL_64
	strcpy(tmp + strlen(lib), "64.hdll");
	h = dlopen(tmp, RTLD_LAZY);
	if (h != NULL) return h;
#	endif

	strcpy(tmp + strlen(lib), ".hdll");
	h = dlopen(tmp, RTLD_LAZY);
	if (h == NULL && !is_opt)
		hl_fatal1("Failed to load library %s", tmp);
	return h;
}

static void disabled_primitive() {
	hl_error("This library primitive has been disabled");
}

static void hl_module_init_indexes(hl_module * m) {
	int i;
	for (i = 0;i < m->code->nfunctions;i++) {
		hl_function* f = m->code->functions + i;
		m->functions_indexes[f->findex] = i;
		m->ctx.functions_types[f->findex] = f->type;
	}
	for (i = 0;i < m->code->nnatives;i++) {
		hl_native* n = m->code->natives + i;
		m->functions_indexes[n->findex] = i + m->code->nfunctions;
		m->ctx.functions_types[n->findex] = n->t;
	}
	for (i = 0;i < m->code->ntypes;i++) {
		hl_type* t = m->code->types + i;
		switch (t->kind) {
		case HOBJ:
		case HSTRUCT:
			t->obj->m = &m->ctx;
			t->obj->global_value = ((int)(int_val)t->obj->global_value) ? (void**)(int_val)(m->globals_data + m->globals_indexes[(int)(int_val)t->obj->global_value - 1]) : NULL;
			{
				int j;
				for (j = 0;j < t->obj->nproto;j++) {
					hl_obj_proto* p = t->obj->proto + j;
					hl_function* f = m->code->functions + m->functions_indexes[p->findex];
					f->obj = t->obj;
					f->field.name = p->name;
				}
				for (j = 0;j < t->obj->nbindings;j++) {
					int fid = t->obj->bindings[j << 1];
					int mid = t->obj->bindings[(j << 1) | 1];
					hl_obj_field* of = hl_obj_field_fetch(t, fid);
					switch (of->t->kind) {
					case HFUN:
					case HDYN:
					{
						hl_function* f = m->code->functions + m->functions_indexes[mid];
						f->obj = t->obj;
						f->field.name = of->name;
					}
					break;
					default:
						break;
					}
				}
			}
			break;
		case HENUM:
			hl_init_enum(t, &m->ctx);
			t->tenum->global_value = ((int)(int_val)t->tenum->global_value) ? (void**)(int_val)(m->globals_data + m->globals_indexes[(int)(int_val)t->tenum->global_value - 1]) : NULL;
			break;
		case HVIRTUAL:
			hl_init_virtual(t, &m->ctx);
			break;
		default:
			break;
		}
	}
	for (i = 0;i < m->code->nfunctions;i++) {
		int k;
		hl_function* f = m->code->functions + i;
		hl_function* real_f = f;
		while (real_f && !real_f->obj) real_f = real_f->field.ref;
		if (real_f == NULL) continue;
		for (k = 0;k < f->nops;k++) {
			hl_opcode* op = f->ops + k;
			switch (op->op) {
			case OCall0:
			case OCall1:
			case OCall2:
			case OCall3:
			case OCall4:
			case OCallN:
			case OStaticClosure:
			case OInstanceClosure:
				if (m->functions_indexes[op->p2] < m->code->nfunctions) {
					hl_function* floc = m->code->functions + m->functions_indexes[op->p2];
					if (floc->obj) continue;
					floc->field.ref = real_f;
					floc->ref = real_f->ref++;
				}
				break;
			default:
				break;
			}
		}
	}

	static hl_type_obj obj_entry = { 0 };
	hl_function* fent = m->code->functions + m->functions_indexes[m->code->entrypoint];
	obj_entry.name = USTR("");
	fent->obj = &obj_entry;
	fent->field.name = USTR("init");
}

#ifdef HL_VTUNE
#include <jitprofiling.h>
h_bool hl_module_init_vtune(hl_module * m) {
	int i;
	if (!iJIT_IsProfilingActive() || m->jit_debug == NULL)
		return false;
	for (i = 0;i < m->code->nfunctions;i++) {
		hl_function* f = m->code->functions + i;
		void* faddr = m->functions_ptrs[f->findex];

		iJIT_Method_Load jm = { 0 };
		char out[256];
		jm.method_id = iJIT_GetNewMethodID();
		if (f->obj) {
			jm.class_file_name = hl_to_utf8(f->obj->name);
			jm.method_name = hl_to_utf8(f->field.name);
		}
		else if (f->field.ref) {
			jm.class_file_name = hl_to_utf8(f->field.ref->obj->name);
			jm.method_name = hl_to_utf8(f->field.ref->field.name);
		}
		else {
			sprintf(out, "fun$%d", f->findex);
			jm.method_name = out;
		}
		jm.method_load_address = faddr;
		jm.method_size = 0;
		int j;
		for (j = 0;j < m->code->nfunctions;j++) {
			hl_function* f2 = m->code->functions + j;
			if (f2 == f) continue;
			void* addr = m->functions_ptrs[f2->findex];
			int_val dif = (char*)addr - (char*)faddr;
			if (dif <= 0) continue;
			if (jm.method_size == 0 || dif < jm.method_size) jm.method_size = (int)dif;
		}

		int file = f->debug[0] & 0x7FFFFFFF;
		int curline = -1;
		LineNumberInfo* lines = (LineNumberInfo*)malloc(sizeof(LineNumberInfo) * f->nops);
		int nlines = 0;
		hl_debug_infos* dbg = m->jit_debug + i;
		jm.source_file_name = m->code->debugfiles[file];
		for (j = 0;j < f->nops;j++) {
			int file2 = f->debug[j << 1] & 0x7FFFFFFF;
			int line = f->debug[(j << 1) | 1];
			if (file2 != file || line == curline) continue;
			lines[nlines].Offset = dbg->large ? ((int*)dbg->offsets)[j] : ((unsigned short*)dbg->offsets)[j];
			lines[nlines].LineNumber = line - 1;
			curline = line;
			nlines++;
		}
		if (nlines && jm.method_size) {
			jm.line_number_table = lines;
			jm.line_number_size = nlines;
		}
		iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void*)&jm);
		free(lines);
	}
	return true;
}
#endif

static void hl_module_init_natives(hl_module * m) {
	char tmp[256];
	int i;
	void* libHandler = NULL;
	const char* curlib = NULL, * sign;
	for (i = 0;i < m->code->nnatives;i++) {
		hl_native* n = m->code->natives + i;
		const char* lib = n->lib;
		bool is_opt = *lib == '?';
		char* p = tmp;
		void* f;
		if (is_opt) lib++;
		if (curlib != lib) {
			curlib = lib;
			libHandler = resolve_library(lib, is_opt);
		}
		if (libHandler == DISABLED_LIB_PTR) {
			m->functions_ptrs[n->findex] = disabled_primitive;
			continue;
		}
		strcpy(p, "hlp_");
		p += 4;
		strcpy(p, n->name);
		p += strlen(n->name);
		*p++ = 0;
		f = dlsym(libHandler, tmp);
		if (f == NULL) {
			if (is_opt) {
				m->functions_ptrs[n->findex] = hl_prim_not_loaded;
				continue;
			}
			hl_fatal2("Failed to load function %s@%s", n->lib, n->name);
		}
		m->functions_ptrs[n->findex] = ((void* (*)(const char** p))f)(&sign);
		p = tmp;
		append_type(&p, n->t);
		*p++ = 0;
		if (sign && memcmp(sign, tmp, strlen(sign) + 1) != 0)
			hl_fatal4("Invalid signature for function %s@%s : %s required but %s found in hdll", n->lib, n->name, tmp, sign);
	}
}

static void hl_module_init_constant(hl_module * m, hl_constant * c) {
	hl_type* t = m->code->globals[c->global];
	hl_runtime_obj* rt;
	vdynamic** global = (vdynamic**)(m->globals_data + m->globals_indexes[c->global]);
	vdynamic* v = NULL;
	switch (t->kind) {
	case HOBJ:
	case HSTRUCT:
		rt = hl_get_obj_rt(t);
		v = (vdynamic*)hl_malloc(&m->ctx.alloc, rt->size);
		v->t = t;
		for (int i = 0;i < c->nfields;i++) {
			int idx = c->fields[i];
			hl_type* ft = t->obj->fields[i].t;
			void* addr = (char*)v + rt->fields_indexes[i];
			switch (ft->kind) {
			case HI32:
				*(int*)addr = m->code->ints[idx];
				break;
			case HBOOL:
				*(bool*)addr = idx != 0;
				break;
			case HF64:
				*(double*)addr = m->code->floats[idx];
				break;
			case HBYTES:
				*(const void**)addr = hl_get_ustring(m->code, idx);
				break;
			case HTYPE:
				*(hl_type**)addr = m->code->types + idx;
				break;
			default:
				*(void**)addr = *(void**)(m->globals_data + m->globals_indexes[idx]);
				break;
			}
		}
		break;
	default:
		hl_fatal("assert");
	}
	*global = v;
	hl_remove_root(global);
}

static void hl_module_add(hl_module * m) {
	hl_module** old_modules = reinterpret_cast<hl_module**>(cur_modules);
	hl_module** new_modules = (hl_module**)malloc(sizeof(void*) * (modules_count + 1));
	memcpy(new_modules, old_modules, sizeof(void*) * modules_count);
	new_modules[modules_count] = m;
	cur_modules = new_modules;
	modules_count++;
	free(old_modules);
}

void hl_setup_vtune(void* vtune_init, void* m);
int hl_module_init(hl_module * m, h_bool hot_reload, h_bool vtune_later) {
	int i;
	jit_ctx* ctx;
	// expand globals
	if (hot_reload) {
		int nsize = m->globals_size + HOT_RELOAD_EXTRA_GLOBALS * sizeof(void*);
		int* nindexes = static_cast<int*>(malloc(sizeof(int) * (m->code->nglobals + HOT_RELOAD_EXTRA_GLOBALS)));
		memcpy(nindexes, m->globals_indexes, sizeof(int) * m->code->nglobals);
		memset(nindexes + m->code->nglobals, 0xFF, HOT_RELOAD_EXTRA_GLOBALS * sizeof(int));
		free(m->globals_indexes);
		free(m->globals_data);
		m->globals_indexes = nindexes;
		m->globals_data = static_cast<unsigned char*>(malloc(nsize));
		memset(m->globals_data, 0, m->globals_size);
		memset(m->globals_data + m->globals_size, 0xFF, HOT_RELOAD_EXTRA_GLOBALS * sizeof(void*));
	}
	// RESET globals
	for (i = 0;i < m->code->nglobals;i++) {
		hl_type* t = m->code->globals[i];
		if (t->kind == HFUN) *(void**)(m->globals_data + m->globals_indexes[i]) = null_function;
		if (hl_is_ptr(t))
			hl_add_root(m->globals_data + m->globals_indexes[i]);
	}
	// inits
	if (hot_reload) m->hash = hl_code_hash_alloc(m->code);
	hl_module_init_natives(m);
	hl_module_init_indexes(m);
	// JIT
	ctx = hl_jit_alloc();
	if (ctx == NULL)
		return 0;
	hl_jit_init(ctx, m);
	for (i = 0;i < m->code->nfunctions;i++) {
		hl_function* f = m->code->functions + i;
		int fpos = hl_jit_function(ctx, m, f);
		if (fpos < 0) {
			hl_jit_free(ctx, false);
			return 0;
		}
		m->functions_ptrs[f->findex] = (void*)(int_val)fpos;
	}
	m->jit_code = hl_jit_code(ctx, m, &m->codesize, &m->jit_debug, NULL);
	for (i = 0;i < m->code->nfunctions;i++) {
		hl_function* f = m->code->functions + i;
		m->functions_ptrs[f->findex] = ((unsigned char*)m->jit_code) + ((int_val)m->functions_ptrs[f->findex]);
	}
	// INIT constants
	for (i = 0;i < m->code->nconstants;i++) {
		hl_constant* c = m->code->constants + i;
		hl_module_init_constant(m, c);
	}

#	ifdef HL_VTUNE
	if (!vtune_later) {
		hl_module_init_vtune(m);
	}
	else {
		hl_setup_vtune(hl_module_init_vtune, m);
	}
#	endif
	hl_module_add(m);
	hl_setup_exception(module_resolve_symbol, module_capture_stack);
	hl_gc_set_dump_types(hl_module_types_dump);
	hl_jit_free(ctx, hot_reload);
	if (hot_reload) {
		hl_code_hash_finalize(m->hash);
		m->jit_ctx = ctx;
	}
	return 1;
}

h_bool hl_module_patch(hl_module * m1, hl_code * c) {
	int i, i1, i2;
	bool has_changes = false;
	int changes_count = 0;
	jit_ctx* ctx = m1->jit_ctx;

	hl_module* m2 = hl_module_alloc(c);
	m2->hash = hl_code_hash_alloc(c);
	hl_code_hash_remap_globals(m2->hash, m1->hash);

	// share global data
	free(m2->globals_data);
	free(m2->globals_indexes);
	m2->globals_data = m1->globals_data;
	m2->globals_indexes = m1->globals_indexes;
	int gsize = m1->globals_size;
	for (i = m1->code->nglobals;i < m2->code->nglobals;i++) {
		hl_type* t = c->globals[i];
		gsize += hl_pad_size(gsize, t);
		m2->globals_indexes[i] = gsize;
		gsize += hl_type_size(t);
		if (hl_is_ptr(t))
			hl_add_root(m2->globals_data + m2->globals_indexes[i]);
	}
	memset(m2->globals_data + m1->globals_size, 0, gsize - m1->globals_size);
	m2->globals_size = gsize;

	hl_module_init_natives(m2);
	hl_module_init_indexes(m2);
	hl_jit_reset(ctx, m2);
	hl_code_hash_finalize(m2->hash);

	for (i = 0;i < m2->code->nconstants;i++) {
		hl_constant* c = m2->code->constants + i;
		if (c->global >= m1->code->nglobals)
			hl_module_init_constant(m2, c);
	}

	for (i2 = 0;i2 < m2->code->nfunctions;i2++) {
		hl_function* f2 = m2->code->functions + i2;
		int sign2 = m2->hash->functions_signs[i2];
		if (f2->field.name == NULL) {
			m2->hash->functions_hashes[i2] = -1;
			continue;
		}
		for (i1 = 0;i1 < m1->code->nfunctions;i1++) {
			int sign1 = m1->hash->functions_signs[i1];
			if (sign1 == sign2) {
				hl_function* f1 = m1->code->functions + i1;
				if ((f1->obj != NULL) != (f2->obj != NULL) || !f1->field.name || !f2->field.name) {
					printf("[HotReload] Signature conflict\n");
					continue;
				}
				if (ucmp(fun_obj(f1)->name, fun_obj(f2)->name) != 0 || ucmp(fun_field_name(f1), fun_field_name(f2)) != 0) {
					printf("[HotReload] Signature conflict\n");
					continue;
				}

				int hash2 = m2->hash->functions_hashes[i2];
				int hash1 = m1->hash->functions_hashes[i1];
				m2->hash->functions_hashes[i2] = i1; // index reference
				if (hash1 == hash2)
					break;
#				ifdef HL_DEBUG
				uprintf(USTR("%s."), fun_obj(f1)->name);
				uprintf(USTR("%s"), fun_field_name(f1));
				if (!f1->obj)
					printf("~%d", f1->ref);
				printf(" has been modified [%d]\n", f1->nops);
#				endif
				changes_count++;

				m1->hash->functions_hashes[i1] = hash2; // update hash
				int fpos = hl_jit_function(ctx, m2, f2);
				if (fpos < 0) return false;
				m2->functions_ptrs[f2->findex] = (void*)(int_val)fpos;
				has_changes = true;
				break;
			}
		}
		if (i1 == m1->code->nfunctions) {
			// not found (signature changed or new method) : inject new method!
			int fpos = hl_jit_function(ctx, m2, f2);
			if (fpos < 0) return false;
			m2->hash->functions_hashes[i2] = -1;
			m2->functions_ptrs[f2->findex] = (void*)(int_val)fpos;
#			ifdef HL_DEBUG
			uprintf(USTR("%s."), fun_obj(f2)->name);
			uprintf(USTR("%s"), fun_field_name(f2));
			if (!f2->obj)
				printf("~%d", f2->ref);
			printf(" has been added\n");
#			endif
			changes_count++;
			has_changes = true;
			// should be added to m1 functions for later reload?
		}
	}
	if (!has_changes) {
		printf("[HotReload] No changes found\n");
		fflush(stdout);
		hl_jit_free(ctx, true);
		return false;
	}

	// patch same types
	for (i1 = 0;i1 < m1->code->ntypes;i1++) {
		hl_type* p = m1->code->types + i1;
		switch (p->kind) {
		case HOBJ:
		case HSTRUCT:
			break;
		case HENUM:
			if (!p->tenum->global_value) continue;
			break;
		default:
			continue;
		}
		for (i2 = 0;i2 < c->ntypes;i2++) {
			hl_type* t = c->types + i2;
			if (p->kind != t->kind) continue;
			switch (p->kind) {
			case HOBJ:
			case HSTRUCT:
				if (ucmp(p->obj->name, t->obj->name) != 0) continue;
				if (hl_code_hash_type(m1->hash, p) == hl_code_hash_type(m2->hash, t)) {
					t->obj = p->obj; // alias the types ! they are different pointers but have the same layout
					t->vobj_proto = p->vobj_proto;
				}
				else {
					uprintf(USTR("[HotReload] Type %s has changed\n"), t->obj->name);
					changes_count++;
				}
				break;
			case HENUM:
				if (ucmp(p->tenum->name, t->tenum->name) != 0) continue;
				if (hl_code_hash_type(m1->hash, p) == hl_code_hash_type(m2->hash, t)) {
					t->tenum = p->tenum; // alias the types ! they are different pointers but have the same layout
				}
				else {
					uprintf(USTR("[HotReload] Type %s has changed\n"), t->tenum->name);
					changes_count++;
				}
				break;
			default:
				break;
			}
			break;
		}
	}

	m2->jit_code = hl_jit_code(ctx, m2, &m2->codesize, &m2->jit_debug, m1);

	// patch missing debug info
	int start = -1;
	if (m2->jit_debug) {
		for (i = 0;i < c->nfunctions;i++) {
			if (m2->jit_debug[i].start < 0) {
				m2->jit_debug[i].start = start;
				m2->jit_debug[i].offsets = NULL;
			}
			else {
				start = m2->jit_debug[i].start;
			}
		}
	}

	hl_jit_free(ctx, true);

	if (m2->jit_code == NULL) {
		printf("[HotReload] Couldn't JIT result\n");
		fflush(stdout);
		return false;
	}

	for (i = 0;i < m2->code->nfunctions;i++) {
		hl_function* f2 = m2->code->functions + i;
		if (m2->hash->functions_hashes[i] < -1) continue;
		if (m2->functions_ptrs[f2->findex] == NULL) continue;
		void* ptr = ((unsigned char*)m2->jit_code) + ((int_val)m2->functions_ptrs[f2->findex]);
		m2->functions_ptrs[f2->findex] = ptr;
		// update real function ptr
		if (m2->hash->functions_hashes[i] < 0) continue;
		hl_function* f1 = m1->code->functions + m2->hash->functions_hashes[i];
		hl_jit_patch_method(m1->functions_ptrs[f1->findex], m1->functions_ptrs + f1->findex);
		m1->functions_ptrs[f1->findex] = ptr;
	}
	for (i = 0;i < m1->code->ntypes;i++) {
		hl_type* t = m1->code->types + i;
		if (t->kind == HOBJ || t->kind == HSTRUCT) hl_flush_proto(t);
	}

	if (changes_count > 0) {
		printf("[HotReload] %d changes\n", changes_count);
		fflush(stdout);
	}
	hl_module_add(m2);

	// call entry point (will only update types)
	for (i = modules_count - 1;i >= 0;i--) {
		hl_module* m = reinterpret_cast<hl_module*>(cur_modules[i]);
		if (m->functions_ptrs[m->code->entrypoint]) {
			vclosure cl;
			cl.t = m->code->functions[m->functions_indexes[m->code->entrypoint]].type;
			cl.fun = m->functions_ptrs[m->code->entrypoint];
			cl.hasValue = 0;
			hl_dyn_call(&cl, NULL, 0);
			break;
		}
	}

	return true;
}

void hl_module_free(hl_module * m) {
	for (int i = 0;i < m->code->nglobals;i++) {
		if (hl_is_ptr(m->code->globals[i]))
			hl_remove_root(m->globals_data + m->globals_indexes[i]);
	}
	hl_free(&m->ctx.alloc);
	hl_free_executable_memory(m->code, m->codesize);
	if (m->hash) hl_code_hash_free(m->hash);
	free(m->functions_indexes);
	free(m->functions_ptrs);
	free(m->ctx.functions_types);
	free(m->globals_indexes);
	free(m->globals_data);
	if (m->jit_debug) {
		int i;
		for (i = 0;i < m->code->nfunctions;i++)
			free(m->jit_debug[i].offsets);
		free(m->jit_debug);
	}
	if (m->jit_ctx)
		hl_jit_free(m->jit_ctx, false);
	free(m);
}

/*
 * Copyright (C)2015-2016 Haxe Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifdef _MSC_VER
#pragma warning(disable:4820)
#endif
#include <math.h>

#ifdef __arm__
#	error "JIT does not support ARM processors, only x86 and x86-64 are supported, please use HashLink/C native compilation instead"
#endif

#ifdef HL_DEBUG
#	define JIT_DEBUG
#endif

typedef enum {
	Eax = 0,
	Ecx = 1,
	Edx = 2,
	Ebx = 3,
	Esp = 4,
	Ebp = 5,
	Esi = 6,
	Edi = 7,
#ifdef HL_64
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15,
#endif
	_LAST = 0xFF
} CpuReg;

typedef enum {
	MOV,
	LEA,
	PUSH,
	ADD,
	SUB,
	IMUL,	// only overflow flag changes compared to MUL
	DIV,
	IDIV,
	CDQ,
	CDQE,
	POP,
	RET,
	CALL,
	AND,
	OR,
	XOR,
	CMP,
	TEST,
	NOP,
	SHL,
	SHR,
	SAR,
	INC,
	DEC,
	JMP,
	// FPU
	FSTP,
	FSTP32,
	FLD,
	FLD32,
	FLDCW,
	// SSE
	MOVSD,
	MOVSS,
	COMISD,
	COMISS,
	ADDSD,
	SUBSD,
	MULSD,
	DIVSD,
	ADDSS,
	SUBSS,
	MULSS,
	DIVSS,
	XORPD,
	CVTSI2SD,
	CVTSI2SS,
	CVTSD2SI,
	CVTSD2SS,
	CVTSS2SD,
	CVTSS2SI,
	STMXCSR,
	LDMXCSR,
	// 8-16 bits
	MOV8,
	CMP8,
	TEST8,
	PUSH8,
	MOV16,
	CMP16,
	TEST16,
	// prefetchs
	PREFETCHT0,
	PREFETCHT1,
	PREFETCHT2,
	PREFETCHNTA,
	PREFETCHW,
	// --
	_CPU_LAST
} CpuOp;

#define JAlways		0
#define JOverflow	0x80
#define JULt		0x82
#define JUGte		0x83
#define JEq			0x84
#define JNeq		0x85
#define JULte		0x86
#define JUGt		0x87
#define JParity		0x8A
#define JNParity	0x8B
#define JSLt		0x8C
#define JSGte		0x8D
#define JSLte		0x8E
#define JSGt		0x8F

#define JCarry		JLt
#define JZero		JEq
#define JNotZero	JNeq

#define B(bv)	*ctx->buf.b++ = (unsigned char)(bv)
#define W(wv)	*ctx->buf.w++ = wv

#ifdef HL_64
#	define W64(wv)	*ctx->buf.w64++ = wv
#else
#	define W64(wv)	W(wv)
#endif

static const int SIB_MULT[] = { -1, 0, 1, -1, 2, -1, -1, -1, 3 };

#define MOD_RM(mod,reg,rm)		B(((mod) << 6) | (((reg)&7) << 3) | ((rm)&7))
#define SIB(mult,rmult,rbase)	B((SIB_MULT[mult]<<6) | (((rmult)&7)<<3) | ((rbase)&7))
#define IS_SBYTE(c)				( (c) >= -128 && (c) < 128 )

#define AddJump(how,local)		{ if( (how) == JAlways ) { B(0xE9); } else { B(0x0F); B(how); }; local = BUF_POS(); W(0); }
#define AddJump_small(how,local) { if( (how) == JAlways ) { B(0xEB); } else B(how - 0x10); local = BUF_POS() | 0x40000000; B(0); }
#define XJump(how,local)		AddJump(how,local)
#define XJump_small(how,local)		AddJump_small(how,local)

#define MAX_OP_SIZE				256

#define BUF_POS()				((int)(ctx->buf.b - ctx->startBuf))
#define RTYPE(r)				r->t->kind

#ifdef HL_64
#	define RESERVE_ADDRESS	0x8000000000000000
#else
#	define RESERVE_ADDRESS	0x80000000
#endif

#if defined(HL_WIN_CALL) && defined(HL_64)
#	define IS_WINCALL64 1
#else
#	define IS_WINCALL64 0
#endif

typedef struct jlist jlist;
struct jlist {
	int pos;
	int target;
	jlist* next;
};

typedef struct vreg vreg;

typedef enum {
	RCPU = 0,
	RFPU = 1,
	RSTACK = 2,
	RCONST = 3,
	RADDR = 4,
	RMEM = 5,
	RUNUSED = 6,
	RCPU_CALL = 1 | 8,
	RCPU_8BITS = 1 | 16
} preg_kind;

typedef struct {
	preg_kind kind;
	int id;
	int lock;
	vreg* holds;
} preg;

struct vreg {
	int stackPos;
	int size;
	hl_type* t;
	preg* current;
	preg stack;
};

#define REG_AT(i)		(ctx->pregs + (i))

#ifdef HL_64
#	define RCPU_COUNT	16
#	define RFPU_COUNT	16
#	ifdef HL_WIN_CALL
#		define CALL_NREGS			4
#		define RCPU_SCRATCH_COUNT	7
#		define RFPU_SCRATCH_COUNT	6
static const int RCPU_SCRATCH_REGS[] = { Eax, Ecx, Edx, R8, R9, R10, R11 };
static const CpuReg CALL_REGS[] = { Ecx, Edx, R8, R9 };
#	else
#		define CALL_NREGS			6 // TODO : XMM6+XMM7 are FPU reg parameters
#		define RCPU_SCRATCH_COUNT	9
#		define RFPU_SCRATCH_COUNT	16
static const int RCPU_SCRATCH_REGS[] = { Eax, Ecx, Edx, Esi, Edi, R8, R9, R10, R11 };
static const CpuReg CALL_REGS[] = { Edi, Esi, Edx, Ecx, R8, R9 };
#	endif
#else
#	define CALL_NREGS	0
#	define RCPU_COUNT	8
#	define RFPU_COUNT	8
#	define RCPU_SCRATCH_COUNT	3
#	define RFPU_SCRATCH_COUNT	8
static const int RCPU_SCRATCH_REGS[] = { Eax, Ecx, Edx };
#endif

#define XMM(i)			((i) + RCPU_COUNT)
#define PXMM(i)			REG_AT(XMM(i))
#define REG_IS_FPU(i)	((i) >= RCPU_COUNT)

#define PEAX			REG_AT(Eax)
#define PESP			REG_AT(Esp)
#define PEBP			REG_AT(Ebp)

#define REG_COUNT	(RCPU_COUNT + RFPU_COUNT)

#define ID2(a,b)	((a) | ((b)<<8))
#define R(id)		(ctx->vregs + (id))
#define ASSERT(i)	{ printf("JIT ERROR %d (jit.c line %d)\n",i,(int)__LINE__); jit_exit(); }
#define IS_FLOAT(r)	((r)->t->kind == HF64 || (r)->t->kind == HF32)
#define RLOCK(r)		if( (r)->lock < ctx->currentPos ) (r)->lock = ctx->currentPos
#define RUNLOCK(r)		if( (r)->lock == ctx->currentPos ) (r)->lock = 0

#define BREAK()		B(0xCC)

#if defined(HL_64) && defined(HL_VCC)
#	define JIT_CUSTOM_LONGJUMP
#endif

static preg _unused = { RUNUSED, 0, 0, NULL };
static preg* UNUSED = &_unused;

struct jit_ctx {
	union {
		unsigned char* b;
		unsigned int* w;
		unsigned long long* w64;
		int* i;
		double* d;
	} buf;
	vreg* vregs;
	preg pregs[REG_COUNT];
	vreg* savedRegs[REG_COUNT];
	int savedLocks[REG_COUNT];
	int* opsPos;
	int maxRegs;
	int maxOps;
	int bufSize;
	int totalRegsSize;
	int functionPos;
	int allocOffset;
	int currentPos;
	int nativeArgsCount;
	unsigned char* startBuf;
	hl_module* m;
	hl_function* f;
	jlist* jumps;
	jlist* calls;
	jlist* switchs;
	hl_alloc falloc; // cleared per-function
	hl_alloc galloc;
	vclosure* closure_list;
	hl_debug_infos* debug;
	int c2hl;
	int hl2c;
	int longjump;
	void* static_functions[8];
};

#define jit_exit() { hl_debug_break(); exit(-1); }
#define jit_error(msg)	_jit_error(ctx,msg,__LINE__)

#ifndef HL_64
#	ifdef HL_DEBUG
#		define error_i64() jit_error("i64-32")
#	else
void error_i64() {
	printf("The module you are loading is using 64 bit ints that are not supported by the HL32.\nPlease run using HL64 or compile with -D hl-legacy32");
	jit_exit();
}
#	endif
#endif

static void _jit_error(jit_ctx* ctx, const char* msg, int line);
static void on_jit_error(const char* msg, int_val line);

static preg* pmem(preg* r, CpuReg reg, int offset) {
	r->kind = RMEM;
	r->id = 0 | (reg << 4) | (offset << 8);
	return r;
}

static preg* pmem2(preg* r, CpuReg reg, CpuReg reg2, int mult, int offset) {
	r->kind = RMEM;
	r->id = mult | (reg << 4) | (reg2 << 8);
	r->holds = (vreg*)(int_val)offset;
	return r;
}

#ifdef HL_64
static preg* pcodeaddr(preg* r, int offset) {
	r->kind = RMEM;
	r->id = 15 | (offset << 4);
	return r;
}
#endif

static preg* pconst(preg* r, int c) {
	r->kind = RCONST;
	r->holds = NULL;
	r->id = c;
	return r;
}

static preg* pconst64(preg* r, int_val c) {
#ifdef HL_64
	if (((int)c) == c)
		return pconst(r, (int)c);
	r->kind = RCONST;
	r->id = 0xC064C064;
	r->holds = (vreg*)c;
	return r;
#else
	return pconst(r, (int)c);
#endif
}

#ifndef HL_64
// it is not possible to access direct 64 bit address in x86-64
static preg* paddr(preg* r, void* p) {
	r->kind = RADDR;
	r->holds = (vreg*)p;
	return r;
}
#endif

static void save_regs(jit_ctx* ctx) {
	int i;
	for (i = 0;i < REG_COUNT;i++) {
		ctx->savedRegs[i] = ctx->pregs[i].holds;
		ctx->savedLocks[i] = ctx->pregs[i].lock;
	}
}

static void restore_regs(jit_ctx* ctx) {
	int i;
	for (i = 0;i < ctx->maxRegs;i++)
		ctx->vregs[i].current = NULL;
	for (i = 0;i < REG_COUNT;i++) {
		vreg* r = ctx->savedRegs[i];
		preg* p = ctx->pregs + i;
		p->holds = r;
		p->lock = ctx->savedLocks[i];
		if (r) r->current = p;
	}
}

static void jit_buf(jit_ctx* ctx) {
	if (BUF_POS() > ctx->bufSize - MAX_OP_SIZE) {
		int nsize = ctx->bufSize * 4 / 3;
		unsigned char* nbuf;
		int curpos;
		if (nsize == 0) {
			int i;
			for (i = 0;i < ctx->m->code->nfunctions;i++)
				nsize += ctx->m->code->functions[i].nops;
			nsize *= 4;
		}
		if (nsize < ctx->bufSize + MAX_OP_SIZE * 4) nsize = ctx->bufSize + MAX_OP_SIZE * 4;
		curpos = BUF_POS();
		nbuf = (unsigned char*)malloc(nsize);
		if (nbuf == NULL) ASSERT(nsize);
		if (ctx->startBuf) {
			memcpy(nbuf, ctx->startBuf, curpos);
			free(ctx->startBuf);
		}
		ctx->startBuf = nbuf;
		ctx->buf.b = nbuf + curpos;
		ctx->bufSize = nsize;
	}
}

static const char* KNAMES[] = { "cpu","fpu","stack","const","addr","mem","unused" };
#define ERRIF(c)	if( c ) { printf("%s(%s,%s)\n",f?f->name:"???",KNAMES[a->kind], KNAMES[b->kind]); ASSERT(0); }

typedef struct {
	const char* name;						// single operand
	int r_mem;		// r32 / r/m32				r32
	int mem_r;		// r/m32 / r32				r/m32
	int r_const;	// r32 / imm32				imm32
	int r_i8;		// r32 / imm8				imm8
	int mem_const;	// r/m32 / imm32			N/A
} opform;

#define FLAG_LONGOP	0x80000000
#define FLAG_16B	0x40000000
#define FLAG_8B		0x20000000
#define FLAG_DUAL   0x10000000

#define RM(op,id) ((op) | (((id)+1)<<8))
#define GET_RM(op)	(((op) >> ((op) < 0 ? 24 : 8)) & 15)
#define SBYTE(op) ((op) << 16)
#define LONG_OP(op)	((op) | FLAG_LONGOP)
#define OP16(op)	LONG_OP((op) | FLAG_16B)
#define LONG_RM(op,id)	LONG_OP(op | (((id) + 1) << 24))

static opform OP_FORMS[_CPU_LAST] = {
	{ "MOV", 0x8B, 0x89, 0xB8, 0, RM(0xC7,0) },
	{ "LEA", 0x8D },
	{ "PUSH", 0x50, RM(0xFF,6), 0x68, 0x6A },
	{ "ADD", 0x03, 0x01, RM(0x81,0), RM(0x83,0) },
	{ "SUB", 0x2B, 0x29, RM(0x81,5), RM(0x83,5) },
	{ "IMUL", LONG_OP(0x0FAF), 0, 0x69 | FLAG_DUAL, 0x6B | FLAG_DUAL },
	{ "DIV", RM(0xF7,6), RM(0xF7,6) },
	{ "IDIV", RM(0xF7,7), RM(0xF7,7) },
	{ "CDQ", 0x99 },
	{ "CDQE", 0x98 },
	{ "POP", 0x58, RM(0x8F,0) },
	{ "RET", 0xC3 },
	{ "CALL", RM(0xFF,2), RM(0xFF,2), 0xE8 },
	{ "AND", 0x23, 0x21, RM(0x81,4), RM(0x83,4) },
	{ "OR", 0x0B, 0x09, RM(0x81,1), RM(0x83,1) },
	{ "XOR", 0x33, 0x31, RM(0x81,6), RM(0x83,6) },
	{ "CMP", 0x3B, 0x39, RM(0x81,7), RM(0x83,7) },
	{ "TEST", 0x85, 0x85/*SWP?*/, RM(0xF7,0) },
	{ "NOP", 0x90 },
	{ "SHL", RM(0xD3,4), 0, 0, RM(0xC1,4) },
	{ "SHR", RM(0xD3,5), 0, 0, RM(0xC1,5) },
	{ "SAR", RM(0xD3,7), 0, 0, RM(0xC1,7) },
	{ "INC", IS_64 ? RM(0xFF,0) : 0x40, RM(0xFF,0) },
	{ "DEC", IS_64 ? RM(0xFF,1) : 0x48, RM(0xFF,1) },
	{ "JMP", RM(0xFF,4) },
	// FPU
	{ "FSTP", 0, RM(0xDD,3) },
	{ "FSTP32", 0, RM(0xD9,3) },
	{ "FLD", 0, RM(0xDD,0) },
	{ "FLD32", 0, RM(0xD9,0) },
	{ "FLDCW", 0, RM(0xD9, 5) },
	// SSE
	{ "MOVSD", 0xF20F10, 0xF20F11  },
	{ "MOVSS", 0xF30F10, 0xF30F11  },
	{ "COMISD", 0x660F2F },
	{ "COMISS", LONG_OP(0x0F2F) },
	{ "ADDSD", 0xF20F58 },
	{ "SUBSD", 0xF20F5C },
	{ "MULSD", 0xF20F59 },
	{ "DIVSD", 0xF20F5E },
	{ "ADDSS", 0xF30F58 },
	{ "SUBSS", 0xF30F5C },
	{ "MULSS", 0xF30F59 },
	{ "DIVSS", 0xF30F5E },
	{ "XORPD", 0x660F57 },
	{ "CVTSI2SD", 0xF20F2A },
	{ "CVTSI2SS", 0xF30F2A },
	{ "CVTSD2SI", 0xF20F2D },
	{ "CVTSD2SS", 0xF20F5A },
	{ "CVTSS2SD", 0xF30F5A },
	{ "CVTSS2SI", 0xF30F2D },
	{ "STMXCSR", 0, LONG_RM(0x0FAE,3) },
	{ "LDMXCSR", 0, LONG_RM(0x0FAE,2) },
	// 8 bits,
	{ "MOV8", 0x8A, 0x88, 0, 0xB0, RM(0xC6,0) },
	{ "CMP8", 0x3A, 0x38, 0, RM(0x80,7) },
	{ "TEST8", 0x84, 0x84, RM(0xF6,0) },
	{ "PUSH8", 0, 0, 0x6A | FLAG_8B },
	{ "MOV16", OP16(0x8B), OP16(0x89), OP16(0xB8) },
	{ "CMP16", OP16(0x3B), OP16(0x39) },
	{ "TEST16", OP16(0x85) },
	// prefetchs
	{ "PREFETCHT0", 0, LONG_RM(0x0F18,1) },
	{ "PREFETCHT1", 0, LONG_RM(0x0F18,2) },
	{ "PREFETCHT2", 0, LONG_RM(0x0F18,3) },
	{ "PREFETCHNTA", 0, LONG_RM(0x0F18,0) },
	{ "PREFETCHW", 0, LONG_RM(0x0F0D,1) },
};

#ifdef HL_64
#	define REX()	if( r64 ) B(r64 | 0x40)
#else
#	define REX()
#endif

#define	OP(b)	\
	if( (b) & 0xFF0000 ) { \
		B((b)>>16); \
		if( r64 ) B(r64 | 0x40); /* also in 32 bits mode */ \
		B((b)>>8); \
		B(b); \
	} else { \
		if( (b) & FLAG_16B ) { \
			B(0x66); \
			REX(); \
		} else {\
			REX(); \
			if( (b) & FLAG_LONGOP ) B((b)>>8); \
		}\
		B(b); \
	}

static bool is_reg8(preg* a) {
	return a->kind == RSTACK || a->kind == RMEM || a->kind == RCONST || (a->kind == RCPU && a->id != Esi && a->id != Edi);
}

static void op(jit_ctx* ctx, CpuOp o, preg* a, preg* b, bool mode64) {
	opform* f = &OP_FORMS[o];
	int r64 = mode64 && (o != PUSH && o != POP && o != CALL && o != PUSH8 && o < PREFETCHT0) ? 8 : 0;
	switch (o) {
	case CMP8:
	case TEST8:
	case MOV8:
		if (!is_reg8(a) || !is_reg8(b))
			ASSERT(0);
		break;
	default:
		break;
	}
	switch (ID2(a->kind, b->kind)) {
	case ID2(RUNUSED, RUNUSED):
		ERRIF(f->r_mem == 0);
		OP(f->r_mem);
		break;
	case ID2(RCPU, RCPU):
	case ID2(RFPU, RFPU):
		ERRIF(f->r_mem == 0);
		if (a->id > 7) r64 |= 4;
		if (b->id > 7) r64 |= 1;
		OP(f->r_mem);
		MOD_RM(3, a->id, b->id);
		break;
	case ID2(RCPU, RFPU):
	case ID2(RFPU, RCPU):
		ERRIF((f->r_mem >> 16) == 0);
		if (a->id > 7) r64 |= 4;
		if (b->id > 7) r64 |= 1;
		OP(f->r_mem);
		MOD_RM(3, a->id, b->id);
		break;
	case ID2(RCPU, RUNUSED):
		ERRIF(f->r_mem == 0);
		if (a->id > 7) r64 |= 1;
		if (GET_RM(f->r_mem) > 0) {
			OP(f->r_mem);
			MOD_RM(3, GET_RM(f->r_mem) - 1, a->id);
		}
		else
			OP(f->r_mem + (a->id & 7));
		break;
	case ID2(RSTACK, RUNUSED):
		ERRIF(f->mem_r == 0 || GET_RM(f->mem_r) == 0);
		{
			int stackPos = R(a->id)->stackPos;
			OP(f->mem_r);
			if (IS_SBYTE(stackPos)) {
				MOD_RM(1, GET_RM(f->mem_r) - 1, Ebp);
				B(stackPos);
			}
			else {
				MOD_RM(2, GET_RM(f->mem_r) - 1, Ebp);
				W(stackPos);
			}
		}
		break;
	case ID2(RCPU, RCONST):
		ERRIF(f->r_const == 0 && f->r_i8 == 0);
		if (a->id > 7) r64 |= 1;
		{
			int_val cval = b->holds ? (int_val)b->holds : b->id;
			// short byte form
			if (f->r_i8 && IS_SBYTE(cval)) {
				if ((f->r_i8 & FLAG_DUAL) && a->id > 7) r64 |= 4;
				OP(f->r_i8);
				if ((f->r_i8 & FLAG_DUAL)) MOD_RM(3, a->id, a->id); else MOD_RM(3, GET_RM(f->r_i8) - 1, a->id);
				B((int)cval);
			}
			else if (GET_RM(f->r_const) > 0 || (f->r_const & FLAG_DUAL)) {
				if ((f->r_i8 & FLAG_DUAL) && a->id > 7) r64 |= 4;
				OP(f->r_const & 0xFF);
				if ((f->r_i8 & FLAG_DUAL)) MOD_RM(3, a->id, a->id); else MOD_RM(3, GET_RM(f->r_const) - 1, a->id);
				if (mode64 && IS_64 && o == MOV) W64(cval); else W((int)cval);
			}
			else {
				ERRIF(f->r_const == 0);
				OP((f->r_const & 0xFF) + (a->id & 7));
				if (mode64 && IS_64 && o == MOV) W64(cval); else W((int)cval);
			}
		}
		break;
	case ID2(RSTACK, RCPU):
	case ID2(RSTACK, RFPU):
		ERRIF(f->mem_r == 0);
		if (b->id > 7) r64 |= 4;
		{
			int stackPos = R(a->id)->stackPos;
			OP(f->mem_r);
			if (IS_SBYTE(stackPos)) {
				MOD_RM(1, b->id, Ebp);
				B(stackPos);
			}
			else {
				MOD_RM(2, b->id, Ebp);
				W(stackPos);
			}
		}
		break;
	case ID2(RCPU, RSTACK):
	case ID2(RFPU, RSTACK):
		ERRIF(f->r_mem == 0);
		if (a->id > 7) r64 |= 4;
		{
			int stackPos = R(b->id)->stackPos;
			OP(f->r_mem);
			if (IS_SBYTE(stackPos)) {
				MOD_RM(1, a->id, Ebp);
				B(stackPos);
			}
			else {
				MOD_RM(2, a->id, Ebp);
				W(stackPos);
			}
		}
		break;
	case ID2(RCONST, RUNUSED):
		ERRIF(f->r_const == 0);
		{
			int_val cval = a->holds ? (int_val)a->holds : a->id;
			OP(f->r_const);
			if (f->r_const & FLAG_8B) B((int)cval); else W((int)cval);
		}
		break;
	case ID2(RMEM, RUNUSED):
		ERRIF(f->mem_r == 0);
		{
			int mult = a->id & 0xF;
			int regOrOffs = mult == 15 ? a->id >> 4 : a->id >> 8;
			CpuReg reg = static_cast<CpuReg>((a->id >> 4) & 0xF);
			if (mult == 15) {
				ERRIF(1);
			}
			else if (mult == 0) {
				if (reg > 7) r64 |= 1;
				OP(f->mem_r);
				if (regOrOffs == 0 && (reg & 7) != Ebp) {
					MOD_RM(0, GET_RM(f->mem_r) - 1, reg);
					if ((reg & 7) == Esp) B(0x24);
				}
				else if (IS_SBYTE(regOrOffs)) {
					MOD_RM(1, GET_RM(f->mem_r) - 1, reg);
					if ((reg & 7) == Esp) B(0x24);
					B(regOrOffs);
				}
				else {
					MOD_RM(2, GET_RM(f->mem_r) - 1, reg);
					if ((reg & 7) == Esp) B(0x24);
					W(regOrOffs);
				}
			}
			else {
				// [eax + ebx * M]
				ERRIF(1);
			}
		}
		break;
	case ID2(RCPU, RMEM):
	case ID2(RFPU, RMEM):
		ERRIF(f->r_mem == 0);
		{
			int mult = b->id & 0xF;
			int regOrOffs = mult == 15 ? b->id >> 4 : b->id >> 8;
			CpuReg reg = static_cast<CpuReg>((b->id >> 4) & 0xF);
			if (mult == 15) {
				int pos;
				if (a->id > 7) r64 |= 4;
				OP(f->r_mem);
				MOD_RM(0, a->id, 5);
				if (IS_64) {
					// offset wrt current code
					pos = BUF_POS() + 4;
					W(regOrOffs - pos);
				}
				else {
					ERRIF(1);
				}
			}
			else if (mult == 0) {
				if (a->id > 7) r64 |= 4;
				if (reg > 7) r64 |= 1;
				OP(f->r_mem);
				if (regOrOffs == 0 && (reg & 7) != Ebp) {
					MOD_RM(0, a->id, reg);
					if ((reg & 7) == Esp) B(0x24);
				}
				else if (IS_SBYTE(regOrOffs)) {
					MOD_RM(1, a->id, reg);
					if ((reg & 7) == Esp) B(0x24);
					B(regOrOffs);
				}
				else {
					MOD_RM(2, a->id, reg);
					if ((reg & 7) == Esp) B(0x24);
					W(regOrOffs);
				}
			}
			else {
				int offset = (int)(int_val)b->holds;
				if (a->id > 7) r64 |= 4;
				if (reg > 7) r64 |= 1;
				if (regOrOffs > 7) r64 |= 2;
				OP(f->r_mem);
				MOD_RM(offset == 0 ? 0 : IS_SBYTE(offset) ? 1 : 2, a->id, 4);
				SIB(mult, regOrOffs, reg);
				if (offset) {
					if (IS_SBYTE(offset)) B(offset); else W(offset);
				}
			}
		}
		break;
#	ifndef HL_64
	case ID2(RFPU, RADDR):
#	endif
	case ID2(RCPU, RADDR):
		ERRIF(f->r_mem == 0);
		if (a->id > 7) r64 |= 4;
		OP(f->r_mem);
		MOD_RM(0, a->id, 5);
		if (IS_64)
			W64((int_val)b->holds);
		else
			W((int)(int_val)b->holds);
		break;
#	ifndef HL_64
	case ID2(RADDR, RFPU):
#	endif
	case ID2(RADDR, RCPU):
		ERRIF(f->mem_r == 0);
		if (b->id > 7) r64 |= 4;
		OP(f->mem_r);
		MOD_RM(0, b->id, 5);
		if (IS_64)
			W64((int_val)a->holds);
		else
			W((int)(int_val)a->holds);
		break;
	case ID2(RMEM, RCPU):
	case ID2(RMEM, RFPU):
		ERRIF(f->mem_r == 0);
		{
			int mult = a->id & 0xF;
			int regOrOffs = mult == 15 ? a->id >> 4 : a->id >> 8;
			CpuReg reg = static_cast<CpuReg>((a->id >> 4) & 0xF);
			if (mult == 15) {
				int pos;
				if (b->id > 7) r64 |= 4;
				OP(f->mem_r);
				MOD_RM(0, b->id, 5);
				if (IS_64) {
					// offset wrt current code
					pos = BUF_POS() + 4;
					W(regOrOffs - pos);
				}
				else {
					ERRIF(1);
				}
			}
			else if (mult == 0) {
				if (b->id > 7) r64 |= 4;
				if (reg > 7) r64 |= 1;
				OP(f->mem_r);
				if (regOrOffs == 0 && (reg & 7) != Ebp) {
					MOD_RM(0, b->id, reg);
					if ((reg & 7) == Esp) B(0x24);
				}
				else if (IS_SBYTE(regOrOffs)) {
					MOD_RM(1, b->id, reg);
					if ((reg & 7) == Esp) B(0x24);
					B(regOrOffs);
				}
				else {
					MOD_RM(2, b->id, reg);
					if ((reg & 7) == Esp) B(0x24);
					W(regOrOffs);
				}
			}
			else {
				int offset = (int)(int_val)a->holds;
				if (b->id > 7) r64 |= 4;
				if (reg > 7) r64 |= 1;
				if (regOrOffs > 7) r64 |= 2;
				OP(f->mem_r);
				MOD_RM(offset == 0 ? 0 : IS_SBYTE(offset) ? 1 : 2, b->id, 4);
				SIB(mult, regOrOffs, reg);
				if (offset) {
					if (IS_SBYTE(offset)) B(offset); else W(offset);
				}
			}
		}
		break;
	default:
		ERRIF(1);
	}
	if (ctx->debug && ctx->f && o == CALL) {
		preg p;
		op(ctx, MOV, pmem(&p, Esp, -HL_WSIZE), PEBP, true); // erase EIP (clean stack report)
	}
}

static void op32(jit_ctx* ctx, CpuOp o, preg* a, preg* b) {
	op(ctx, o, a, b, false);
}

static void op64(jit_ctx* ctx, CpuOp o, preg* a, preg* b) {
#ifndef HL_64
	op(ctx, o, a, b, false);
#else
	op(ctx, o, a, b, true);
#endif
}

static void patch_jump(jit_ctx* ctx, int p) {
	if (p == 0) return;
	if (p & 0x40000000) {
		int d;
		p &= 0x3FFFFFFF;
		d = BUF_POS() - (p + 1);
		if (d < -128 || d >= 128) ASSERT(d);
		*(char*)(ctx->startBuf + p) = (char)d;
	}
	else {
		*(int*)(ctx->startBuf + p) = BUF_POS() - (p + 4);
	}
}

static void patch_jump_to(jit_ctx* ctx, int p, int target) {
	if (p == 0) return;
	if (p & 0x40000000) {
		int d;
		p &= 0x3FFFFFFF;
		d = target - (p + 1);
		if (d < -128 || d >= 128) ASSERT(d);
		*(char*)(ctx->startBuf + p) = (char)d;
	}
	else {
		*(int*)(ctx->startBuf + p) = target - (p + 4);
	}
}

static int stack_size(hl_type* t) {
	switch (t->kind) {
	case HUI8:
	case HUI16:
	case HBOOL:
#	ifdef HL_64
	case HI32:
	case HF32:
#	endif
		return sizeof(int_val);
	case HI64:
	default:
		return hl_type_size(t);
	}
}

static int call_reg_index(int reg) {
#	ifdef HL_64
	int i;
	for (i = 0;i < CALL_NREGS;i++)
		if (CALL_REGS[i] == reg)
			return i;
#	endif
	return -1;
}

static bool is_call_reg(preg* p) {
#	ifdef HL_64
	int i;
	if (p->kind == RFPU)
		return p->id < CALL_NREGS;
	for (i = 0;i < CALL_NREGS;i++)
		if (p->kind == RCPU && p->id == CALL_REGS[i])
			return true;
	return false;
#	else
	return false;
#	endif
}

static preg* alloc_reg(jit_ctx* ctx, preg_kind k) {
	int i;
	preg* p;
	switch (k) {
	case RCPU:
	case RCPU_CALL:
	case RCPU_8BITS:
	{
		int off = ctx->allocOffset++;
		const int count = RCPU_SCRATCH_COUNT;
		for (i = 0;i < count;i++) {
			int r = RCPU_SCRATCH_REGS[(i + off) % count];
			p = ctx->pregs + r;
			if (p->lock >= ctx->currentPos) continue;
			if (k == RCPU_CALL && is_call_reg(p)) continue;
			if (k == RCPU_8BITS && !is_reg8(p)) continue;
			if (p->holds == NULL) {
				RLOCK(p);
				return p;
			}
		}
		for (i = 0;i < count;i++) {
			preg* p = ctx->pregs + RCPU_SCRATCH_REGS[(i + off) % count];
			if (p->lock >= ctx->currentPos) continue;
			if (k == RCPU_CALL && is_call_reg(p)) continue;
			if (k == RCPU_8BITS && !is_reg8(p)) continue;
			if (p->holds) {
				RLOCK(p);
				p->holds->current = NULL;
				p->holds = NULL;
				return p;
			}
		}
	}
	break;
	case RFPU:
	{
		int off = ctx->allocOffset++;
		const int count = RFPU_SCRATCH_COUNT;
		for (i = 0;i < count;i++) {
			preg* p = PXMM((i + off) % count);
			if (p->lock >= ctx->currentPos) continue;
			if (p->holds == NULL) {
				RLOCK(p);
				return p;
			}
		}
		for (i = 0;i < count;i++) {
			preg* p = PXMM((i + off) % count);
			if (p->lock >= ctx->currentPos) continue;
			if (p->holds) {
				RLOCK(p);
				p->holds->current = NULL;
				p->holds = NULL;
				return p;
			}
		}
	}
	break;
	default:
		ASSERT(k);
	}
	ASSERT(0); // out of registers !
	return NULL;
}

static preg* fetch(vreg* r) {
	if (r->current)
		return r->current;
	return &r->stack;
}

static void scratch(preg* r) {
	if (r && r->holds) {
		r->holds->current = NULL;
		r->holds = NULL;
		r->lock = 0;
	}
}

static preg* copy(jit_ctx* ctx, preg* to, preg* from, int size);

static void load(jit_ctx* ctx, preg* r, vreg* v) {
	preg* from = fetch(v);
	if (from == r || v->size == 0) return;
	if (r->holds) r->holds->current = NULL;
	if (v->current) {
		v->current->holds = NULL;
		from = r;
	}
	r->holds = v;
	v->current = r;
	copy(ctx, r, from, v->size);
}

static preg* alloc_fpu(jit_ctx* ctx, vreg* r, bool andLoad) {
	preg* p = fetch(r);
	if (p->kind != RFPU) {
		if (!IS_FLOAT(r) && (IS_64 || r->t->kind != HI64)) ASSERT(r->t->kind);
		p = alloc_reg(ctx, RFPU);
		if (andLoad)
			load(ctx, p, r);
		else {
			if (r->current)
				r->current->holds = NULL;
			r->current = p;
			p->holds = r;
		}
	}
	else
		RLOCK(p);
	return p;
}

static void reg_bind(vreg* r, preg* p) {
	if (r->current)
		r->current->holds = NULL;
	r->current = p;
	p->holds = r;
}

static preg* alloc_cpu(jit_ctx* ctx, vreg* r, bool andLoad) {
	preg* p = fetch(r);
	if (p->kind != RCPU) {
#		ifndef HL_64
		if (r->t->kind == HI64) return alloc_fpu(ctx, r, andLoad);
		if (r->size > 4) ASSERT(r->size);
#		endif
		p = alloc_reg(ctx, RCPU);
		if (andLoad)
			load(ctx, p, r);
		else
			reg_bind(r, p);
	}
	else
		RLOCK(p);
	return p;
}

// allocate a register that is not a call parameter
static preg* alloc_cpu_call(jit_ctx* ctx, vreg* r) {
	preg* p = fetch(r);
	if (p->kind != RCPU) {
#		ifndef HL_64
		if (r->t->kind == HI64) return alloc_fpu(ctx, r, true);
		if (r->size > 4) ASSERT(r->size);
#		endif
		p = alloc_reg(ctx, RCPU_CALL);
		load(ctx, p, r);
	}
	else if (is_call_reg(p)) {
		preg* p2 = alloc_reg(ctx, RCPU_CALL);
		op64(ctx, MOV, p2, p);
		scratch(p);
		reg_bind(r, p2);
		return p2;
	}
	else
		RLOCK(p);
	return p;
}

static preg* fetch32(jit_ctx* ctx, vreg* r) {
	if (r->current)
		return r->current;
	// make sure that the register is correctly erased
	if (r->size < 4) {
		preg* p = alloc_cpu(ctx, r, true);
		RUNLOCK(p);
		return p;
	}
	return fetch(r);
}

// make sure higher bits are zeroes
static preg* alloc_cpu64(jit_ctx* ctx, vreg* r, bool andLoad) {
#	ifndef HL_64
	return alloc_cpu(ctx, r, andLoad);
#	else
	preg* p = fetch(r);
	if (!andLoad) ASSERT(0);
	if (p->kind != RCPU) {
		p = alloc_reg(ctx, RCPU);
		op64(ctx, XOR, p, p);
		load(ctx, p, r);
	}
	else {
		// remove higher bits
		preg tmp;
		op64(ctx, SHL, p, pconst(&tmp, 32));
		op64(ctx, SHR, p, pconst(&tmp, 32));
		RLOCK(p);
	}
	return p;
#	endif
}

// make sure the register can be used with 8 bits access
static preg* alloc_cpu8(jit_ctx* ctx, vreg* r, bool andLoad) {
	preg* p = fetch(r);
	if (p->kind != RCPU) {
		p = alloc_reg(ctx, RCPU_8BITS);
		load(ctx, p, r);
	}
	else if (!is_reg8(p)) {
		preg* p2 = alloc_reg(ctx, RCPU_8BITS);
		op64(ctx, MOV, p2, p);
		scratch(p);
		reg_bind(r, p2);
		return p2;
	}
	else
		RLOCK(p);
	return p;
}

static preg* copy(jit_ctx* ctx, preg* to, preg* from, int size) {
	if (size == 0 || to == from) return to;
	switch (ID2(to->kind, from->kind)) {
	case ID2(RMEM, RCPU):
	case ID2(RSTACK, RCPU):
	case ID2(RCPU, RSTACK):
	case ID2(RCPU, RMEM):
	case ID2(RCPU, RCPU):
#	ifndef HL_64
	case ID2(RCPU, RADDR):
	case ID2(RADDR, RCPU):
#	endif
		switch (size) {
		case 1:
			if (to->kind == RCPU) {
				op64(ctx, XOR, to, to);
				if (!is_reg8(to)) {
					preg p;
					op32(ctx, MOV16, to, from);
					op32(ctx, SHL, to, pconst(&p, 24));
					op32(ctx, SHR, to, pconst(&p, 24));
					break;
				}
			}
			if (!is_reg8(from)) {
				preg* r = alloc_reg(ctx, RCPU_CALL);
				op32(ctx, MOV, r, from);
				RUNLOCK(r);
				op32(ctx, MOV8, to, r);
				return from;
			}
			op32(ctx, MOV8, to, from);
			break;
		case 2:
			if (to->kind == RCPU)
				op64(ctx, XOR, to, to);
			op32(ctx, MOV16, to, from);
			break;
		case 4:
			op32(ctx, MOV, to, from);
			break;
		case 8:
			if (IS_64) {
				op64(ctx, MOV, to, from);
				break;
			}
		default:
			ASSERT(size);
		}
		return to->kind == RCPU ? to : from;
	case ID2(RFPU, RFPU):
	case ID2(RMEM, RFPU):
	case ID2(RSTACK, RFPU):
	case ID2(RFPU, RMEM):
	case ID2(RFPU, RSTACK):
		switch (size) {
		case 8:
			op64(ctx, MOVSD, to, from);
			break;
		case 4:
			op32(ctx, MOVSS, to, from);
			break;
		default:
			ASSERT(size);
		}
		return to->kind == RFPU ? to : from;
	case ID2(RMEM, RSTACK):
	{
		vreg* rfrom = R(from->id);
		if (IS_FLOAT(rfrom))
			return copy(ctx, to, alloc_fpu(ctx, rfrom, true), size);
		return copy(ctx, to, alloc_cpu(ctx, rfrom, true), size);
	}
	case ID2(RMEM, RMEM):
	case ID2(RSTACK, RMEM):
	case ID2(RSTACK, RSTACK):
#	ifndef HL_64
	case ID2(RMEM, RADDR):
	case ID2(RSTACK, RADDR):
	case ID2(RADDR, RSTACK):
#	endif
	{
		preg* tmp;
		if ((!IS_64 && size == 8) || (to->kind == RSTACK && IS_FLOAT(R(to->id))) || (from->kind == RSTACK && IS_FLOAT(R(from->id)))) {
			tmp = alloc_reg(ctx, RFPU);
			op64(ctx, size == 8 ? MOVSD : MOVSS, tmp, from);
		}
		else {
			tmp = alloc_reg(ctx, RCPU);
			copy(ctx, tmp, from, size);
		}
		return copy(ctx, to, tmp, size);
	}
#	ifdef HL_64
	case ID2(RCPU, RADDR):
	case ID2(RMEM, RADDR):
	case ID2(RSTACK, RADDR):
	{
		preg p;
		preg* tmp = alloc_reg(ctx, RCPU);
		op64(ctx, MOV, tmp, pconst64(&p, (int_val)from->holds));
		return copy(ctx, to, pmem(&p, static_cast<CpuReg>(tmp->id), 0), size);
	}
	case ID2(RADDR, RCPU):
	case ID2(RADDR, RMEM):
	case ID2(RADDR, RSTACK):
	{
		preg p;
		preg* tmp = alloc_reg(ctx, RCPU);
		op64(ctx, MOV, tmp, pconst64(&p, (int_val)to->holds));
		return copy(ctx, pmem(&p, static_cast<CpuReg>(tmp->id), 0), from, size);
	}
#	endif
	default:
		break;
	}
	printf("copy(%s,%s)\n", KNAMES[to->kind], KNAMES[from->kind]);
	ASSERT(0);
	return NULL;
}

static void store(jit_ctx* ctx, vreg* r, preg* v, bool bind) {
	if (r->current && r->current != v) {
		r->current->holds = NULL;
		r->current = NULL;
	}
	v = copy(ctx, &r->stack, v, r->size);
	if (IS_FLOAT(r) != (v->kind == RFPU))
		ASSERT(0);
	if (bind && r->current != v && (v->kind == RCPU || v->kind == RFPU)) {
		scratch(v);
		r->current = v;
		v->holds = r;
	}
}

static void store_result(jit_ctx* ctx, vreg* r) {
#	ifndef HL_64
	switch (r->t->kind) {
	case HF64:
		scratch(r->current);
		op64(ctx, FSTP, &r->stack, UNUSED);
		break;
	case HF32:
		scratch(r->current);
		op64(ctx, FSTP32, &r->stack, UNUSED);
		break;
	case HI64:
		scratch(r->current);
		error_i64();
		break;
	default:
#	endif
		store(ctx, r, IS_FLOAT(r) ? REG_AT(XMM(0)) : PEAX, true);
#	ifndef HL_64
		break;
	}
#	endif
}

static void op_mov(jit_ctx* ctx, vreg* to, vreg* from) {
	preg* r = fetch(from);
#	ifndef HL_64
	if (to->t->kind == HI64) {
		error_i64();
		return;
	}
#	endif
	if (from->t->kind == HF32 && r->kind != RFPU)
		r = alloc_fpu(ctx, from, true);
	store(ctx, to, r, true);
}

static void copy_to(jit_ctx* ctx, vreg* to, preg* from) {
	store(ctx, to, from, true);
}

static void copy_from(jit_ctx* ctx, preg* to, vreg* from) {
	copy(ctx, to, fetch(from), from->size);
}

static void store_const(jit_ctx* ctx, vreg* r, int c) {
	preg p;
	if (c == 0)
		op(ctx, XOR, alloc_cpu(ctx, r, false), alloc_cpu(ctx, r, false), r->size == 8);
	else if (r->size == 8)
		op64(ctx, MOV, alloc_cpu(ctx, r, false), pconst64(&p, c));
	else
		op32(ctx, MOV, alloc_cpu(ctx, r, false), pconst(&p, c));
	store(ctx, r, r->current, false);
}

static void discard_regs(jit_ctx* ctx, bool native_call) {
	int i;
	for (i = 0;i < RCPU_SCRATCH_COUNT;i++) {
		preg* r = ctx->pregs + RCPU_SCRATCH_REGS[i];
		if (r->holds) {
			r->holds->current = NULL;
			r->holds = NULL;
		}
	}
	for (i = 0;i < RFPU_COUNT;i++) {
		preg* r = ctx->pregs + XMM(i);
		if (r->holds) {
			r->holds->current = NULL;
			r->holds = NULL;
		}
	}
}

static int pad_before_call(jit_ctx* ctx, int size) {
	int total = size + ctx->totalRegsSize + HL_WSIZE * 2; // EIP+EBP
	if (total & 15) {
		int pad = 16 - (total & 15);
		preg p;
		if (pad) op64(ctx, SUB, PESP, pconst(&p, pad));
		size += pad;
	}
	return size;
}

static void push_reg(jit_ctx* ctx, vreg* r) {
	preg p;
	switch (stack_size(r->t)) {
	case 1:
		op64(ctx, SUB, PESP, pconst(&p, 1));
		op32(ctx, MOV8, pmem(&p, Esp, 0), alloc_cpu8(ctx, r, true));
		break;
	case 2:
		op64(ctx, SUB, PESP, pconst(&p, 2));
		op32(ctx, MOV16, pmem(&p, Esp, 0), alloc_cpu(ctx, r, true));
		break;
	case 4:
		if (r->size < 4)
			alloc_cpu(ctx, r, true); // force fetch (higher bits set to 0)
		if (!IS_64) {
			if (r->current != NULL && r->current->kind == RFPU) scratch(r->current);
			op32(ctx, PUSH, fetch(r), UNUSED);
		}
		else {
			// pseudo push32 (not available)
			op64(ctx, SUB, PESP, pconst(&p, 4));
			op32(ctx, MOV, pmem(&p, Esp, 0), alloc_cpu(ctx, r, true));
		}
		break;
	case 8:
		if (fetch(r)->kind == RFPU) {
			op64(ctx, SUB, PESP, pconst(&p, 8));
			op64(ctx, MOVSD, pmem(&p, Esp, 0), fetch(r));
		}
		else if (IS_64)
			op64(ctx, PUSH, fetch(r), UNUSED);
		else if (r->stack.kind == RSTACK) {
			scratch(r->current);
			r->stackPos += 4;
			op32(ctx, PUSH, &r->stack, UNUSED);
			r->stackPos -= 4;
			op32(ctx, PUSH, &r->stack, UNUSED);
		}
		else
			ASSERT(0);
		break;
	default:
		ASSERT(r->size);
	}
}

static int begin_native_call(jit_ctx* ctx, int nargs) {
	ctx->nativeArgsCount = nargs;
	return pad_before_call(ctx, nargs > CALL_NREGS ? (nargs - CALL_NREGS) * HL_WSIZE : 0);
}

static preg* alloc_native_arg(jit_ctx* ctx) {
#	ifdef HL_64
	int rid = ctx->nativeArgsCount - 1;
	preg* r = rid < CALL_NREGS ? REG_AT(CALL_REGS[rid]) : alloc_reg(ctx, RCPU_CALL);
	scratch(r);
	return r;
#	else
	return alloc_reg(ctx, RCPU);
#	endif
}

static void set_native_arg(jit_ctx* ctx, preg* r) {
	if (r->kind == RSTACK) {
		vreg* v = ctx->vregs + r->id;
		if (v->size < 4)
			r = fetch32(ctx, v);
	}
#	ifdef HL_64
	if (r->kind == RFPU) ASSERT(0);
	int rid = --ctx->nativeArgsCount;
	preg* target;
	if (rid >= CALL_NREGS) {
		op64(ctx, PUSH, r, UNUSED);
		return;
	}
	target = REG_AT(CALL_REGS[rid]);
	if (target != r) {
		op64(ctx, MOV, target, r);
		scratch(target);
	}
#	else
	op32(ctx, PUSH, r, UNUSED);
#	endif
}

static void set_native_arg_fpu(jit_ctx* ctx, preg* r, bool isf32) {
#	ifdef HL_64
	if (r->kind == RCPU) ASSERT(0);
	// can only be used if last argument !!
	ctx->nativeArgsCount--;
	preg* target = REG_AT(XMM(IS_WINCALL64 ? ctx->nativeArgsCount : 0));
	if (target != r) {
		op64(ctx, isf32 ? MOVSS : MOVSD, target, r);
		scratch(target);
	}
#	else
	op32(ctx, PUSH, r, UNUSED);
#	endif
}

typedef struct {
	int nextCpu;
	int nextFpu;
	int mapped[REG_COUNT];
} call_regs;

static int select_call_reg(call_regs* regs, hl_type* t, int id) {
#	ifndef HL_64
	return -1;
#else
	bool isFloat = t->kind == HF32 || t->kind == HF64;
#	ifdef HL_WIN_CALL
	int index = regs->nextCpu++;
#	else
	int index = isFloat ? regs->nextFpu++ : regs->nextCpu++;
#	endif
	if (index >= CALL_NREGS)
		return -1;
	int reg = isFloat ? XMM(index) : CALL_REGS[index];
	regs->mapped[reg] = id + 1;
	return reg;
#endif
}

static int mapped_reg(call_regs* regs, int id) {
#	ifndef HL_64
	return -1;
#else
	int i;
	for (i = 0;i < CALL_NREGS;i++) {
		int r = CALL_REGS[i];
		if (regs->mapped[r] == id + 1) return r;
		r = XMM(i);
		if (regs->mapped[r] == id + 1) return r;
	}
	return -1;
#endif
}

static int prepare_call_args(jit_ctx* ctx, int count, int* args, vreg* vregs, int extraSize) {
	int i;
	int size = extraSize, paddedSize;
	call_regs ctmp = { 0 };
	for (i = 0;i < count;i++) {
		vreg* r = vregs + args[i];
		int cr = select_call_reg(&ctmp, r->t, i);
		if (cr >= 0) {
			preg* c = REG_AT(cr);
			preg* cur = fetch(r);
			if (cur != c) {
				copy(ctx, c, cur, r->size);
				scratch(c);
			}
			RLOCK(c);
			continue;
		}
		size += stack_size(r->t);
	}
	paddedSize = pad_before_call(ctx, size);
	for (i = 0;i < count;i++) {
		// RTL
		int j = count - (i + 1);
		vreg* r = vregs + args[j];
		if ((i & 7) == 0) jit_buf(ctx);
		if (mapped_reg(&ctmp, j) >= 0) continue;
		push_reg(ctx, r);
		if (r->current) RUNLOCK(r->current);
	}
	return paddedSize;
}

static void op_call(jit_ctx* ctx, preg* r, int size) {
	preg p;
#	ifdef JIT_DEBUG
	if (IS_64 && size >= 0) {
		int jchk;
		op32(ctx, TEST, PESP, pconst(&p, 15));
		XJump(JZero, jchk);
		BREAK(); // unaligned ESP
		patch_jump(ctx, jchk);
	}
#	endif
	if (IS_WINCALL64) {
		// MSVC requires 32bytes of free space here
		op64(ctx, SUB, PESP, pconst(&p, 32));
		if (size >= 0) size += 32;
	}
	op32(ctx, CALL, r, UNUSED);
	if (size > 0) op64(ctx, ADD, PESP, pconst(&p, size));
}

static void call_native(jit_ctx* ctx, void* nativeFun, int size) {
	bool isExc = nativeFun == hl_assert || nativeFun == hl_throw || nativeFun == on_jit_error;
	preg p;
	// native function, already resolved
	op64(ctx, MOV, PEAX, pconst64(&p, (int_val)nativeFun));
	op_call(ctx, PEAX, isExc ? -1 : size);
	if (isExc)
		return;
	discard_regs(ctx, true);
}

static void op_call_fun(jit_ctx* ctx, vreg* dst, int findex, int count, int* args) {
	int fid = findex < 0 ? -1 : ctx->m->functions_indexes[findex];
	bool isNative = fid >= ctx->m->code->nfunctions;
	int size = prepare_call_args(ctx, count, args, ctx->vregs, 0);
	preg p;
	if (fid < 0) {
		ASSERT(fid);
	}
	else if (isNative) {
		call_native(ctx, ctx->m->functions_ptrs[findex], size);
	}
	else {
		int cpos = BUF_POS() + (IS_WINCALL64 ? 4 : 0);
#		ifdef JIT_DEBUG
		if (IS_64) cpos += 13; // ESP CHECK
#		endif
		if (ctx->m->functions_ptrs[findex]) {
			// already compiled
			op_call(ctx, pconst(&p, (int)(int_val)ctx->m->functions_ptrs[findex] - (cpos + 5)), size);
		}
		else if (ctx->m->code->functions + fid == ctx->f) {
			// our current function
			op_call(ctx, pconst(&p, ctx->functionPos - (cpos + 5)), size);
		}
		else {
			// stage for later
			jlist* j = (jlist*)hl_malloc(&ctx->galloc, sizeof(jlist));
			j->pos = cpos;
			j->target = findex;
			j->next = ctx->calls;
			ctx->calls = j;
			op_call(ctx, pconst(&p, 0), size);
		}
		discard_regs(ctx, false);
	}
	if (dst)
		store_result(ctx, dst);
}

static void op_enter(jit_ctx* ctx) {
	preg p;
	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);
	if (ctx->totalRegsSize) op64(ctx, SUB, PESP, pconst(&p, ctx->totalRegsSize));
}

static void op_ret(jit_ctx* ctx, vreg* r) {
	preg p;
	switch (r->t->kind) {
	case HF32:
#		ifdef HL_64
		op64(ctx, MOVSS, PXMM(0), fetch(r));
#		else
		op64(ctx, FLD32, &r->stack, UNUSED);
#		endif
		break;
	case HF64:
#		ifdef HL_64
		op64(ctx, MOVSD, PXMM(0), fetch(r));
#		else
		op64(ctx, FLD, &r->stack, UNUSED);
#		endif
		break;
	default:
		if (r->size < 4 && !r->current)
			fetch32(ctx, r);
		if (r->current != PEAX)
			op64(ctx, MOV, PEAX, fetch(r));
		break;
	}
	if (ctx->totalRegsSize) op64(ctx, ADD, PESP, pconst(&p, ctx->totalRegsSize));
#	ifdef JIT_DEBUG
	{
		int jeq;
		op64(ctx, CMP, PESP, PEBP);
		XJump_small(JEq, jeq);
		jit_error("invalid ESP");
		patch_jump(ctx, jeq);
	}
#	endif
	op64(ctx, POP, PEBP, UNUSED);
	op64(ctx, RET, UNUSED, UNUSED);
}

static void call_native_consts(jit_ctx* ctx, void* nativeFun, int_val* args, int nargs) {
	int size = pad_before_call(ctx, IS_64 ? 0 : HL_WSIZE * nargs);
	preg p;
	int i;
#	ifdef HL_64
	for (i = 0;i < nargs;i++)
		op64(ctx, MOV, REG_AT(CALL_REGS[i]), pconst64(&p, args[i]));
#	else
	for (i = nargs - 1;i >= 0;i--)
		op32(ctx, PUSH, pconst64(&p, args[i]), UNUSED);
#	endif
	call_native(ctx, nativeFun, size);
}

static void on_jit_error(const char* msg, int_val line) {
	char buf[256];
	int iline = (int)line;
	sprintf(buf, "%s (line %d)", msg, iline);
#ifdef HL_WIN_DESKTOP
	MessageBoxA(NULL, buf, "JIT ERROR", MB_OK);
#else
	printf("JIT ERROR : %s\n", buf);
#endif
	hl_debug_break();
	hl_throw(NULL);
}

static void _jit_error(jit_ctx* ctx, const char* msg, int line) {
	int_val args[2] = { (int_val)msg, (int_val)line };
	call_native_consts(ctx, on_jit_error, args, 2);
}


static preg* op_binop(jit_ctx* ctx, vreg* dst, vreg* a, vreg* b, hl_op bop) {
	preg* pa = fetch(a), * pb = fetch(b), * out = NULL;
	CpuOp o;
	if (IS_FLOAT(a)) {
		bool isf32 = a->t->kind == HF32;
		switch (bop) {
		case OAdd: o = isf32 ? ADDSS : ADDSD; break;
		case OSub: o = isf32 ? SUBSS : SUBSD; break;
		case OMul: o = isf32 ? MULSS : MULSD; break;
		case OSDiv: o = isf32 ? DIVSS : DIVSD; break;
		case OJSLt:
		case OJSGte:
		case OJSLte:
		case OJSGt:
		case OJEq:
		case OJNotEq:
		case OJNotLt:
		case OJNotGte:
			o = isf32 ? COMISS : COMISD;
			break;
		case OSMod:
		{
			int args[] = { a->stack.id, b->stack.id };
			int size = prepare_call_args(ctx, 2, args, ctx->vregs, 0);
			void* mod_fun;
			if (isf32)
				mod_fun = static_cast<float(*)(float, float)>(fmodf);
			else
				mod_fun = static_cast<double(*)(double, double)>(fmod);
			call_native(ctx, mod_fun, size);
			store_result(ctx, dst);
			return fetch(dst);
		}
		default:
			printf("%s\n", hl_op_name(bop));
			ASSERT(bop);
		}
	}
	else {
		bool is64 = a->t->kind == HI64;
#	ifndef HL_64
		if (is64) {
			error_i64();
			return fetch(a);
		}
#	endif
		switch (bop) {
		case OAdd: o = ADD; break;
		case OSub: o = SUB; break;
		case OMul: o = IMUL; break;
		case OAnd: o = AND; break;
		case OOr: o = OR; break;
		case OXor: o = XOR; break;
		case OShl:
		case OUShr:
		case OSShr:
			if (!b->current || b->current->kind != RCPU || b->current->id != Ecx) {
				scratch(REG_AT(Ecx));
				op(ctx, MOV, REG_AT(Ecx), pb, is64);
				RLOCK(REG_AT(Ecx));
				pa = fetch(a);
			}
			else
				RLOCK(b->current);
			if (pa->kind != RCPU) {
				pa = alloc_reg(ctx, RCPU);
				op(ctx, MOV, pa, fetch(a), is64);
			}
			op(ctx, bop == OShl ? SHL : (bop == OUShr ? SHR : SAR), pa, UNUSED, is64);
			if (dst) store(ctx, dst, pa, true);
			return pa;
		case OSDiv:
		case OUDiv:
		case OSMod:
		case OUMod:
		{
			preg* out = bop == OSMod || bop == OUMod ? REG_AT(Edx) : PEAX;
			preg* r;
			preg p;
			int jz, jz1 = 0, jend;
			if (pa->kind == RCPU && pa->id == Eax) RLOCK(pa);
			r = alloc_cpu(ctx, b, true);
			// integer div 0 => 0
			op(ctx, TEST, r, r, is64);
			XJump_small(JZero, jz);
			// Prevent MIN/-1 overflow exception
			// OSMod: r = (b == 0 || b == -1) ? 0 : a % b
			// OSDiv: r = (b == 0 || b == -1) ? a * b : a / b
			if (bop == OSMod || bop == OSDiv) {
				op(ctx, CMP, r, pconst(&p, -1), is64);
				XJump_small(JEq, jz1);
			}
			pa = fetch(a);
			if (pa->kind != RCPU || pa->id != Eax) {
				scratch(PEAX);
				scratch(pa);
				load(ctx, PEAX, a);
			}
			scratch(REG_AT(Edx));
			scratch(REG_AT(Eax));
			if (bop == OUDiv || bop == OUMod)
				op(ctx, XOR, REG_AT(Edx), REG_AT(Edx), is64);
			else
				op(ctx, CDQ, UNUSED, UNUSED, is64); // sign-extend Eax into Eax:Edx
			op(ctx, bop == OUDiv || bop == OUMod ? DIV : IDIV, fetch(b), UNUSED, is64);
			XJump_small(JAlways, jend);
			patch_jump(ctx, jz);
			patch_jump(ctx, jz1);
			if (bop != OSDiv) {
				op(ctx, XOR, out, out, is64);
			}
			else {
				load(ctx, out, a);
				op(ctx, IMUL, out, r, is64);
			}
			patch_jump(ctx, jend);
			if (dst) store(ctx, dst, out, true);
			return out;
		}
		case OJSLt:
		case OJSGte:
		case OJSLte:
		case OJSGt:
		case OJULt:
		case OJUGte:
		case OJEq:
		case OJNotEq:
			switch (a->t->kind) {
			case HUI8:
			case HBOOL:
				o = CMP8;
				break;
			case HUI16:
				o = CMP16;
				break;
			default:
				o = CMP;
				break;
			}
			break;
		default:
			printf("%s\n", hl_op_name(bop));
			ASSERT(bop);
		}
	}
	switch (RTYPE(a)) {
	case HI32:
	case HUI8:
	case HUI16:
	case HBOOL:
#	ifndef HL_64
	case HDYNOBJ:
	case HVIRTUAL:
	case HOBJ:
	case HSTRUCT:
	case HFUN:
	case HMETHOD:
	case HBYTES:
	case HNULL:
	case HENUM:
	case HDYN:
	case HTYPE:
	case HABSTRACT:
	case HARRAY:
#	endif
		switch (ID2(pa->kind, pb->kind)) {
		case ID2(RCPU, RCPU):
		case ID2(RCPU, RSTACK):
			op32(ctx, o, pa, pb);
			scratch(pa);
			out = pa;
			break;
		case ID2(RSTACK, RCPU):
			if (dst == a && o != IMUL) {
				op32(ctx, o, pa, pb);
				dst = NULL;
				out = pa;
			}
			else {
				alloc_cpu(ctx, a, true);
				return op_binop(ctx, dst, a, b, bop);
			}
			break;
		case ID2(RSTACK, RSTACK):
			alloc_cpu(ctx, a, true);
			return op_binop(ctx, dst, a, b, bop);
		default:
			printf("%s(%d,%d)\n", hl_op_name(bop), pa->kind, pb->kind);
			ASSERT(ID2(pa->kind, pb->kind));
		}
		if (dst) store(ctx, dst, out, true);
		return out;
#	ifdef HL_64
	case HOBJ:
	case HSTRUCT:
	case HDYNOBJ:
	case HVIRTUAL:
	case HFUN:
	case HMETHOD:
	case HBYTES:
	case HNULL:
	case HENUM:
	case HDYN:
	case HTYPE:
	case HABSTRACT:
	case HARRAY:
	case HI64:
		switch (ID2(pa->kind, pb->kind)) {
		case ID2(RCPU, RCPU):
		case ID2(RCPU, RSTACK):
			op64(ctx, o, pa, pb);
			scratch(pa);
			out = pa;
			break;
		case ID2(RSTACK, RCPU):
			if (dst == a && OP_FORMS[o].mem_r) {
				op64(ctx, o, pa, pb);
				dst = NULL;
				out = pa;
			}
			else {
				alloc_cpu(ctx, a, true);
				return op_binop(ctx, dst, a, b, bop);
			}
			break;
		case ID2(RSTACK, RSTACK):
			alloc_cpu(ctx, a, true);
			return op_binop(ctx, dst, a, b, bop);
		default:
			printf("%s(%d,%d)\n", hl_op_name(bop), pa->kind, pb->kind);
			ASSERT(ID2(pa->kind, pb->kind));
		}
		if (dst) store(ctx, dst, out, true);
		return out;
#	endif
	case HF64:
	case HF32:
		pa = alloc_fpu(ctx, a, true);
		pb = alloc_fpu(ctx, b, true);
		switch (ID2(pa->kind, pb->kind)) {
		case ID2(RFPU, RFPU):
			op64(ctx, o, pa, pb);
			if ((o == COMISD || o == COMISS) && bop != OJSGt) {
				int jnotnan;
				XJump_small(JNParity, jnotnan);
				switch (bop) {
				case OJSLt:
				case OJNotLt:
				{
					preg* r = alloc_reg(ctx, RCPU);
					// set CF=0, ZF=1
					op64(ctx, XOR, r, r);
					RUNLOCK(r);
					break;
				}
				case OJSGte:
				case OJNotGte:
				{
					preg* r = alloc_reg(ctx, RCPU);
					// set ZF=0, CF=1
					op64(ctx, XOR, r, r);
					op64(ctx, CMP, r, PESP);
					RUNLOCK(r);
					break;
				}
				break;
				case OJNotEq:
				case OJEq:
					// set ZF=0, CF=?
				case OJSLte:
					// set ZF=0, CF=0
					op64(ctx, TEST, PESP, PESP);
					break;
				default:
					ASSERT(bop);
				}
				patch_jump(ctx, jnotnan);
			}
			scratch(pa);
			out = pa;
			break;
		default:
			printf("%s(%d,%d)\n", hl_op_name(bop), pa->kind, pb->kind);
			ASSERT(ID2(pa->kind, pb->kind));
		}
		if (dst) store(ctx, dst, out, true);
		return out;
	default:
		ASSERT(RTYPE(a));
	}
	return NULL;
}

static int do_jump(jit_ctx* ctx, hl_op op, bool isFloat) {
	int j;
	switch (op) {
	case OJAlways:
		XJump(JAlways, j);
		break;
	case OJSGte:
		XJump(isFloat ? JUGte : JSGte, j);
		break;
	case OJSGt:
		XJump(isFloat ? JUGt : JSGt, j);
		break;
	case OJUGte:
		XJump(JUGte, j);
		break;
	case OJSLt:
		XJump(isFloat ? JULt : JSLt, j);
		break;
	case OJSLte:
		XJump(isFloat ? JULte : JSLte, j);
		break;
	case OJULt:
		XJump(JULt, j);
		break;
	case OJEq:
		XJump(JEq, j);
		break;
	case OJNotEq:
		XJump(JNeq, j);
		break;
	case OJNotLt:
		XJump(JUGte, j);
		break;
	case OJNotGte:
		XJump(JULt, j);
		break;
	default:
		j = 0;
		printf("Unknown JUMP %d\n", op);
		break;
	}
	return j;
}

static void register_jump(jit_ctx* ctx, int pos, int target) {
	jlist* j = (jlist*)hl_malloc(&ctx->falloc, sizeof(jlist));
	j->pos = pos;
	j->target = target;
	j->next = ctx->jumps;
	ctx->jumps = j;
	if (target != 0 && ctx->opsPos[target] == 0)
		ctx->opsPos[target] = -1;
}

#define HDYN_VALUE 8

static void dyn_value_compare(jit_ctx* ctx, preg* a, preg* b, hl_type* t) {
	preg p;
	switch (t->kind) {
	case HUI8:
	case HBOOL:
		op32(ctx, MOV8, a, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op32(ctx, MOV8, b, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, CMP8, a, b);
		break;
	case HUI16:
		op32(ctx, MOV16, a, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op32(ctx, MOV16, b, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, CMP16, a, b);
		break;
	case HI32:
		op32(ctx, MOV, a, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op32(ctx, MOV, b, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, CMP, a, b);
		break;
	case HF32:
	{
		preg* fa = alloc_reg(ctx, RFPU);
		preg* fb = alloc_reg(ctx, RFPU);
		op64(ctx, MOVSS, fa, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op64(ctx, MOVSS, fb, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, COMISD, fa, fb);
	}
	break;
	case HF64:
	{
		preg* fa = alloc_reg(ctx, RFPU);
		preg* fb = alloc_reg(ctx, RFPU);
		op64(ctx, MOVSD, fa, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op64(ctx, MOVSD, fb, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, COMISD, fa, fb);
	}
	break;
	case HI64:
	default:
		// ptr comparison
		op64(ctx, MOV, a, pmem(&p, static_cast<CpuReg>(a->id), HDYN_VALUE));
		op64(ctx, MOV, b, pmem(&p, static_cast<CpuReg>(b->id), HDYN_VALUE));
		op64(ctx, CMP, a, b);
		break;
	}
}

static void op_jump(jit_ctx* ctx, vreg* a, vreg* b, hl_opcode* op, int targetPos) {
	if (a->t->kind == HDYN || b->t->kind == HDYN || a->t->kind == HFUN || b->t->kind == HFUN) {
		int args[] = { a->stack.id, b->stack.id };
		int size = prepare_call_args(ctx, 2, args, ctx->vregs, 0);
		call_native(ctx, hl_dyn_compare, size);
		if (op->op == OJSGt || op->op == OJSGte) {
			preg p;
			int jinvalid;
			op32(ctx, CMP, PEAX, pconst(&p, hl_invalid_comparison));
			XJump_small(JEq, jinvalid);
			op32(ctx, TEST, PEAX, PEAX);
			register_jump(ctx, do_jump(ctx, op->op, IS_FLOAT(a)), targetPos);
			patch_jump(ctx, jinvalid);
			return;
		}
		op32(ctx, TEST, PEAX, PEAX);
	}
	else switch (a->t->kind) {
	case HTYPE:
	{
		int args[] = { a->stack.id, b->stack.id };
		int size = prepare_call_args(ctx, 2, args, ctx->vregs, 0);
		preg p;
		call_native(ctx, hl_same_type, size);
		op64(ctx, CMP8, PEAX, pconst(&p, 1));
	}
	break;
	case HNULL:
	{
		preg* pa = hl_type_size(a->t->tparam) == 1 ? alloc_cpu8(ctx, a, true) : alloc_cpu(ctx, a, true);
		preg* pb = hl_type_size(b->t->tparam) == 1 ? alloc_cpu8(ctx, b, true) : alloc_cpu(ctx, b, true);
		if (op->op == OJEq) {
			// if( a == b || (a && b && a->v == b->v) ) goto
			int ja, jb;
			// if( a != b && (!a || !b || a->v != b->v) ) goto
			op64(ctx, CMP, pa, pb);
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			op64(ctx, TEST, pa, pa);
			XJump_small(JZero, ja);
			op64(ctx, TEST, pb, pb);
			XJump_small(JZero, jb);
			dyn_value_compare(ctx, pa, pb, a->t->tparam);
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			scratch(pa);
			scratch(pb);
			patch_jump(ctx, ja);
			patch_jump(ctx, jb);
		}
		else if (op->op == OJNotEq) {
			int jeq, jcmp;
			// if( a != b && (!a || !b || a->v != b->v) ) goto
			op64(ctx, CMP, pa, pb);
			XJump_small(JEq, jeq);
			op64(ctx, TEST, pa, pa);
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			op64(ctx, TEST, pb, pb);
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			dyn_value_compare(ctx, pa, pb, a->t->tparam);
			XJump_small(JZero, jcmp);
			scratch(pa);
			scratch(pb);
			register_jump(ctx, do_jump(ctx, OJNotEq, false), targetPos);
			patch_jump(ctx, jcmp);
			patch_jump(ctx, jeq);
		}
		else
			ASSERT(op->op);
		return;
	}
	case HVIRTUAL:
	{
		preg p;
		preg* pa = alloc_cpu(ctx, a, true);
		preg* pb = alloc_cpu(ctx, b, true);
		int ja, jb, jav, jbv, jvalue;
		if (b->t->kind == HOBJ) {
			if (op->op == OJEq) {
				// if( a ? (b && a->value == b) : (b == NULL) ) goto
				op64(ctx, TEST, pa, pa);
				XJump_small(JZero, ja);
				op64(ctx, TEST, pb, pb);
				XJump_small(JZero, jb);
				op64(ctx, MOV, pa, pmem(&p, static_cast<CpuReg>(pa->id), HL_WSIZE));
				op64(ctx, CMP, pa, pb);
				XJump_small(JAlways, jvalue);
				patch_jump(ctx, ja);
				op64(ctx, TEST, pb, pb);
				patch_jump(ctx, jvalue);
				register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
				patch_jump(ctx, jb);
			}
			else if (op->op == OJNotEq) {
				// if( a ? (b == NULL || a->value != b) : (b != NULL) ) goto
				op64(ctx, TEST, pa, pa);
				XJump_small(JZero, ja);
				op64(ctx, TEST, pb, pb);
				register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
				op64(ctx, MOV, pa, pmem(&p, static_cast<CpuReg>(pa->id), HL_WSIZE));
				op64(ctx, CMP, pa, pb);
				XJump_small(JAlways, jvalue);
				patch_jump(ctx, ja);
				op64(ctx, TEST, pb, pb);
				patch_jump(ctx, jvalue);
				register_jump(ctx, do_jump(ctx, OJNotEq, false), targetPos);
			}
			else
				ASSERT(op->op);
			scratch(pa);
			return;
		}
		op64(ctx, CMP, pa, pb);
		if (op->op == OJEq) {
			// if( a == b || (a && b && a->value && b->value && a->value == b->value) ) goto
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			op64(ctx, TEST, pa, pa);
			XJump_small(JZero, ja);
			op64(ctx, TEST, pb, pb);
			XJump_small(JZero, jb);
			op64(ctx, MOV, pa, pmem(&p, static_cast<CpuReg>(pa->id), HL_WSIZE));
			op64(ctx, TEST, pa, pa);
			XJump_small(JZero, jav);
			op64(ctx, MOV, pb, pmem(&p, static_cast<CpuReg>(pb->id), HL_WSIZE));
			op64(ctx, TEST, pb, pb);
			XJump_small(JZero, jbv);
			op64(ctx, CMP, pa, pb);
			XJump_small(JNeq, jvalue);
			register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
			patch_jump(ctx, ja);
			patch_jump(ctx, jb);
			patch_jump(ctx, jav);
			patch_jump(ctx, jbv);
			patch_jump(ctx, jvalue);
		}
		else if (op->op == OJNotEq) {
			int jnext;
			// if( a != b && (!a || !b || !a->value || !b->value || a->value != b->value) ) goto
			XJump_small(JEq, jnext);
			op64(ctx, TEST, pa, pa);
			XJump_small(JZero, ja);
			op64(ctx, TEST, pb, pb);
			XJump_small(JZero, jb);
			op64(ctx, MOV, pa, pmem(&p, static_cast<CpuReg>(pa->id), HL_WSIZE));
			op64(ctx, TEST, pa, pa);
			XJump_small(JZero, jav);
			op64(ctx, MOV, pb, pmem(&p, static_cast<CpuReg>(pb->id), HL_WSIZE));
			op64(ctx, TEST, pb, pb);
			XJump_small(JZero, jbv);
			op64(ctx, CMP, pa, pb);
			XJump_small(JEq, jvalue);
			patch_jump(ctx, ja);
			patch_jump(ctx, jb);
			patch_jump(ctx, jav);
			patch_jump(ctx, jbv);
			register_jump(ctx, do_jump(ctx, OJAlways, false), targetPos);
			patch_jump(ctx, jnext);
			patch_jump(ctx, jvalue);
		}
		else
			ASSERT(op->op);
		scratch(pa);
		scratch(pb);
		return;
	}
	break;
	case HOBJ:
	case HSTRUCT:
		if (b->t->kind == HVIRTUAL) {
			op_jump(ctx, b, a, op, targetPos); // inverse
			return;
		}
		if (hl_get_obj_rt(a->t)->compareFun) {
			preg* pa = alloc_cpu(ctx, a, true);
			preg* pb = alloc_cpu(ctx, b, true);
			preg p;
			int jeq, ja, jb, jcmp;
			int args[] = { a->stack.id, b->stack.id };
			switch (op->op) {
			case OJEq:
				// if( a == b || (a && b && cmp(a,b) == 0) ) goto
				op64(ctx, CMP, pa, pb);
				XJump_small(JEq, jeq);
				op64(ctx, TEST, pa, pa);
				XJump_small(JZero, ja);
				op64(ctx, TEST, pb, pb);
				XJump_small(JZero, jb);
				op_call_fun(ctx, NULL, (int)(int_val)a->t->obj->rt->compareFun, 2, args);
				op32(ctx, TEST, PEAX, PEAX);
				XJump_small(JNotZero, jcmp);
				patch_jump(ctx, jeq);
				register_jump(ctx, do_jump(ctx, OJAlways, false), targetPos);
				patch_jump(ctx, ja);
				patch_jump(ctx, jb);
				patch_jump(ctx, jcmp);
				break;
			case OJNotEq:
				// if( a != b && (!a || !b || cmp(a,b) != 0) ) goto
				op64(ctx, CMP, pa, pb);
				XJump_small(JEq, jeq);
				op64(ctx, TEST, pa, pa);
				register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);
				op64(ctx, TEST, pb, pb);
				register_jump(ctx, do_jump(ctx, OJEq, false), targetPos);

				op_call_fun(ctx, NULL, (int)(int_val)a->t->obj->rt->compareFun, 2, args);
				op32(ctx, TEST, PEAX, PEAX);
				XJump_small(JZero, jcmp);

				register_jump(ctx, do_jump(ctx, OJNotEq, false), targetPos);
				patch_jump(ctx, jcmp);
				patch_jump(ctx, jeq);
				break;
			default:
				// if( a && b && cmp(a,b) ?? 0 ) goto
				op64(ctx, TEST, pa, pa);
				XJump_small(JZero, ja);
				op64(ctx, TEST, pb, pb);
				XJump_small(JZero, jb);
				op_call_fun(ctx, NULL, (int)(int_val)a->t->obj->rt->compareFun, 2, args);
				op32(ctx, CMP, PEAX, pconst(&p, 0));
				register_jump(ctx, do_jump(ctx, op->op, false), targetPos);
				patch_jump(ctx, ja);
				patch_jump(ctx, jb);
				break;
			}
			return;
		}
		// fallthrough
	default:
		// make sure we have valid 8 bits registers
		if (a->size == 1) alloc_cpu8(ctx, a, true);
		if (b->size == 1) alloc_cpu8(ctx, b, true);
		op_binop(ctx, NULL, a, b, op->op);
		break;
	}
	register_jump(ctx, do_jump(ctx, op->op, IS_FLOAT(a)), targetPos);
}

jit_ctx* hl_jit_alloc() {
	int i;
	jit_ctx* ctx = (jit_ctx*)malloc(sizeof(jit_ctx));
	if (ctx == NULL) return NULL;
	memset(ctx, 0, sizeof(jit_ctx));
	hl_alloc_init(&ctx->falloc);
	hl_alloc_init(&ctx->galloc);
	for (i = 0;i < RCPU_COUNT;i++) {
		preg* r = REG_AT(i);
		r->id = i;
		r->kind = RCPU;
	}
	for (i = 0;i < RFPU_COUNT;i++) {
		preg* r = REG_AT(XMM(i));
		r->id = i;
		r->kind = RFPU;
	}
	return ctx;
}

void hl_jit_free(jit_ctx* ctx, h_bool can_reset) {
	free(ctx->vregs);
	free(ctx->opsPos);
	free(ctx->startBuf);
	ctx->maxRegs = 0;
	ctx->vregs = NULL;
	ctx->maxOps = 0;
	ctx->opsPos = NULL;
	ctx->startBuf = NULL;
	ctx->bufSize = 0;
	ctx->buf.b = NULL;
	ctx->calls = NULL;
	ctx->switchs = NULL;
	ctx->closure_list = NULL;
	hl_free(&ctx->falloc);
	hl_free(&ctx->galloc);
	if (!can_reset) free(ctx);
}

static void jit_nops(jit_ctx* ctx) {
	while (BUF_POS() & 15)
		op32(ctx, NOP, UNUSED, UNUSED);
}

#define MAX_ARGS 16

static void* call_jit_c2hl = NULL;
static void* call_jit_hl2c = NULL;

static void* callback_c2hl(void** f, hl_type* t, void** args, vdynamic* ret) {
	/*
		prepare stack and regs according to prepare_call_args, but by reading runtime type information
		from the function type. The stack and regs will be setup by the trampoline function.
	*/
	unsigned char stack[MAX_ARGS * 8];
	call_regs cregs = { 0 };
	if (t->fun->nargs > MAX_ARGS)
		hl_error("Too many arguments for dynamic call");
	int i, size = 0, pad = 0, pos = 0;
	for (i = 0;i < t->fun->nargs;i++) {
		hl_type* at = t->fun->args[i];
		int creg = select_call_reg(&cregs, at, i);
		if (creg >= 0)
			continue;
		size += stack_size(at);
	}
	pad = (-size) & 15;
	size += pad;
	pos = 0;
	for (i = 0;i < t->fun->nargs;i++) {
		// RTL
		hl_type* at = t->fun->args[i];
		void* v = args[i];
		int creg = mapped_reg(&cregs, i);
		void* store;
		if (creg >= 0) {
			if (REG_IS_FPU(creg)) {
				store = stack + size + CALL_NREGS * HL_WSIZE + (creg - XMM(0)) * sizeof(double);
			}
			else {
				store = stack + size + call_reg_index(creg) * HL_WSIZE;
			}
			switch (at->kind) {
			case HBOOL:
			case HUI8:
				*(int_val*)store = *(unsigned char*)v;
				break;
			case HUI16:
				*(int_val*)store = *(unsigned short*)v;
				break;
			case HI32:
				*(int_val*)store = *(int*)v;
				break;
			case HF32:
				*(void**)store = 0;
				*(float*)store = *(float*)v;
				break;
			case HF64:
				*(double*)store = *(double*)v;
				break;
			case HI64:
				*(int64*)store = *(int64*)v;
				break;
			default:
				*(void**)store = v;
				break;
			}
		}
		else {
			int tsize = stack_size(at);
			store = stack + pos;
			pos += tsize;
			switch (at->kind) {
			case HBOOL:
			case HUI8:
				*(int*)store = *(unsigned char*)v;
				break;
			case HUI16:
				*(int*)store = *(unsigned short*)v;
				break;
			case HI32:
			case HF32:
				*(int*)store = *(int*)v;
				break;
			case HF64:
				*(double*)store = *(double*)v;
				break;
			case HI64:
				*(int64*)store = *(int64*)v;
				break;
			default:
				*(void**)store = v;
				break;
			}
		}
	}
	pos += pad;
	pos >>= IS_64 ? 3 : 2;
	switch (t->fun->ret->kind) {
	case HUI8:
	case HUI16:
	case HI32:
	case HBOOL:
		ret->v.i = ((int (*)(void*, void*, void*))call_jit_c2hl)(*f, (void**)&stack + pos, &stack);
		return &ret->v.i;
	case HI64:
		ret->v.i64 = ((int64(*)(void*, void*, void*))call_jit_c2hl)(*f, (void**)&stack + pos, &stack);
		return &ret->v.i64;
	case HF32:
		ret->v.f = ((float (*)(void*, void*, void*))call_jit_c2hl)(*f, (void**)&stack + pos, &stack);
		return &ret->v.f;
	case HF64:
		ret->v.d = ((double (*)(void*, void*, void*))call_jit_c2hl)(*f, (void**)&stack + pos, &stack);
		return &ret->v.d;
	default:
		return ((void* (*)(void*, void*, void*))call_jit_c2hl)(*f, (void**)&stack + pos, &stack);
	}
}

static void jit_c2hl(jit_ctx* ctx) {
	//	create the function that will be called by callback_c2hl
	//	it will make sure to prepare the stack/regs according to native calling conventions
	int jeq, jloop, jstart;
	preg* fptr, * stack, * stend;
	preg p;

	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);

#	ifdef HL_64

	fptr = REG_AT(R10);
	stack = PEAX;
	stend = REG_AT(R11);
	op64(ctx, MOV, fptr, REG_AT(CALL_REGS[0]));
	op64(ctx, MOV, stack, REG_AT(CALL_REGS[1]));
	op64(ctx, MOV, stend, REG_AT(CALL_REGS[2]));

	// set native call regs
	int i;
	for (i = 0;i < CALL_NREGS;i++)
		op64(ctx, MOV, REG_AT(CALL_REGS[i]), pmem(&p, static_cast<CpuReg>(stack->id), i * HL_WSIZE));
	for (i = 0;i < CALL_NREGS;i++)
		op64(ctx, MOVSD, REG_AT(XMM(i)), pmem(&p, static_cast<CpuReg>(stack->id), (i + CALL_NREGS) * HL_WSIZE));

#	else

	// make sure the stack is aligned on 16 bytes
	// the amount of push we will do afterwards is guaranteed to be a multiple of 16bytes by hl_callback
#	ifdef HL_VCC
	// VCC does not guarantee us an aligned stack...
	op64(ctx, MOV, PEAX, PESP);
	op64(ctx, AND, PEAX, pconst(&p, 15));
	op64(ctx, SUB, PESP, PEAX);
#	else
	op64(ctx, SUB, PESP, pconst(&p, 8));
#	endif

	// mov arguments to regs
	fptr = REG_AT(Eax);
	stack = REG_AT(Edx);
	stend = REG_AT(Ecx);
	op64(ctx, MOV, fptr, pmem(&p, Ebp, HL_WSIZE * 2));
	op64(ctx, MOV, stack, pmem(&p, Ebp, HL_WSIZE * 3));
	op64(ctx, MOV, stend, pmem(&p, Ebp, HL_WSIZE * 4));

#	endif

	// push stack args
	jstart = BUF_POS();
	op64(ctx, CMP, stack, stend);
	XJump(JEq, jeq);
	op64(ctx, SUB, stack, pconst(&p, HL_WSIZE));
	op64(ctx, PUSH, pmem(&p, static_cast<CpuReg>(stack->id), 0), UNUSED);
	XJump(JAlways, jloop);
	patch_jump(ctx, jeq);
	patch_jump_to(ctx, jloop, jstart);

	op_call(ctx, fptr, 0);

	// cleanup and ret
	op64(ctx, MOV, PESP, PEBP);
	op64(ctx, POP, PEBP, UNUSED);
	op64(ctx, RET, UNUSED, UNUSED);
}

static vdynamic* jit_wrapper_call(vclosure_wrapper* c, char* stack_args, void** regs) {
	vdynamic* args[MAX_ARGS];
	int i;
	int nargs = c->cl.t->fun->nargs;
	call_regs cregs = { 0 };
	if (nargs > MAX_ARGS)
		hl_error("Too many arguments for wrapped call");
	cregs.nextCpu++; // skip fptr in HL64 - was passed as arg0
	for (i = 0;i < nargs;i++) {
		hl_type* t = c->cl.t->fun->args[i];
		int creg = select_call_reg(&cregs, t, i);
		if (creg < 0) {
			args[i] = hl_is_dynamic(t) ? *(vdynamic**)stack_args : hl_make_dyn(stack_args, t);
			stack_args += stack_size(t);
		}
		else if (hl_is_dynamic(t)) {
			args[i] = *(vdynamic**)(regs + call_reg_index(creg));
		}
		else if (t->kind == HF32 || t->kind == HF64) {
			args[i] = hl_make_dyn(regs + CALL_NREGS + creg - XMM(0), &hlt_f64);
		}
		else {
			args[i] = hl_make_dyn(regs + call_reg_index(creg), t);
		}
	}
	return hl_dyn_call(c->wrappedFun, args, nargs);
}

static void* jit_wrapper_ptr(vclosure_wrapper* c, char* stack_args, void** regs) {
	vdynamic* ret = jit_wrapper_call(c, stack_args, regs);
	hl_type* tret = c->cl.t->fun->ret;
	switch (tret->kind) {
	case HVOID:
		return NULL;
	case HUI8:
	case HUI16:
	case HI32:
	case HBOOL:
		return (void*)(int_val)hl_dyn_casti(&ret, &hlt_dyn, tret);
	case HI64:
		return (void*)(int_val)hl_dyn_casti64(&ret, &hlt_dyn);
	default:
		return hl_dyn_castp(&ret, &hlt_dyn, tret);
	}
}

static double jit_wrapper_d(vclosure_wrapper* c, char* stack_args, void** regs) {
	vdynamic* ret = jit_wrapper_call(c, stack_args, regs);
	return hl_dyn_castd(&ret, &hlt_dyn);
}

static void jit_hl2c(jit_ctx* ctx) {
	// create a function that is called with a vclosure_wrapper* and native args
	// and pack and pass the args to callback_hl2c
	preg p;
	int jfloat1, jfloat2, jexit;
	hl_type_fun* ft = NULL;
	int size;
#	ifdef HL_64
	preg* cl = REG_AT(CALL_REGS[0]);
	preg* tmp = REG_AT(CALL_REGS[1]);
#	else
	preg* cl = REG_AT(Ecx);
	preg* tmp = REG_AT(Edx);
#	endif

	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);

#	ifdef HL_64
	// push registers
	int i;
	op64(ctx, SUB, PESP, pconst(&p, CALL_NREGS * 8));
	for (i = 0;i < CALL_NREGS;i++)
		op64(ctx, MOVSD, pmem(&p, Esp, i * 8), REG_AT(XMM(i)));
	for (i = 0;i < CALL_NREGS;i++)
		op64(ctx, PUSH, REG_AT(CALL_REGS[CALL_NREGS - 1 - i]), UNUSED);
#	endif

	// opcodes for:
	//		switch( arg0->t->fun->ret->kind ) {
	//		case HF32: case HF64: return jit_wrapper_d(arg0,&args);
	//		default: return jit_wrapper_ptr(arg0,&args);
	//		}
	if (!IS_64)
		op64(ctx, MOV, cl, pmem(&p, Ebp, HL_WSIZE * 2)); // load arg0
	op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(cl->id), 0)); // ->t
	op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), HL_WSIZE)); // ->fun
	op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), (int)(int_val)&ft->ret)); // ->ret
	op32(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), 0)); // -> kind

	op32(ctx, CMP, tmp, pconst(&p, HF64));
	XJump_small(JEq, jfloat1);
	op32(ctx, CMP, tmp, pconst(&p, HF32));
	XJump_small(JEq, jfloat2);

	// 64 bits : ESP + EIP (+WIN64PAD)
	// 32 bits : ESP + EIP + PARAM0
	int args_pos = IS_64 ? ((IS_WINCALL64 ? 32 : 0) + HL_WSIZE * 2) : (HL_WSIZE * 3);

	size = begin_native_call(ctx, 3);
	op64(ctx, LEA, tmp, pmem(&p, Ebp, -HL_WSIZE * CALL_NREGS * 2));
	set_native_arg(ctx, tmp);
	op64(ctx, LEA, tmp, pmem(&p, Ebp, args_pos));
	set_native_arg(ctx, tmp);
	set_native_arg(ctx, cl);
	call_native(ctx, jit_wrapper_ptr, size);
	XJump_small(JAlways, jexit);

	patch_jump(ctx, jfloat1);
	patch_jump(ctx, jfloat2);
	size = begin_native_call(ctx, 3);
	op64(ctx, LEA, tmp, pmem(&p, Ebp, -HL_WSIZE * CALL_NREGS * 2));
	set_native_arg(ctx, tmp);
	op64(ctx, LEA, tmp, pmem(&p, Ebp, args_pos));
	set_native_arg(ctx, tmp);
	set_native_arg(ctx, cl);
	call_native(ctx, jit_wrapper_d, size);

	patch_jump(ctx, jexit);
	op64(ctx, MOV, PESP, PEBP);
	op64(ctx, POP, PEBP, UNUSED);
	op64(ctx, RET, UNUSED, UNUSED);
}

#ifdef JIT_CUSTOM_LONGJUMP
// Win64 debug CRT performs a Rtl stack check in debug mode, preventing from
// using longjump. This in an alternate implementation that follows the native
// setjump storage.
//
// Another more reliable way of handling this would be to use RtlAddFunctionTable
// but this would require complex creation of unwind info
static void jit_longjump(jit_ctx* ctx) {
	preg* buf = REG_AT(CALL_REGS[0]);
	preg* ret = REG_AT(CALL_REGS[1]);
	preg p;
	int i;
	op64(ctx, MOV, PEAX, ret); // return value
	op64(ctx, MOV, REG_AT(Edx), pmem(&p, static_cast<CpuReg>(buf->id), 0x0));
	op64(ctx, MOV, REG_AT(Ebx), pmem(&p, static_cast<CpuReg>(buf->id), 0x8));
	op64(ctx, MOV, REG_AT(Esp), pmem(&p, static_cast<CpuReg>(buf->id), 0x10));
	op64(ctx, MOV, REG_AT(Ebp), pmem(&p, static_cast<CpuReg>(buf->id), 0x18));
	op64(ctx, MOV, REG_AT(Esi), pmem(&p, static_cast<CpuReg>(buf->id), 0x20));
	op64(ctx, MOV, REG_AT(Edi), pmem(&p, static_cast<CpuReg>(buf->id), 0x28));
	op64(ctx, MOV, REG_AT(R12), pmem(&p, static_cast<CpuReg>(buf->id), 0x30));
	op64(ctx, MOV, REG_AT(R13), pmem(&p, static_cast<CpuReg>(buf->id), 0x38));
	op64(ctx, MOV, REG_AT(R14), pmem(&p, static_cast<CpuReg>(buf->id), 0x40));
	op64(ctx, MOV, REG_AT(R15), pmem(&p, static_cast<CpuReg>(buf->id), 0x48));
	op64(ctx, LDMXCSR, pmem(&p, static_cast<CpuReg>(buf->id), 0x58), UNUSED);
	op64(ctx, FLDCW, pmem(&p, static_cast<CpuReg>(buf->id), 0x5C), UNUSED);
	for (i = 0;i < 10;i++)
		op64(ctx, MOVSD, REG_AT(XMM(i + 6)), pmem(&p, static_cast<CpuReg>(buf->id), 0x60 + i * 16));
	op64(ctx, PUSH, pmem(&p, static_cast<CpuReg>(buf->id), 0x50), UNUSED);
	op64(ctx, RET, UNUSED, UNUSED);
}
#endif

static void jit_fail(uchar* msg) {
	if (msg == NULL) {
		hl_debug_break();
		msg = const_cast<wchar_t*>(USTR("assert"));
	}
	vdynamic* d = hl_alloc_dynamic(&hlt_bytes);
	d->v.ptr = msg;
	hl_throw(d);
}

static void jit_null_access(jit_ctx* ctx) {
	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);
	int_val arg = (int_val)USTR("Null access");
	call_native_consts(ctx, jit_fail, &arg, 1);
}

static void jit_null_fail(int fhash) {
	vbyte* field = hl_field_name(fhash);
	hl_buffer* b = hl_alloc_buffer();
	hl_buffer_str(b, USTR("Null access ."));
	hl_buffer_str(b, (uchar*)field);
	vdynamic* d = hl_alloc_dynamic(&hlt_bytes);
	d->v.ptr = hl_buffer_content(b, NULL);
	hl_throw(d);
}

static void jit_null_field_access(jit_ctx* ctx) {
	preg p;
	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);
	int size = begin_native_call(ctx, 1);
	int args_pos = (IS_WINCALL64 ? 32 : 0) + HL_WSIZE * 2;
	set_native_arg(ctx, pmem(&p, Ebp, args_pos));
	call_native(ctx, jit_null_fail, size);
}

static void jit_assert(jit_ctx* ctx) {
	op64(ctx, PUSH, PEBP, UNUSED);
	op64(ctx, MOV, PEBP, PESP);
	int_val arg = 0;
	call_native_consts(ctx, jit_fail, &arg, 1);
}

static int jit_build(jit_ctx* ctx, void (*fbuild)(jit_ctx*)) {
	int pos;
	jit_buf(ctx);
	jit_nops(ctx);
	pos = BUF_POS();
	fbuild(ctx);
	jit_nops(ctx);
	return pos;
}

static void hl_jit_init_module(jit_ctx* ctx, hl_module* m) {
	int i;
	ctx->m = m;
	if (m->code->hasdebug) {
		ctx->debug = (hl_debug_infos*)malloc(sizeof(hl_debug_infos) * m->code->nfunctions);
		memset(ctx->debug, -1, sizeof(hl_debug_infos) * m->code->nfunctions);
	}
	for (i = 0;i < m->code->nfloats;i++) {
		jit_buf(ctx);
		*ctx->buf.d++ = m->code->floats[i];
	}
}

void hl_jit_init(jit_ctx* ctx, hl_module* m) {
	hl_jit_init_module(ctx, m);
	ctx->c2hl = jit_build(ctx, jit_c2hl);
	ctx->hl2c = jit_build(ctx, jit_hl2c);
#	ifdef JIT_CUSTOM_LONGJUMP
	ctx->longjump = jit_build(ctx, jit_longjump);
#	endif
	ctx->static_functions[0] = (void*)(int_val)jit_build(ctx, jit_null_access);
	ctx->static_functions[1] = (void*)(int_val)jit_build(ctx, jit_assert);
	ctx->static_functions[2] = (void*)(int_val)jit_build(ctx, jit_null_field_access);
}

void hl_jit_reset(jit_ctx* ctx, hl_module* m) {
	ctx->debug = NULL;
	hl_jit_init_module(ctx, m);
}

static void* get_dyncast(hl_type* t) {
	switch (t->kind) {
	case HF32:
		return hl_dyn_castf;
	case HF64:
		return hl_dyn_castd;
	case HI64:
		return hl_dyn_casti64;
	case HI32:
	case HUI16:
	case HUI8:
	case HBOOL:
		return hl_dyn_casti;
	default:
		return hl_dyn_castp;
	}
}

static void* get_dynset(hl_type* t) {
	switch (t->kind) {
	case HF32:
		return hl_dyn_setf;
	case HF64:
		return hl_dyn_setd;
	case HI64:
		return hl_dyn_seti64;
	case HI32:
	case HUI16:
	case HUI8:
	case HBOOL:
		return hl_dyn_seti;
	default:
		return hl_dyn_setp;
	}
}

static void* get_dynget(hl_type* t) {
	switch (t->kind) {
	case HF32:
		return hl_dyn_getf;
	case HF64:
		return hl_dyn_getd;
	case HI64:
		return hl_dyn_geti64;
	case HI32:
	case HUI16:
	case HUI8:
	case HBOOL:
		return hl_dyn_geti;
	default:
		return hl_dyn_getp;
	}
}

static double uint_to_double(unsigned int v) {
	return v;
}

static vclosure* alloc_static_closure(jit_ctx* ctx, int fid) {
	hl_module* m = ctx->m;
	vclosure* c = static_cast<vclosure*>(hl_malloc(&m->ctx.alloc, sizeof(vclosure)));
	int fidx = m->functions_indexes[fid];
	c->hasValue = 0;
	if (fidx >= m->code->nfunctions) {
		// native
		c->t = m->code->natives[fidx - m->code->nfunctions].t;
		c->fun = m->functions_ptrs[fid];
		c->value = NULL;
	}
	else {
		c->t = m->code->functions[fidx].type;
		c->fun = (void*)(int_val)fid;
		c->value = ctx->closure_list;
		ctx->closure_list = c;
	}
	return c;
}

static void make_dyn_cast(jit_ctx* ctx, vreg* dst, vreg* v) {
	int size;
	preg p;
	preg* tmp;
	if (v->t->kind == HNULL && v->t->tparam->kind == dst->t->kind) {
		int jnull, jend;
		preg* out;
		switch (dst->t->kind) {
		case HUI8:
		case HUI16:
		case HI32:
		case HBOOL:
		case HI64:
			tmp = alloc_cpu(ctx, v, true);
			op64(ctx, TEST, tmp, tmp);
			XJump_small(JZero, jnull);
			op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), 8));
			XJump_small(JAlways, jend);
			patch_jump(ctx, jnull);
			op64(ctx, XOR, tmp, tmp);
			patch_jump(ctx, jend);
			store(ctx, dst, tmp, true);
			return;
		case HF32:
		case HF64:
			tmp = alloc_cpu(ctx, v, true);
			out = alloc_fpu(ctx, dst, false);
			op64(ctx, TEST, tmp, tmp);
			XJump_small(JZero, jnull);
			op64(ctx, dst->t->kind == HF32 ? MOVSS : MOVSD, out, pmem(&p, static_cast<CpuReg>(tmp->id), 8));
			XJump_small(JAlways, jend);
			patch_jump(ctx, jnull);
			op64(ctx, XORPD, out, out);
			patch_jump(ctx, jend);
			store(ctx, dst, out, true);
			return;
		default:
			break;
		}
	}
	switch (dst->t->kind) {
	case HF32:
	case HF64:
	case HI64:
		size = begin_native_call(ctx, 2);
		set_native_arg(ctx, pconst64(&p, (int_val)v->t));
		break;
	default:
		size = begin_native_call(ctx, 3);
		set_native_arg(ctx, pconst64(&p, (int_val)dst->t));
		set_native_arg(ctx, pconst64(&p, (int_val)v->t));
		break;
	}
	tmp = alloc_native_arg(ctx);
	op64(ctx, MOV, tmp, REG_AT(Ebp));
	if (v->stackPos >= 0)
		op64(ctx, ADD, tmp, pconst(&p, v->stackPos));
	else
		op64(ctx, SUB, tmp, pconst(&p, -v->stackPos));
	set_native_arg(ctx, tmp);
	call_native(ctx, get_dyncast(dst->t), size);
	store_result(ctx, dst);
}

int hl_jit_function(jit_ctx* ctx, hl_module* m, hl_function* f) {
	int i, size = 0, opCount;
	int codePos = BUF_POS();
	int nargs = f->type->fun->nargs;
	unsigned short* debug16 = NULL;
	int* debug32 = NULL;
	call_regs cregs = { 0 };
	hl_thread_info* tinf = NULL;
	preg p;
	ctx->f = f;
	ctx->allocOffset = 0;
	if (f->nregs > ctx->maxRegs) {
		free(ctx->vregs);
		ctx->vregs = (vreg*)malloc(sizeof(vreg) * (f->nregs + 1));
		if (ctx->vregs == NULL) {
			ctx->maxRegs = 0;
			return -1;
		}
		ctx->maxRegs = f->nregs;
	}
	if (f->nops > ctx->maxOps) {
		free(ctx->opsPos);
		ctx->opsPos = (int*)malloc(sizeof(int) * (f->nops + 1));
		if (ctx->opsPos == NULL) {
			ctx->maxOps = 0;
			return -1;
		}
		ctx->maxOps = f->nops;
	}
	memset(ctx->opsPos, 0, (f->nops + 1) * sizeof(int));
	for (i = 0;i < f->nregs;i++) {
		vreg* r = R(i);
		r->t = f->regs[i];
		r->size = hl_type_size(r->t);
		r->current = NULL;
		r->stack.holds = NULL;
		r->stack.id = i;
		r->stack.kind = RSTACK;
	}
	size = 0;
	int argsSize = 0;
	for (i = 0;i < nargs;i++) {
		vreg* r = R(i);
		int creg = select_call_reg(&cregs, r->t, i);
		if (creg < 0 || IS_WINCALL64) {
			// use existing stack storage
			r->stackPos = argsSize + HL_WSIZE * 2;
			argsSize += stack_size(r->t);
		}
		else {
			// make room in local vars
			size += r->size;
			size += hl_pad_size(size, r->t);
			r->stackPos = -size;
		}
	}
	for (i = nargs;i < f->nregs;i++) {
		vreg* r = R(i);
		size += r->size;
		size += hl_pad_size(size, r->t); // align local vars
		r->stackPos = -size;
	}
#	ifdef HL_64
	size += (-size) & 15; // align on 16 bytes
#	else
	size += hl_pad_size(size, &hlt_dyn); // align on word size
#	endif
	ctx->totalRegsSize = size;
	jit_buf(ctx);
	ctx->functionPos = BUF_POS();
	op_enter(ctx);
#	ifdef HL_64
	{
		// store in local var
		for (i = 0;i < nargs;i++) {
			vreg* r = R(i);
			preg* p;
			int reg = mapped_reg(&cregs, i);
			if (reg < 0) continue;
			p = REG_AT(reg);
			copy(ctx, fetch(r), p, r->size);
			p->holds = r;
			r->current = p;
		}
	}
#	endif
	if (ctx->m->code->hasdebug) {
		debug16 = (unsigned short*)malloc(sizeof(unsigned short) * (f->nops + 1));
		debug16[0] = (unsigned short)(BUF_POS() - codePos);
	}
	ctx->opsPos[0] = BUF_POS();

	for (opCount = 0;opCount < f->nops;opCount++) {
		int jump;
		hl_opcode* o = f->ops + opCount;
		vreg* dst = R(o->p1);
		vreg* ra = R(o->p2);
		vreg* rb = R(o->p3);
		ctx->currentPos = opCount + 1;
		jit_buf(ctx);
#		ifdef JIT_DEBUG
		if (opCount == 0 || f->ops[opCount - 1].op != OAsm) {
			int uid = opCount + (f->findex << 16);
			op32(ctx, PUSH, pconst(&p, uid), UNUSED);
			op64(ctx, ADD, PESP, pconst(&p, HL_WSIZE));
		}
#		endif
		// emit code
		switch (o->op) {
		case OMov:
		case OUnsafeCast:
			op_mov(ctx, dst, ra);
			break;
		case OInt:
			store_const(ctx, dst, m->code->ints[o->p2]);
			break;
		case OBool:
			store_const(ctx, dst, o->p2);
			break;
		case OGetGlobal:
		{
			void* addr = m->globals_data + m->globals_indexes[o->p2];
#				ifdef HL_64
			preg* tmp = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, tmp, pconst64(&p, (int_val)addr));
			copy_to(ctx, dst, pmem(&p, static_cast<CpuReg>(tmp->id), 0));
#				else
			copy_to(ctx, dst, paddr(&p, addr));
#				endif
		}
		break;
		case OSetGlobal:
		{
			void* addr = m->globals_data + m->globals_indexes[o->p1];
#				ifdef HL_64
			preg* tmp = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, tmp, pconst64(&p, (int_val)addr));
			copy_from(ctx, pmem(&p, static_cast<CpuReg>(tmp->id), 0), ra);
#				else
			copy_from(ctx, paddr(&p, addr), ra);
#				endif
		}
		break;
		case OCall3:
		{
			int args[3] = { o->p3, o->extra[0], o->extra[1] };
			op_call_fun(ctx, dst, o->p2, 3, args);
		}
		break;
		case OCall4:
		{
			int args[4] = { o->p3, o->extra[0], o->extra[1], o->extra[2] };
			op_call_fun(ctx, dst, o->p2, 4, args);
		}
		break;
		case OCallN:
			op_call_fun(ctx, dst, o->p2, o->p3, o->extra);
			break;
		case OCall0:
			op_call_fun(ctx, dst, o->p2, 0, NULL);
			break;
		case OCall1:
			op_call_fun(ctx, dst, o->p2, 1, &o->p3);
			break;
		case OCall2:
		{
			int args[2] = { o->p3, (int)(int_val)o->extra };
			op_call_fun(ctx, dst, o->p2, 2, args);
		}
		break;
		case OSub:
		case OAdd:
		case OMul:
		case OSDiv:
		case OUDiv:
		case OShl:
		case OSShr:
		case OUShr:
		case OAnd:
		case OOr:
		case OXor:
		case OSMod:
		case OUMod:
			op_binop(ctx, dst, ra, rb, o->op);
			break;
		case ONeg:
		{
			if (IS_FLOAT(ra)) {
				preg* pa = alloc_reg(ctx, RFPU);
				preg* pb = alloc_fpu(ctx, ra, true);
				op64(ctx, XORPD, pa, pa);
				op64(ctx, ra->t->kind == HF32 ? SUBSS : SUBSD, pa, pb);
				store(ctx, dst, pa, true);
			}
			else if (ra->t->kind == HI64) {
#					ifdef HL_64
				preg* pa = alloc_reg(ctx, RCPU);
				preg* pb = alloc_cpu(ctx, ra, true);
				op64(ctx, XOR, pa, pa);
				op64(ctx, SUB, pa, pb);
				store(ctx, dst, pa, true);
#					else
				error_i64();
#					endif
			}
			else {
				preg* pa = alloc_reg(ctx, RCPU);
				preg* pb = alloc_cpu(ctx, ra, true);
				op32(ctx, XOR, pa, pa);
				op32(ctx, SUB, pa, pb);
				store(ctx, dst, pa, true);
			}
		}
		break;
		case ONot:
		{
			preg* v = alloc_cpu(ctx, ra, true);
			op32(ctx, XOR, v, pconst(&p, 1));
			store(ctx, dst, v, true);
		}
		break;
		case OJFalse:
		case OJTrue:
		case OJNotNull:
		case OJNull:
		{
			preg* r = dst->t->kind == HBOOL ? alloc_cpu8(ctx, dst, true) : alloc_cpu(ctx, dst, true);
			op64(ctx, dst->t->kind == HBOOL ? TEST8 : TEST, r, r);
			XJump(o->op == OJFalse || o->op == OJNull ? JZero : JNotZero, jump);
			register_jump(ctx, jump, (opCount + 1) + o->p2);
		}
		break;
		case OJEq:
		case OJNotEq:
		case OJSLt:
		case OJSGte:
		case OJSLte:
		case OJSGt:
		case OJULt:
		case OJUGte:
		case OJNotLt:
		case OJNotGte:
			op_jump(ctx, dst, ra, o, (opCount + 1) + o->p3);
			break;
		case OJAlways:
			jump = do_jump(ctx, o->op, false);
			register_jump(ctx, jump, (opCount + 1) + o->p1);
			break;
		case OToDyn:
			if (ra->t->kind == HBOOL) {
				int size = begin_native_call(ctx, 1);
				set_native_arg(ctx, fetch(ra));
				call_native(ctx, hl_alloc_dynbool, size);
				store(ctx, dst, PEAX, true);
			}
			else {
				int_val rt = (int_val)ra->t;
				int jskip = 0;
				if (hl_is_ptr(ra->t)) {
					int jnz;
					preg* a = alloc_cpu(ctx, ra, true);
					op64(ctx, TEST, a, a);
					XJump_small(JNotZero, jnz);
					op64(ctx, XOR, PEAX, PEAX); // will replace the result of alloc_dynamic at jump land
					XJump_small(JAlways, jskip);
					patch_jump(ctx, jnz);
				}
				call_native_consts(ctx, hl_alloc_dynamic, &rt, 1);
				// copy value to dynamic
				if ((IS_FLOAT(ra) || ra->size == 8) && !IS_64) {
					preg* tmp = REG_AT(RCPU_SCRATCH_REGS[1]);
					op64(ctx, MOV, tmp, &ra->stack);
					op32(ctx, MOV, pmem(&p, Eax, HDYN_VALUE), tmp);
					if (ra->t->kind == HF64) {
						ra->stackPos += 4;
						op64(ctx, MOV, tmp, &ra->stack);
						op32(ctx, MOV, pmem(&p, Eax, HDYN_VALUE + 4), tmp);
						ra->stackPos -= 4;
					}
				}
				else {
					preg* tmp = REG_AT(RCPU_SCRATCH_REGS[1]);
					copy_from(ctx, tmp, ra);
					op64(ctx, MOV, pmem(&p, Eax, HDYN_VALUE), tmp);
				}
				if (hl_is_ptr(ra->t)) patch_jump(ctx, jskip);
				store(ctx, dst, PEAX, true);
			}
			break;
		case OToSFloat:
			if (ra == dst) break;
			if (ra->t->kind == HI32 || ra->t->kind == HUI16 || ra->t->kind == HUI8) {
				preg* r = alloc_cpu(ctx, ra, true);
				preg* w = alloc_fpu(ctx, dst, false);
				op32(ctx, dst->t->kind == HF64 ? CVTSI2SD : CVTSI2SS, w, r);
				store(ctx, dst, w, true);
			}
			else if (ra->t->kind == HF64 && dst->t->kind == HF32) {
				preg* r = alloc_fpu(ctx, ra, true);
				preg* w = alloc_fpu(ctx, dst, false);
				op32(ctx, CVTSD2SS, w, r);
				store(ctx, dst, w, true);
			}
			else if (ra->t->kind == HF32 && dst->t->kind == HF64) {
				preg* r = alloc_fpu(ctx, ra, true);
				preg* w = alloc_fpu(ctx, dst, false);
				op32(ctx, CVTSS2SD, w, r);
				store(ctx, dst, w, true);
			}
			else
				ASSERT(0);
			break;
		case OToUFloat:
		{
			int size;
			size = prepare_call_args(ctx, 1, &o->p2, ctx->vregs, 0);
			call_native(ctx, uint_to_double, size);
			store_result(ctx, dst);
		}
		break;
		case OToInt:
			if (ra == dst) break;
			if (ra->t->kind == HF64) {
				preg* r = alloc_fpu(ctx, ra, true);
				preg* w = alloc_cpu(ctx, dst, false);
				preg* tmp = alloc_reg(ctx, RCPU);
				op32(ctx, STMXCSR, pmem(&p, Esp, -4), UNUSED);
				op32(ctx, MOV, tmp, &p);
				op32(ctx, OR, tmp, pconst(&p, 0x6000)); // set round towards 0
				op32(ctx, MOV, pmem(&p, Esp, -8), tmp);
				op32(ctx, LDMXCSR, &p, UNUSED);
				op32(ctx, CVTSD2SI, w, r);
				op32(ctx, LDMXCSR, pmem(&p, Esp, -4), UNUSED);
				store(ctx, dst, w, true);
			}
			else if (ra->t->kind == HF32) {
				preg* r = alloc_fpu(ctx, ra, true);
				preg* w = alloc_cpu(ctx, dst, false);
				preg* tmp = alloc_reg(ctx, RCPU);
				op32(ctx, STMXCSR, pmem(&p, Esp, -4), UNUSED);
				op32(ctx, MOV, tmp, &p);
				op32(ctx, OR, tmp, pconst(&p, 0x6000)); // set round towards 0
				op32(ctx, MOV, pmem(&p, Esp, -8), tmp);
				op32(ctx, LDMXCSR, &p, UNUSED);
				op32(ctx, CVTSS2SI, w, r);
				op32(ctx, LDMXCSR, pmem(&p, Esp, -4), UNUSED);
				store(ctx, dst, w, true);
			}
			else if (dst->t->kind == HI64 && ra->t->kind == HI32) {
				if (ra->current != PEAX) {
					op32(ctx, MOV, PEAX, fetch(ra));
					scratch(PEAX);
				}
#				ifdef HL_64
				op64(ctx, CDQE, UNUSED, UNUSED); // sign-extend Eax into Rax
				store(ctx, dst, PEAX, true);
#				else
				op32(ctx, CDQ, UNUSED, UNUSED); // sign-extend Eax into Eax:Edx
				scratch(REG_AT(Edx));
				op32(ctx, MOV, fetch(dst), PEAX);
				dst->stackPos += 4;
				op32(ctx, MOV, fetch(dst), REG_AT(Edx));
				dst->stackPos -= 4;
			}
			else if (dst->t->kind == HI32 && ra->t->kind == HI64) {
				error_i64();
#				endif
			}
			else {
				preg* r = alloc_cpu(ctx, dst, false);
				copy_from(ctx, r, ra);
				store(ctx, dst, r, true);
			}
			break;
		case ORet:
			op_ret(ctx, dst);
			break;
		case OIncr:
		{
			if (IS_FLOAT(dst)) {
				ASSERT(0);
			}
			else {
				preg* v = fetch32(ctx, dst);
				op32(ctx, INC, v, UNUSED);
				if (v->kind != RSTACK) store(ctx, dst, v, false);
			}
		}
		break;
		case ODecr:
		{
			if (IS_FLOAT(dst)) {
				ASSERT(0);
			}
			else {
				preg* v = fetch32(ctx, dst);
				op32(ctx, DEC, v, UNUSED);
				if (v->kind != RSTACK) store(ctx, dst, v, false);
			}
		}
		break;
		case OFloat:
		{
			if (m->code->floats[o->p2] == 0) {
				preg* f = alloc_fpu(ctx, dst, false);
				op64(ctx, XORPD, f, f);
			}
			else switch (dst->t->kind) {
			case HF64:
			case HF32:
#					ifdef HL_64
				op64(ctx, dst->t->kind == HF32 ? CVTSD2SS : MOVSD, alloc_fpu(ctx, dst, false), pcodeaddr(&p, o->p2 * 8));
#					else
				op64(ctx, dst->t->kind == HF32 ? MOVSS : MOVSD, alloc_fpu(ctx, dst, false), paddr(&p, m->code->floats + o->p2));
#					endif
				break;
			default:
				ASSERT(dst->t->kind);
			}
			store(ctx, dst, dst->current, false);
		}
		break;
		case OString:
			op64(ctx, MOV, alloc_cpu(ctx, dst, false), pconst64(&p, (int_val)hl_get_ustring(m->code, o->p2)));
			store(ctx, dst, dst->current, false);
			break;
		case OBytes:
		{
			char* b = m->code->version >= 5 ? m->code->bytes + m->code->bytes_pos[o->p2] : m->code->strings[o->p2];
			op64(ctx, MOV, alloc_cpu(ctx, dst, false), pconst64(&p, (int_val)b));
			store(ctx, dst, dst->current, false);
		}
		break;
		case ONull:
		{
			op64(ctx, XOR, alloc_cpu(ctx, dst, false), alloc_cpu(ctx, dst, false));
			store(ctx, dst, dst->current, false);
		}
		break;
		case ONew:
		{
			int_val args[] = { (int_val)dst->t };
			void* allocFun;
			int nargs = 1;
			switch (dst->t->kind) {
			case HOBJ:
			case HSTRUCT:
				allocFun = hl_alloc_obj;
				break;
			case HDYNOBJ:
				allocFun = hl_alloc_dynobj;
				nargs = 0;
				break;
			case HVIRTUAL:
				allocFun = hl_alloc_virtual;
				break;
			default:
				ASSERT(dst->t->kind);
			}
			call_native_consts(ctx, allocFun, args, nargs);
			store(ctx, dst, PEAX, true);
		}
		break;
		case OInstanceClosure:
		{
			preg* r = alloc_cpu(ctx, rb, true);
			jlist* j = (jlist*)hl_malloc(&ctx->galloc, sizeof(jlist));
			int size = begin_native_call(ctx, 3);
			set_native_arg(ctx, r);

			j->pos = BUF_POS();
			j->target = o->p2;
			j->next = ctx->calls;
			ctx->calls = j;

			set_native_arg(ctx, pconst64(&p, RESERVE_ADDRESS));
			set_native_arg(ctx, pconst64(&p, (int_val)m->code->functions[m->functions_indexes[o->p2]].type));
			call_native(ctx, hl_alloc_closure_ptr, size);
			store(ctx, dst, PEAX, true);
		}
		break;
		case OVirtualClosure:
		{
			int size, i;
			preg* r = alloc_cpu_call(ctx, ra);
			hl_type* t = NULL;
			hl_type* ot = ra->t;
			while (t == NULL) {
				for (i = 0;i < ot->obj->nproto;i++) {
					hl_obj_proto* pp = ot->obj->proto + i;
					if (pp->pindex == o->p3) {
						t = m->code->functions[m->functions_indexes[pp->findex]].type;
						break;
					}
				}
				ot = ot->obj->super;
			}
			size = begin_native_call(ctx, 3);
			set_native_arg(ctx, r);
			// read r->type->vobj_proto[i] for function address
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(r->id), 0));
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(r->id), HL_WSIZE * 2));
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(r->id), HL_WSIZE * o->p3));
			set_native_arg(ctx, r);
			op64(ctx, MOV, r, pconst64(&p, (int_val)t));
			set_native_arg(ctx, r);
			call_native(ctx, hl_alloc_closure_ptr, size);
			store(ctx, dst, PEAX, true);
		}
		break;
		case OCallClosure:
			if (ra->t->kind == HDYN) {
				// ASM for {
				//	vdynamic *args[] = {args};
				//  vdynamic *ret = hl_dyn_call(closure,args,nargs);
				//  dst = hl_dyncast(ret,t_dynamic,t_dst);
				// }
				int offset = o->p3 * HL_WSIZE;
				preg* r = alloc_reg(ctx, RCPU_CALL);
				if (offset & 15) offset += 16 - (offset & 15);
				op64(ctx, SUB, PESP, pconst(&p, offset));
				op64(ctx, MOV, r, PESP);
				for (i = 0;i < o->p3;i++) {
					vreg* a = R(o->extra[i]);
					if (!hl_is_dynamic(a->t)) ASSERT(0);
					preg* v = alloc_cpu(ctx, a, true);
					op64(ctx, MOV, pmem(&p, static_cast<CpuReg>(r->id), i * HL_WSIZE), v);
					RUNLOCK(v);
				}
#				ifdef HL_64
				int size = begin_native_call(ctx, 3) + offset;
				set_native_arg(ctx, pconst(&p, o->p3));
				set_native_arg(ctx, r);
				set_native_arg(ctx, fetch(ra));
#				else
				int size = pad_before_call(ctx, HL_WSIZE * 2 + sizeof(int) + offset);
				op64(ctx, PUSH, pconst(&p, o->p3), UNUSED);
				op64(ctx, PUSH, r, UNUSED);
				op64(ctx, PUSH, alloc_cpu(ctx, ra, true), UNUSED);
#				endif
				call_native(ctx, hl_dyn_call, size);
				if (dst->t->kind != HVOID) {
					store(ctx, dst, PEAX, true);
					make_dyn_cast(ctx, dst, dst);
				}
			}
			else {
				int jhasvalue, jend, size;
				// ASM for  if( c->hasValue ) c->fun(value,args) else c->fun(args)
				preg* r = alloc_cpu(ctx, ra, true);
				preg* tmp = alloc_reg(ctx, RCPU);
				op32(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(r->id), HL_WSIZE * 2));
				op32(ctx, TEST, tmp, tmp);
				scratch(tmp);
				XJump_small(JNotZero, jhasvalue);
				save_regs(ctx);
				size = prepare_call_args(ctx, o->p3, o->extra, ctx->vregs, 0);
				preg* rr = r;
				if (rr->holds != ra) rr = alloc_cpu(ctx, ra, true);
				op_call(ctx, pmem(&p, static_cast<CpuReg>(rr->id), HL_WSIZE), size);
				XJump_small(JAlways, jend);
				patch_jump(ctx, jhasvalue);
				restore_regs(ctx);
#				ifdef HL_64
				{
					int regids[64];
					preg* pc = REG_AT(CALL_REGS[0]);
					vreg* sc = R(f->nregs); // scratch register that we temporary rebind
					if (o->p3 >= 63) jit_error("assert");
					memcpy(regids + 1, o->extra, o->p3 * sizeof(int));
					regids[0] = f->nregs;
					sc->size = HL_WSIZE;
					sc->t = &hlt_dyn;
					op64(ctx, MOV, pc, pmem(&p, static_cast<CpuReg>(r->id), HL_WSIZE * 3));
					scratch(pc);
					sc->current = pc;
					pc->holds = sc;
					size = prepare_call_args(ctx, o->p3 + 1, regids, ctx->vregs, 0);
					if (r->holds != ra) r = alloc_cpu(ctx, ra, true);
				}
#				else
				size = prepare_call_args(ctx, o->p3, o->extra, ctx->vregs, HL_WSIZE);
				if (r->holds != ra) r = alloc_cpu(ctx, ra, true);
				op64(ctx, PUSH, pmem(&p, r->id, HL_WSIZE * 3), UNUSED); // push closure value
#				endif
				op_call(ctx, pmem(&p, static_cast<CpuReg>(r->id), HL_WSIZE), size);
				discard_regs(ctx, false);
				patch_jump(ctx, jend);
				store_result(ctx, dst);
			}
			break;
		case OStaticClosure:
		{
			vclosure* c = alloc_static_closure(ctx, o->p2);
			preg* r = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, r, pconst64(&p, (int_val)c));
			store(ctx, dst, r, true);
		}
		break;
		case OField:
		{
#				ifndef HL_64
			if (dst->t->kind == HI64) {
				error_i64();
				break;
			}
#				endif
			switch (ra->t->kind) {
			case HOBJ:
			case HSTRUCT:
			{
				hl_runtime_obj* rt = hl_get_obj_rt(ra->t);
				preg* rr = alloc_cpu(ctx, ra, true);
				if (dst->t->kind == HSTRUCT) {
					hl_type* ft = hl_obj_field_fetch(ra->t, o->p3)->t;
					if (ft->kind == HPACKED) {
						preg* r = alloc_reg(ctx, RCPU);
						op64(ctx, LEA, r, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p3]));
						store(ctx, dst, r, true);
						break;
					}
				}
				copy_to(ctx, dst, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p3]));
			}
			break;
			case HVIRTUAL:
				// ASM for --> if( hl_vfields(o)[f] ) r = *hl_vfields(o)[f]; else r = hl_dyn_get(o,hash(field),vt)
			{
				int jhasfield, jend, size;
				bool need_type = !(IS_FLOAT(dst) || dst->t->kind == HI64);
				preg* v = alloc_cpu_call(ctx, ra);
				preg* r = alloc_reg(ctx, RCPU);
				op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(v->id), sizeof(vvirtual) + HL_WSIZE * o->p3));
				op64(ctx, TEST, r, r);
				XJump_small(JNotZero, jhasfield);
				size = begin_native_call(ctx, need_type ? 3 : 2);
				if (need_type) set_native_arg(ctx, pconst64(&p, (int_val)dst->t));
				set_native_arg(ctx, pconst64(&p, (int_val)ra->t->virt->fields[o->p3].hashed_name));
				set_native_arg(ctx, v);
				call_native(ctx, get_dynget(dst->t), size);
				store_result(ctx, dst);
				XJump_small(JAlways, jend);
				patch_jump(ctx, jhasfield);
				copy_to(ctx, dst, pmem(&p, (CpuReg)r->id, 0));
				patch_jump(ctx, jend);
				scratch(dst->current);
			}
			break;
			default:
				ASSERT(ra->t->kind);
				break;
			}
		}
		break;
		case OSetField:
		{
			switch (dst->t->kind) {
			case HOBJ:
			case HSTRUCT:
			{
				hl_runtime_obj* rt = hl_get_obj_rt(dst->t);
				preg* rr = alloc_cpu(ctx, dst, true);
				if (rb->t->kind == HSTRUCT) {
					hl_type* ft = hl_obj_field_fetch(dst->t, o->p2)->t;
					if (ft->kind == HPACKED) {
						hl_runtime_obj* frt = hl_get_obj_rt(ft->tparam);
						preg* prb = alloc_cpu(ctx, rb, true);
						preg* tmp = alloc_reg(ctx, RCPU_CALL);
						int offset = 0;
						while (offset < frt->size) {
							int remain = frt->size - offset;
							int copy_size = remain >= HL_WSIZE ? HL_WSIZE : (remain >= 4 ? 4 : (remain >= 2 ? 2 : 1));
							copy(ctx, tmp, pmem(&p, (CpuReg)prb->id, offset), copy_size);
							copy(ctx, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p2] + offset), tmp, copy_size);
							offset += copy_size;
						}
						break;
					}
				}
				copy_from(ctx, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p2]), rb);
			}
			break;
			case HVIRTUAL:
				// ASM for --> if( hl_vfields(o)[f] ) *hl_vfields(o)[f] = v; else hl_dyn_set(o,hash(field),vt,v)
			{
				int jhasfield, jend;
				preg* obj = alloc_cpu_call(ctx, dst);
				preg* r = alloc_reg(ctx, RCPU);
				op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(obj->id), sizeof(vvirtual) + HL_WSIZE * o->p2));
				op64(ctx, TEST, r, r);
				XJump_small(JNotZero, jhasfield);
#						ifdef HL_64
				switch (rb->t->kind) {
				case HF64:
				case HF32:
					size = begin_native_call(ctx, 3);
					set_native_arg_fpu(ctx, fetch(rb), rb->t->kind == HF32);
					break;
				case HI64:
					size = begin_native_call(ctx, 3);
					set_native_arg(ctx, fetch(rb));
					break;
				default:
					size = begin_native_call(ctx, 4);
					set_native_arg(ctx, fetch(rb));
					set_native_arg(ctx, pconst64(&p, (int_val)rb->t));
					break;
				}
				set_native_arg(ctx, pconst(&p, dst->t->virt->fields[o->p2].hashed_name));
				set_native_arg(ctx, obj);
#						else
				switch (rb->t->kind) {
				case HF64:
				case HI64:
					size = pad_before_call(ctx, HL_WSIZE * 2 + sizeof(double));
					push_reg(ctx, rb);
					break;
				case HF32:
					size = pad_before_call(ctx, HL_WSIZE * 2 + sizeof(float));
					push_reg(ctx, rb);
					break;
				default:
					size = pad_before_call(ctx, HL_WSIZE * 4);
					op64(ctx, PUSH, fetch32(ctx, rb), UNUSED);
					op64(ctx, MOV, r, pconst64(&p, (int_val)rb->t));
					op64(ctx, PUSH, r, UNUSED);
					break;
				}
				op32(ctx, MOV, r, pconst(&p, dst->t->virt->fields[o->p2].hashed_name));
				op64(ctx, PUSH, r, UNUSED);
				op64(ctx, PUSH, obj, UNUSED);
#						endif
				call_native(ctx, get_dynset(rb->t), size);
				XJump_small(JAlways, jend);
				patch_jump(ctx, jhasfield);
				copy_from(ctx, pmem(&p, (CpuReg)r->id, 0), rb);
				patch_jump(ctx, jend);
				scratch(rb->current);
			}
			break;
			default:
				ASSERT(dst->t->kind);
				break;
			}
		}
		break;
		case OGetThis:
		{
			vreg* r = R(0);
			hl_runtime_obj* rt = hl_get_obj_rt(r->t);
			preg* rr = alloc_cpu(ctx, r, true);
			if (dst->t->kind == HSTRUCT) {
				hl_type* ft = hl_obj_field_fetch(r->t, o->p2)->t;
				if (ft->kind == HPACKED) {
					preg* r = alloc_reg(ctx, RCPU);
					op64(ctx, LEA, r, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p2]));
					store(ctx, dst, r, true);
					break;
				}
			}
			copy_to(ctx, dst, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p2]));
		}
		break;
		case OSetThis:
		{
			vreg* r = R(0);
			hl_runtime_obj* rt = hl_get_obj_rt(r->t);
			preg* rr = alloc_cpu(ctx, r, true);
			if (ra->t->kind == HSTRUCT) {
				hl_type* ft = hl_obj_field_fetch(r->t, o->p1)->t;
				if (ft->kind == HPACKED) {
					hl_runtime_obj* frt = hl_get_obj_rt(ft->tparam);
					preg* pra = alloc_cpu(ctx, ra, true);
					preg* tmp = alloc_reg(ctx, RCPU_CALL);
					int offset = 0;
					while (offset < frt->size) {
						int remain = frt->size - offset;
						int copy_size = remain >= HL_WSIZE ? HL_WSIZE : (remain >= 4 ? 4 : (remain >= 2 ? 2 : 1));
						copy(ctx, tmp, pmem(&p, (CpuReg)pra->id, offset), copy_size);
						copy(ctx, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p1] + offset), tmp, copy_size);
						offset += copy_size;
					}
					break;
				}
			}
			copy_from(ctx, pmem(&p, (CpuReg)rr->id, rt->fields_indexes[o->p1]), ra);
		}
		break;
		case OCallThis:
		{
			int nargs = o->p3 + 1;
			int* args = (int*)hl_malloc(&ctx->falloc, sizeof(int) * nargs);
			int size;
			preg* r = alloc_cpu(ctx, R(0), true);
			preg* tmp;
			tmp = alloc_reg(ctx, RCPU_CALL);
			op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(r->id), 0)); // read type
			op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), HL_WSIZE * 2)); // read proto
			args[0] = 0;
			for (i = 1;i < nargs;i++)
				args[i] = o->extra[i - 1];
			size = prepare_call_args(ctx, nargs, args, ctx->vregs, 0);
			op_call(ctx, pmem(&p, static_cast<CpuReg>(tmp->id), o->p2 * HL_WSIZE), size);
			discard_regs(ctx, false);
			store_result(ctx, dst);
		}
		break;
		case OCallMethod:
			switch (R(o->extra[0])->t->kind) {
			case HOBJ: {
				int size;
				preg* r = alloc_cpu(ctx, R(o->extra[0]), true);
				preg* tmp;
				tmp = alloc_reg(ctx, RCPU_CALL);
				op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(r->id), 0)); // read type
				op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(tmp->id), HL_WSIZE * 2)); // read proto
				size = prepare_call_args(ctx, o->p3, o->extra, ctx->vregs, 0);
				op_call(ctx, pmem(&p, static_cast<CpuReg>(tmp->id), o->p2 * HL_WSIZE), size);
				discard_regs(ctx, false);
				store_result(ctx, dst);
				break;
			}
			case HVIRTUAL:
				// ASM for --> if( hl_vfields(o)[f] ) dst = *hl_vfields(o)[f](o->value,args...); else dst = hl_dyn_call_obj(o->value,field,args,&ret)
			{
				int size;
				int paramsSize;
				int jhasfield, jend;
				bool need_dyn;
				bool obj_in_args = false;
				vreg* obj = R(o->extra[0]);
				preg* v = alloc_cpu_call(ctx, obj);
				preg* r = alloc_reg(ctx, RCPU_CALL);
				op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(v->id), sizeof(vvirtual) + HL_WSIZE * o->p2));
				op64(ctx, TEST, r, r);
				save_regs(ctx);

				if (o->p3 < 6) {
					XJump_small(JNotZero, jhasfield);
				}
				else {
					XJump(JNotZero, jhasfield);
				}

				need_dyn = !hl_is_ptr(dst->t) && dst->t->kind != HVOID;
				paramsSize = (o->p3 - 1) * HL_WSIZE;
				if (need_dyn) paramsSize += sizeof(vdynamic);
				if (paramsSize & 15) paramsSize += 16 - (paramsSize & 15);
				op64(ctx, SUB, PESP, pconst(&p, paramsSize));
				op64(ctx, MOV, r, PESP);

				for (i = 0;i < o->p3 - 1;i++) {
					vreg* a = R(o->extra[i + 1]);
					if (hl_is_ptr(a->t)) {
						op64(ctx, MOV, pmem(&p, static_cast<CpuReg>(r->id), i * HL_WSIZE), alloc_cpu(ctx, a, true));
						if (a->current != v) {
							RUNLOCK(a->current);
						}
						else
							obj_in_args = true;
					}
					else {
						preg* r2 = alloc_reg(ctx, RCPU);
						op64(ctx, LEA, r2, &a->stack);
						op64(ctx, MOV, pmem(&p, static_cast<CpuReg>(r->id), i * HL_WSIZE), r2);
						if (r2 != v) RUNLOCK(r2);
					}
				}

				jit_buf(ctx);

				if (!need_dyn) {
					size = begin_native_call(ctx, 5);
					set_native_arg(ctx, pconst(&p, 0));
				}
				else {
					preg* rtmp = alloc_reg(ctx, RCPU);
					op64(ctx, LEA, rtmp, pmem(&p, Esp, paramsSize - sizeof(vdynamic)));
					size = begin_native_call(ctx, 5);
					set_native_arg(ctx, rtmp);
					if (!IS_64) RUNLOCK(rtmp);
				}
				set_native_arg(ctx, r);
				set_native_arg(ctx, pconst(&p, obj->t->virt->fields[o->p2].hashed_name)); // fid
				set_native_arg(ctx, pconst64(&p, (int_val)obj->t->virt->fields[o->p2].t)); // ftype
				set_native_arg(ctx, pmem(&p, static_cast<CpuReg>(v->id), HL_WSIZE)); // o->value
				call_native(ctx, hl_dyn_call_obj, size + paramsSize);
				if (need_dyn) {
					preg* r = IS_FLOAT(dst) ? REG_AT(XMM(0)) : PEAX;
					copy(ctx, r, pmem(&p, Esp, HDYN_VALUE - (int)sizeof(vdynamic)), dst->size);
					store(ctx, dst, r, false);
				}
				else
					store(ctx, dst, PEAX, false);

				XJump_small(JAlways, jend);
				patch_jump(ctx, jhasfield);
				restore_regs(ctx);

				if (!obj_in_args) {
					// o = o->value hack
					if (v->holds) v->holds->current = NULL;
					obj->current = v;
					v->holds = obj;
					op64(ctx, MOV, v, pmem(&p, static_cast<CpuReg>(v->id), HL_WSIZE));
					size = prepare_call_args(ctx, o->p3, o->extra, ctx->vregs, 0);
				}
				else {
					// keep o->value in R(f->nregs)
					int regids[64];
					preg* pc = alloc_reg(ctx, RCPU_CALL);
					vreg* sc = R(f->nregs); // scratch register that we temporary rebind
					if (o->p3 >= 63) jit_error("assert");
					memcpy(regids, o->extra, o->p3 * sizeof(int));
					regids[0] = f->nregs;
					sc->size = HL_WSIZE;
					sc->t = &hlt_dyn;
					op64(ctx, MOV, pc, pmem(&p, static_cast<CpuReg>(v->id), HL_WSIZE));
					scratch(pc);
					sc->current = pc;
					pc->holds = sc;
					size = prepare_call_args(ctx, o->p3, regids, ctx->vregs, 0);
				}

				op_call(ctx, r, size);
				discard_regs(ctx, false);
				store_result(ctx, dst);
				patch_jump(ctx, jend);
			}
			break;
			default:
				ASSERT(0);
				break;
			}
			break;
		case ORethrow:
		{
			int size = prepare_call_args(ctx, 1, &o->p1, ctx->vregs, 0);
			call_native(ctx, hl_rethrow, size);
		}
		break;
		case OThrow:
		{
			int size = prepare_call_args(ctx, 1, &o->p1, ctx->vregs, 0);
			call_native(ctx, hl_throw, size);
		}
		break;
		case OLabel:
			// NOP for now
			discard_regs(ctx, false);
			break;
		case OGetI8:
		case OGetI16:
		{
			preg* base = alloc_cpu(ctx, ra, true);
			preg* offset = alloc_cpu64(ctx, rb, true);
			preg* r = alloc_reg(ctx, o->op == OGetI8 ? RCPU_8BITS : RCPU);
			op64(ctx, XOR, r, r);
			op32(ctx, o->op == OGetI8 ? MOV8 : MOV16, r, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0));
			store(ctx, dst, r, true);
		}
		break;
		case OGetMem:
		{
#ifndef HL_64
			if (dst->t->kind == HI64) {
				error_i64();
			}
#endif
			preg* base = alloc_cpu(ctx, ra, true);
			preg* offset = alloc_cpu64(ctx, rb, true);
			store(ctx, dst, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), false);
		}
		break;
		case OSetI8:
		{
			preg* base = alloc_cpu(ctx, dst, true);
			preg* offset = alloc_cpu64(ctx, ra, true);
			preg* value = alloc_cpu8(ctx, rb, true);
			op32(ctx, MOV8, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
		}
		break;
		case OSetI16:
		{
			preg* base = alloc_cpu(ctx, dst, true);
			preg* offset = alloc_cpu64(ctx, ra, true);
			preg* value = alloc_cpu(ctx, rb, true);
			op32(ctx, MOV16, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
		}
		break;
		case OSetMem:
		{
			preg* base = alloc_cpu(ctx, dst, true);
			preg* offset = alloc_cpu64(ctx, ra, true);
			preg* value;
			switch (rb->t->kind) {
			case HI32:
				value = alloc_cpu(ctx, rb, true);
				op32(ctx, MOV, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
				break;
			case HF32:
				value = alloc_fpu(ctx, rb, true);
				op32(ctx, MOVSS, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
				break;
			case HF64:
				value = alloc_fpu(ctx, rb, true);
				op32(ctx, MOVSD, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
				break;
			case HI64:
				value = alloc_cpu(ctx, rb, true);
				op64(ctx, MOV, pmem2(&p, static_cast<CpuReg>(base->id), static_cast<CpuReg>(offset->id), 1, 0), value);
				break;
			default:
				ASSERT(rb->t->kind);
				break;
			}
		}
		break;
		case OType:
		{
			op64(ctx, MOV, alloc_cpu(ctx, dst, false), pconst64(&p, (int_val)(m->code->types + o->p2)));
			store(ctx, dst, dst->current, false);
		}
		break;
		case OGetType:
		{
			int jnext, jend;
			preg* r = alloc_cpu(ctx, ra, true);
			preg* tmp = alloc_reg(ctx, RCPU);
			op64(ctx, TEST, r, r);
			XJump_small(JNotZero, jnext);
			op64(ctx, MOV, tmp, pconst64(&p, (int_val)&hlt_void));
			XJump_small(JAlways, jend);
			patch_jump(ctx, jnext);
			op64(ctx, MOV, tmp, pmem(&p, static_cast<CpuReg>(r->id), 0));
			patch_jump(ctx, jend);
			store(ctx, dst, tmp, true);
		}
		break;
		case OGetArray:
		{
			preg* rdst = IS_FLOAT(dst) ? alloc_fpu(ctx, dst, false) : alloc_cpu(ctx, dst, false);
			if (ra->t->kind == HABSTRACT) {
				int osize;
				bool isRead = dst->t->kind != HOBJ && dst->t->kind != HSTRUCT;
				if (isRead)
					osize = sizeof(void*);
				else {
					hl_runtime_obj* rt = hl_get_obj_rt(dst->t);
					osize = rt->size;
				}
				preg* idx = alloc_cpu64(ctx, rb, true);
				op64(ctx, IMUL, idx, pconst(&p, osize));
				op64(ctx, isRead ? MOV : LEA, rdst, pmem2(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), static_cast<CpuReg>(idx->id), 1, 0));
				store(ctx, dst, dst->current, false);
				scratch(idx);
			}
			else {
				copy(ctx, rdst, pmem2(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), static_cast<CpuReg>(alloc_cpu64(ctx, rb, true)->id), hl_type_size(dst->t), sizeof(varray)), dst->size);
				store(ctx, dst, dst->current, false);
			}
		}
		break;
		case OSetArray:
		{
			if (dst->t->kind == HABSTRACT) {
				int osize;
				bool isWrite = rb->t->kind != HOBJ && rb->t->kind != HSTRUCT;
				if (isWrite) {
					osize = sizeof(void*);
				}
				else {
					hl_runtime_obj* rt = hl_get_obj_rt(rb->t);
					osize = rt->size;
				}
				preg* pdst = alloc_cpu(ctx, dst, true);
				preg* pra = alloc_cpu64(ctx, ra, true);
				op64(ctx, IMUL, pra, pconst(&p, osize));
				op64(ctx, ADD, pdst, pra);
				scratch(pra);
				preg* prb = alloc_cpu(ctx, rb, true);
				preg* tmp = alloc_reg(ctx, RCPU_CALL);
				int offset = 0;
				while (offset < osize) {
					int remain = osize - offset;
					int copy_size = remain >= HL_WSIZE ? HL_WSIZE : (remain >= 4 ? 4 : (remain >= 2 ? 2 : 1));
					copy(ctx, tmp, pmem(&p, static_cast<CpuReg>(prb->id), offset), copy_size);
					copy(ctx, pmem(&p, static_cast<CpuReg>(pdst->id), offset), tmp, copy_size);
					offset += copy_size;
				}
				scratch(pdst);
			}
			else {
				preg* rrb = IS_FLOAT(rb) ? alloc_fpu(ctx, rb, true) : alloc_cpu(ctx, rb, true);
				copy(ctx, pmem2(&p, static_cast<CpuReg>(alloc_cpu(ctx, dst, true)->id), static_cast<CpuReg>(alloc_cpu64(ctx, ra, true)->id), hl_type_size(rb->t), sizeof(varray)), rrb, rb->size);
			}
		}
		break;
		case OArraySize:
		{
			op32(ctx, MOV, alloc_cpu(ctx, dst, false), pmem(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), ra->t->kind == HABSTRACT ? HL_WSIZE + 4 : HL_WSIZE * 2));
			store(ctx, dst, dst->current, false);
		}
		break;
		case ORef:
		{
			scratch(ra->current);
			op64(ctx, MOV, alloc_cpu(ctx, dst, false), REG_AT(Ebp));
			if (ra->stackPos < 0)
				op64(ctx, SUB, dst->current, pconst(&p, -ra->stackPos));
			else
				op64(ctx, ADD, dst->current, pconst(&p, ra->stackPos));
			store(ctx, dst, dst->current, false);
		}
		break;
		case OUnref:
			copy_to(ctx, dst, pmem(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), 0));
			break;
		case OSetref:
			copy_from(ctx, pmem(&p, static_cast<CpuReg>(alloc_cpu(ctx, dst, true)->id), 0), ra);
			break;
		case ORefData:
			switch (ra->t->kind) {
			case HARRAY:
			{
				preg* r = fetch(ra);
				preg* d = alloc_cpu(ctx, dst, false);
				op64(ctx, MOV, d, r);
				op64(ctx, ADD, d, pconst(&p, sizeof(varray)));
				store(ctx, dst, dst->current, false);
			}
			break;
			default:
				ASSERT(ra->t->kind);
			}
			break;
		case ORefOffset:
		{
			preg* d = alloc_cpu(ctx, rb, true);
			preg* r2 = alloc_cpu(ctx, dst, false);
			preg* r = fetch(ra);
			int size = hl_type_size(dst->t->tparam);
			op64(ctx, MOV, r2, r);
			switch (size) {
			case 1:
				break;
			case 2:
				op64(ctx, SHL, d, pconst(&p, 1));
				break;
			case 4:
				op64(ctx, SHL, d, pconst(&p, 2));
				break;
			case 8:
				op64(ctx, SHL, d, pconst(&p, 3));
				break;
			default:
				op64(ctx, IMUL, d, pconst(&p, size));
				break;
			}
			op64(ctx, ADD, r2, d);
			scratch(d);
			store(ctx, dst, dst->current, false);
		}
		break;
		case OToVirtual:
		{
#				ifdef HL_64
			int size = pad_before_call(ctx, 0);
			op64(ctx, MOV, REG_AT(CALL_REGS[1]), fetch(ra));
			op64(ctx, MOV, REG_AT(CALL_REGS[0]), pconst64(&p, (int_val)dst->t));
#				else
			int size = pad_before_call(ctx, HL_WSIZE * 2);
			op32(ctx, PUSH, fetch(ra), UNUSED);
			op32(ctx, PUSH, pconst(&p, (int)(int_val)dst->t), UNUSED);
#				endif
			if (ra->t->kind == HOBJ) hl_get_obj_rt(ra->t); // ensure it's initialized
			call_native(ctx, hl_to_virtual, size);
			store(ctx, dst, PEAX, true);
		}
		break;
		case OMakeEnum:
		{
			hl_enum_construct* c = &dst->t->tenum->constructs[o->p2];
			int_val args[] = { (int_val)dst->t, o->p2 };
			int i;
			call_native_consts(ctx, hl_alloc_enum, args, 2);
			RLOCK(PEAX);
			for (i = 0;i < c->nparams;i++) {
				preg* r = fetch(R(o->extra[i]));
				copy(ctx, pmem(&p, Eax, c->offsets[i]), r, R(o->extra[i])->size);
				RUNLOCK(fetch(R(o->extra[i])));
				if ((i & 15) == 0) jit_buf(ctx);
			}
			store(ctx, dst, PEAX, true);
		}
		break;
		case OEnumAlloc:
		{
			int_val args[] = { (int_val)dst->t, o->p2 };
			call_native_consts(ctx, hl_alloc_enum, args, 2);
			store(ctx, dst, PEAX, true);
		}
		break;
		case OEnumField:
		{
			hl_enum_construct* c = &ra->t->tenum->constructs[o->p3];
			preg* r = alloc_cpu(ctx, ra, true);
			copy_to(ctx, dst, pmem(&p, static_cast<CpuReg>(r->id), c->offsets[(int)(int_val)o->extra]));
		}
		break;
		case OSetEnumField:
		{
			hl_enum_construct* c = &dst->t->tenum->constructs[0];
			preg* r = alloc_cpu(ctx, dst, true);
			switch (rb->t->kind) {
			case HF64:
			{
				preg* d = alloc_fpu(ctx, rb, true);
				copy(ctx, pmem(&p, static_cast<CpuReg>(r->id), c->offsets[o->p2]), d, 8);
				break;
			}
			default:
				copy(ctx, pmem(&p, static_cast<CpuReg>(r->id), c->offsets[o->p2]), alloc_cpu(ctx, rb, true), hl_type_size(c->params[o->p2]));
				break;
			}
		}
		break;
		case ONullCheck:
		{
			int jz;
			preg* r = alloc_cpu(ctx, dst, true);
			op64(ctx, TEST, r, r);
			XJump_small(JNotZero, jz);

			hl_opcode* next = f->ops + opCount + 1;
			bool null_field_access = false;
			int hashed_name = 0;
			// skip const and operation between nullcheck and access
			while ((next < f->ops + f->nops - 1) && (next->op >= OInt && next->op <= ODecr)) {
				next++;
			}
			if ((next->op == OField && next->p2 == o->p1) || (next->op == OSetField && next->p1 == o->p1)) {
				int fid = next->op == OField ? next->p3 : next->p2;
				hl_obj_field* f = NULL;
				if (dst->t->kind == HOBJ || dst->t->kind == HSTRUCT)
					f = hl_obj_field_fetch(dst->t, fid);
				else if (dst->t->kind == HVIRTUAL)
					f = dst->t->virt->fields + fid;
				if (f == NULL) ASSERT(dst->t->kind);
				null_field_access = true;
				hashed_name = f->hashed_name;
			}
			else if ((next->op >= OCall1 && next->op <= OCallN) && next->p3 == o->p1) {
				int fid = next->p2 < 0 ? -1 : ctx->m->functions_indexes[next->p2];
				hl_function* cf = ctx->m->code->functions + fid;
				vbyte* name = reinterpret_cast<vbyte*>(const_cast<uchar*>(fun_field_name(cf)));
				null_field_access = true;
				hashed_name = hl_hash_gen(reinterpret_cast<uchar*>(name), true);
			}

			if (null_field_access) {
				pad_before_call(ctx, HL_WSIZE);
				if (hashed_name >= 0 && hashed_name < 256)
					op64(ctx, PUSH8, pconst(&p, hashed_name), UNUSED);
				else
					op32(ctx, PUSH, pconst(&p, hashed_name), UNUSED);
			}
			else {
				pad_before_call(ctx, 0);
			}

			jlist* j = (jlist*)hl_malloc(&ctx->galloc, sizeof(jlist));
			j->pos = BUF_POS();
			j->target = null_field_access ? -3 : -1;
			j->next = ctx->calls;
			ctx->calls = j;

			op64(ctx, MOV, PEAX, pconst64(&p, RESERVE_ADDRESS));
			op_call(ctx, PEAX, -1);
			patch_jump(ctx, jz);
		}
		break;
		case OSafeCast:
			make_dyn_cast(ctx, dst, ra);
			break;
		case ODynGet:
		{
			int size;
#				ifdef HL_64
			if (IS_FLOAT(dst) || dst->t->kind == HI64) {
				size = begin_native_call(ctx, 2);
			}
			else {
				size = begin_native_call(ctx, 3);
				set_native_arg(ctx, pconst64(&p, (int_val)dst->t));
			}
			set_native_arg(ctx, pconst64(&p, (int_val)hl_hash_utf8(m->code->strings[o->p3])));
			set_native_arg(ctx, fetch(ra));
#				else
			preg* r;
			r = alloc_reg(ctx, RCPU);
			if (IS_FLOAT(dst) || dst->t->kind == HI64) {
				size = pad_before_call(ctx, HL_WSIZE * 2);
			}
			else {
				size = pad_before_call(ctx, HL_WSIZE * 3);
				op64(ctx, MOV, r, pconst64(&p, (int_val)dst->t));
				op64(ctx, PUSH, r, UNUSED);
			}
			op64(ctx, MOV, r, pconst64(&p, (int_val)hl_hash_utf8(m->code->strings[o->p3])));
			op64(ctx, PUSH, r, UNUSED);
			op64(ctx, PUSH, fetch(ra), UNUSED);
#				endif
			call_native(ctx, get_dynget(dst->t), size);
			store_result(ctx, dst);
		}
		break;
		case ODynSet:
		{
			int size;
#				ifdef HL_64
			switch (rb->t->kind) {
			case HF32:
			case HF64:
				size = begin_native_call(ctx, 3);
				set_native_arg_fpu(ctx, fetch(rb), rb->t->kind == HF32);
				set_native_arg(ctx, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)));
				set_native_arg(ctx, fetch(dst));
				call_native(ctx, get_dynset(rb->t), size);
				break;
			case HI64:
				size = begin_native_call(ctx, 3);
				set_native_arg(ctx, fetch(rb));
				set_native_arg(ctx, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)));
				set_native_arg(ctx, fetch(dst));
				call_native(ctx, get_dynset(rb->t), size);
				break;
			default:
				size = begin_native_call(ctx, 4);
				set_native_arg(ctx, fetch(rb));
				set_native_arg(ctx, pconst64(&p, (int_val)rb->t));
				set_native_arg(ctx, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)));
				set_native_arg(ctx, fetch(dst));
				call_native(ctx, get_dynset(rb->t), size);
				break;
			}
#				else
			switch (rb->t->kind) {
			case HF32:
				size = pad_before_call(ctx, HL_WSIZE * 2 + sizeof(float));
				push_reg(ctx, rb);
				op32(ctx, PUSH, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)), UNUSED);
				op32(ctx, PUSH, fetch(dst), UNUSED);
				call_native(ctx, get_dynset(rb->t), size);
				break;
			case HF64:
			case HI64:
				size = pad_before_call(ctx, HL_WSIZE * 2 + sizeof(double));
				push_reg(ctx, rb);
				op32(ctx, PUSH, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)), UNUSED);
				op32(ctx, PUSH, fetch(dst), UNUSED);
				call_native(ctx, get_dynset(rb->t), size);
				break;
			default:
				size = pad_before_call(ctx, HL_WSIZE * 4);
				op32(ctx, PUSH, fetch32(ctx, rb), UNUSED);
				op32(ctx, PUSH, pconst64(&p, (int_val)rb->t), UNUSED);
				op32(ctx, PUSH, pconst64(&p, hl_hash_gen(hl_get_ustring(m->code, o->p2), true)), UNUSED);
				op32(ctx, PUSH, fetch(dst), UNUSED);
				call_native(ctx, get_dynset(rb->t), size);
				break;
			}
#				endif
		}
		break;
		case OTrap:
		{
			int size, jenter, jtrap;
			int offset = 0;
			int trap_size = (sizeof(hl_trap_ctx) + 15) & 0xFFF0;
			hl_trap_ctx* t = NULL;
#				ifndef HL_THREADS
			if (tinf == NULL) tinf = hl_get_thread(); // single thread
#				endif

#				ifdef HL_64
			preg* trap = REG_AT(CALL_REGS[0]);
#				else
			preg* trap = PEAX;
#				endif
			RLOCK(trap);

			preg* treg = alloc_reg(ctx, RCPU);
			if (!tinf) {
				call_native(ctx, hl_get_thread, 0);
				op64(ctx, MOV, treg, PEAX);
				offset = (int)(int_val)&tinf->trap_current;
			}
			else {
				offset = 0;
				op64(ctx, MOV, treg, pconst64(&p, (int_val)&tinf->trap_current));
			}
			op64(ctx, MOV, trap, pmem(&p, static_cast<CpuReg>(treg->id), offset));
			op64(ctx, SUB, PESP, pconst(&p, trap_size));
			op64(ctx, MOV, pmem(&p, Esp, (int)(int_val)&t->prev), trap);
			op64(ctx, MOV, trap, PESP);
			op64(ctx, MOV, pmem(&p, static_cast<CpuReg>(treg->id), offset), trap);

			/*
				This is a bit hackshish : we want to detect the type of exception filtered by the catch so we check the following
				sequence of HL opcodes:

				trap E,@catch
				...
				@catch:
				global R, _
				call _, ???(R,E)

				??? is expected to be hl.BaseType.check
			*/
			hl_opcode* next = f->ops + opCount + 1 + o->p2;
			hl_opcode* next2 = f->ops + opCount + 2 + o->p2;
			if (next->op == OGetGlobal && next2->op == OCall2 && next2->p3 == next->p1 && dst->stack.id == (int)(int_val)next2->extra) {
				hl_type* gt = m->code->globals[next->p2];
				while (gt->kind == HOBJ && gt->obj->super) gt = gt->obj->super;
				if (gt->kind == HOBJ && gt->obj->nfields && gt->obj->fields[0].t->kind == HTYPE) {
					void* addr = m->globals_data + m->globals_indexes[next->p2];
#						ifdef HL_64
					op64(ctx, MOV, treg, pconst64(&p, (int_val)addr));
					op64(ctx, MOV, treg, pmem(&p, static_cast<CpuReg>(treg->id), 0));
#						else
					op64(ctx, MOV, treg, paddr(&p, addr));
#						endif
				}
				else
					op64(ctx, MOV, treg, pconst(&p, 0));
			}
			else {
				op64(ctx, MOV, treg, pconst(&p, 0));
			}
			op64(ctx, MOV, pmem(&p, Esp, (int)(int_val)&t->tcheck), treg);

			size = begin_native_call(ctx, 1);
			set_native_arg(ctx, trap);
			call_native(ctx, setjmp_wrapper, size);
			op64(ctx, TEST, PEAX, PEAX);
			XJump_small(JZero, jenter);
			op64(ctx, ADD, PESP, pconst(&p, trap_size));
			if (!tinf) {
				call_native(ctx, hl_get_thread, 0);
				op64(ctx, MOV, PEAX, pmem(&p, Eax, (int)(int_val)&tinf->exc_value));
			}
			else {
				op64(ctx, MOV, PEAX, pconst64(&p, (int_val)&tinf->exc_value));
				op64(ctx, MOV, PEAX, pmem(&p, Eax, 0));
			}
			store(ctx, dst, PEAX, false);

			jtrap = do_jump(ctx, OJAlways, false);
			register_jump(ctx, jtrap, (opCount + 1) + o->p2);
			patch_jump(ctx, jenter);
		}
		break;
		case OEndTrap:
		{
			int trap_size = (sizeof(hl_trap_ctx) + 15) & 0xFFF0;
			hl_trap_ctx* tmp = NULL;
			preg* addr, * r;
			int offset;
			if (!tinf) {
				call_native(ctx, hl_get_thread, 0);
				addr = PEAX;
				RLOCK(addr);
				offset = (int)(int_val)&tinf->trap_current;
			}
			else {
				offset = 0;
				addr = alloc_reg(ctx, RCPU);
				op64(ctx, MOV, addr, pconst64(&p, (int_val)&tinf->trap_current));
			}
			r = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(addr->id), offset));
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(r->id), (int)(int_val)&tmp->prev));
			op64(ctx, MOV, pmem(&p, static_cast<CpuReg>(addr->id), offset), r);
#				ifdef HL_WIN
			// erase eip (prevent false positive)
			{
				_JUMP_BUFFER* b = NULL;
#					ifdef HL_64
				op64(ctx, MOV, pmem(&p, Esp, (int)(int_val) & (b->Rip)), PEAX);
#					else
				op64(ctx, MOV, pmem(&p, Esp, (int)&(b->Eip)), PEAX);
#					endif
			}
#				endif
			op64(ctx, ADD, PESP, pconst(&p, trap_size));
		}
		break;
		case OEnumIndex:
		{
			preg* r = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, r, pmem(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), HL_WSIZE));
			store(ctx, dst, r, true);
			break;
		}
		break;
		case OSwitch:
		{
			int jdefault;
			int i;
			preg* r = alloc_cpu(ctx, dst, true);
			preg* r2 = alloc_reg(ctx, RCPU);
			op32(ctx, CMP, r, pconst(&p, o->p2));
			XJump(JUGte, jdefault);
			// r2 = r * 5 + eip
#				ifdef HL_64
			op64(ctx, XOR, r2, r2);
#				endif
			op32(ctx, MOV, r2, r);
			op32(ctx, SHL, r2, pconst(&p, 2));
			op32(ctx, ADD, r2, r);
#				ifdef HL_64
			preg* tmp = alloc_reg(ctx, RCPU);
			op64(ctx, MOV, tmp, pconst64(&p, RESERVE_ADDRESS));
#				else
			op64(ctx, ADD, r2, pconst64(&p, RESERVE_ADDRESS));
#				endif
			{
				jlist* s = (jlist*)hl_malloc(&ctx->galloc, sizeof(jlist));
				s->pos = BUF_POS() - sizeof(void*);
				s->next = ctx->switchs;
				ctx->switchs = s;
			}
#				ifdef HL_64
			op64(ctx, ADD, r2, tmp);
#				endif
			op64(ctx, JMP, r2, UNUSED);
			for (i = 0;i < o->p2;i++) {
				int j = do_jump(ctx, OJAlways, false);
				register_jump(ctx, j, (opCount + 1) + o->extra[i]);
				if ((i & 15) == 0) jit_buf(ctx);
			}
			patch_jump(ctx, jdefault);
		}
		break;
		case OGetTID:
			op32(ctx, MOV, alloc_cpu(ctx, dst, false), pmem(&p, static_cast<CpuReg>(alloc_cpu(ctx, ra, true)->id), 0));
			store(ctx, dst, dst->current, false);
			break;
		case OAssert:
		{
			jlist* j = (jlist*)hl_malloc(&ctx->galloc, sizeof(jlist));
			j->pos = BUF_POS();
			j->target = -2;
			j->next = ctx->calls;
			ctx->calls = j;

			op64(ctx, MOV, PEAX, pconst64(&p, RESERVE_ADDRESS));
			op_call(ctx, PEAX, -1);
		}
		break;
		case ONop:
			break;
		case OPrefetch:
		{
			preg* r = alloc_cpu(ctx, dst, true);
			if (o->p2 > 0) {
				switch (dst->t->kind) {
				case HOBJ:
				case HSTRUCT:
				{
					hl_runtime_obj* rt = hl_get_obj_rt(dst->t);
					preg* r2 = alloc_reg(ctx, RCPU);
					op64(ctx, LEA, r2, pmem(&p, static_cast<CpuReg>(r->id), rt->fields_indexes[o->p2 - 1]));
					r = r2;
				}
				break;
				default:
					ASSERT(dst->t->kind);
					break;
				}
			}
			switch (o->p3) {
			case 0:
				op64(ctx, PREFETCHT0, pmem(&p, static_cast<CpuReg>(r->id), 0), UNUSED);
				break;
			case 1:
				op64(ctx, PREFETCHT1, pmem(&p, static_cast<CpuReg>(r->id), 0), UNUSED);
				break;
			case 2:
				op64(ctx, PREFETCHT2, pmem(&p, static_cast<CpuReg>(r->id), 0), UNUSED);
				break;
			case 3:
				op64(ctx, PREFETCHNTA, pmem(&p, static_cast<CpuReg>(r->id), 0), UNUSED);
				break;
			case 4:
				op64(ctx, PREFETCHW, pmem(&p, static_cast<CpuReg>(r->id), 0), UNUSED);
				break;
			default:
				ASSERT(o->p3);
				break;
			}
		}
		break;
		case OAsm:
		{
			switch (o->p1) {
			case 0: // byte output
				B(o->p2);
				break;
			case 1: // scratch cpu reg
				scratch(REG_AT(o->p2));
				break;
			case 2: // read vm reg
				rb--;
				copy(ctx, REG_AT(o->p2), &rb->stack, rb->size);
				scratch(REG_AT(o->p2));
				break;
			case 3: // write vm reg
				rb--;
				copy(ctx, &rb->stack, REG_AT(o->p2), rb->size);
				scratch(rb->current);
				break;
			case 4:
				if (ctx->totalRegsSize != 0)
					hl_fatal("Asm naked function should not have local variables");
				if (opCount != 0)
					hl_fatal("Asm naked function should be on first opcode");
				ctx->buf.b -= BUF_POS() - ctx->functionPos; // reset to our function start
				break;
			default:
				ASSERT(o->p1);
				break;
			}
		}
		break;
		default:
			jit_error(hl_op_name(o->op));
			break;
		}
		// we are landing at this position, assume we have lost our registers
		if (ctx->opsPos[opCount + 1] == -1)
			discard_regs(ctx, true);
		ctx->opsPos[opCount + 1] = BUF_POS();

		// write debug infos
		size = BUF_POS() - codePos;
		if (debug16 && size > 0xFF00) {
			debug32 = static_cast<int*>(malloc(sizeof(int) * (f->nops + 1)));
			for (i = 0;i < ctx->currentPos;i++)
				debug32[i] = debug16[i];
			free(debug16);
			debug16 = NULL;
		}
		if (debug16) debug16[ctx->currentPos] = (unsigned short)size; else if (debug32) debug32[ctx->currentPos] = size;

	}
	// patch jumps
	{
		jlist* j = ctx->jumps;
		while (j) {
			*(int*)(ctx->startBuf + j->pos) = ctx->opsPos[j->target] - (j->pos + 4);
			j = j->next;
		}
		ctx->jumps = NULL;
	}
	// add nops padding
	jit_nops(ctx);
	// clear regs
	for (i = 0;i < REG_COUNT;i++) {
		preg* r = REG_AT(i);
		r->holds = NULL;
		r->lock = 0;
	}
	// save debug infos
	if (ctx->debug) {
		int fid = (int)(f - m->code->functions);
		ctx->debug[fid].start = codePos;
		ctx->debug[fid].offsets = debug32 ? (void*)debug32 : (void*)debug16;
		ctx->debug[fid].large = debug32 != NULL;
	}
	// reset tmp allocator
	hl_free(&ctx->falloc);
	return codePos;
}

static void* get_wrapper(hl_type* t) {
	return call_jit_hl2c;
}

void hl_jit_patch_method(void* old_fun, void** new_fun_table) {
	// mov eax, addr
	// jmp [eax]
	unsigned char* b = (unsigned char*)old_fun;
	unsigned long long addr = (unsigned long long)(int_val)new_fun_table;
#	ifdef HL_64
	* b++ = 0x48;
	*b++ = 0xB8;
	*b++ = (unsigned char)addr;
	*b++ = (unsigned char)(addr >> 8);
	*b++ = (unsigned char)(addr >> 16);
	*b++ = (unsigned char)(addr >> 24);
	*b++ = (unsigned char)(addr >> 32);
	*b++ = (unsigned char)(addr >> 40);
	*b++ = (unsigned char)(addr >> 48);
	*b++ = (unsigned char)(addr >> 56);
#	else
	* b++ = 0xB8;
	*b++ = (unsigned char)addr;
	*b++ = (unsigned char)(addr >> 8);
	*b++ = (unsigned char)(addr >> 16);
	*b++ = (unsigned char)(addr >> 24);
#	endif
	* b++ = 0xFF;
	*b++ = 0x20;
}

static void missing_closure() {
	hl_error("Missing static closure");
}

void* hl_jit_code(jit_ctx* ctx, hl_module* m, int* codesize, hl_debug_infos** debug, hl_module* previous) {
	jlist* c;
	int size = BUF_POS();
	unsigned char* code;
	if (size & 4095) size += 4096 - (size & 4095);
	code = (unsigned char*)hl_alloc_executable_memory(size);
	if (code == NULL) return NULL;
	memcpy(code, ctx->startBuf, BUF_POS());
	*codesize = size;
	*debug = ctx->debug;
	if (!call_jit_c2hl) {
		call_jit_c2hl = code + ctx->c2hl;
		call_jit_hl2c = code + ctx->hl2c;
		hl_setup_callbacks2(callback_c2hl, get_wrapper, 1);
#		ifdef JIT_CUSTOM_LONGJUMP
		hl_setup_longjump(code + ctx->longjump);
#		endif
		int i;
		for (i = 0;i < sizeof(ctx->static_functions) / sizeof(void*);i++)
			ctx->static_functions[i] = (void*)(code + (int)(int_val)ctx->static_functions[i]);
	}
	// patch calls
	c = ctx->calls;
	while (c) {
		void* fabs;
		if (c->target < 0)
			fabs = ctx->static_functions[-c->target - 1];
		else {
			fabs = m->functions_ptrs[c->target];
			if (fabs == NULL) {
				// read absolute address from previous module
				int old_idx = m->hash->functions_hashes[m->functions_indexes[c->target]];
				if (old_idx < 0)
					return NULL;
				fabs = previous->functions_ptrs[(previous->code->functions + old_idx)->findex];
			}
			else {
				// relative
				fabs = (unsigned char*)code + (int)(int_val)fabs;
			}
		}
		if ((code[c->pos] & ~3) == (IS_64 ? 0x48 : 0xB8) || code[c->pos] == 0x68) // MOV : absolute | PUSH
			*(void**)(code + c->pos + (IS_64 ? 2 : 1)) = fabs;
		else {
			int_val delta = (int_val)fabs - (int_val)code - (c->pos + 5);
			int rpos = (int)delta;
			if ((int_val)rpos != delta) {
				printf("Target code too far too rebase\n");
				return NULL;
			}
			*(int*)(code + c->pos + 1) = rpos;
		}
		c = c->next;
	}
	// patch switchs
	c = ctx->switchs;
	while (c) {
		*(void**)(code + c->pos) = code + c->pos + (IS_64 ? 14 : 6);
		c = c->next;
	}
	// patch closures
	{
		vclosure* c = ctx->closure_list;
		while (c) {
			vclosure* next;
			int fidx = (int)(int_val)c->fun;
			void* fabs = m->functions_ptrs[fidx];
			if (fabs == NULL) {
				// read absolute address from previous module
				int old_idx = m->hash->functions_hashes[m->functions_indexes[fidx]];
				if (old_idx < 0)
					fabs = missing_closure;
				else
					fabs = previous->functions_ptrs[(previous->code->functions + old_idx)->findex];
			}
			else {
				// relative
				fabs = (unsigned char*)code + (int)(int_val)fabs;
			}
			c->fun = fabs;
			next = (vclosure*)c->value;
			c->value = NULL;
			c = next;
		}
	}
	return code;
}

