
// =====================================================================================
    static void __cdecl MixStereoSS(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {  
			index += increment;

        *dest++ += volinfo->lvolsel * srce[(himacro(index)*2)];
        *dest++ += volinfo->rvolsel * srce[(himacro(index)*2)+1];
    }
}


// =====================================================================================
    static void __cdecl MixStereoSSI(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
			register SLONG  sroot = srce[himacro(index)*2];
        *dest++ += volinfo->lvolsel * (SCAST)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2)+1];
        *dest++ += volinfo->rvolsel * (SCAST)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        index  += increment;
    }
}


// =====================================================================================
    static void __cdecl MixMonoSS(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
			*dest++ += (volinfo->lvolsel * ((srce[himacro(index)*2] + srce[(himacro(index)*2)+1]))) / 2;
        index  += increment;
    }
}


// =====================================================================================
    static void __cdecl MixMonoSSI(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {  
			register SLONG  sroot = srce[himacro(index)*2], crap;
        crap = (sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2)+1];
        *dest++ += volinfo->lvolsel * (crap + (sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))) / 2;

        index  += increment;
    }
}
