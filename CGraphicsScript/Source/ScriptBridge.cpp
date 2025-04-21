#include "CGR/Core/Log.h"

#define HL_NAME(n) CGraphicsCore_##n

#include "hl.h"


HL_PRIM void HL_NAME(cgr_trace)(vstring* message) {
    const char* msg = hl_to_utf8(message->bytes);
    CGR_TRACE("{0}", msg);
}

HL_PRIM void HL_NAME(cgr_info)(vstring* message) {
    const char* msg = hl_to_utf8(message->bytes);
    CGR_INFO("{0}", msg);
}

DEFINE_PRIM(_VOID, cgr_trace, _STRING);
DEFINE_PRIM(_VOID, cgr_info, _STRING);