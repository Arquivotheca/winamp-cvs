#include "frames.h"

// order needs to match the enum in nsid3v2.h
const FrameID frame_ids[] =
{
	{"PIC", "APIC", "APIC"},
	{"COM", "COMM", "COMM"},
	{"POP", "POPM", "POPM"},
	{"TAL", "TALB", "TALB"},
	{"TBP", "TBPM", "TBPM"},
	{"TCM", "TCOM", "TCOM"},
	{"TCO", "TCON", "TCON"},
	{"TCR", "TCOP", "TCOP"},
	{"TDA", "TDAT", "TDAT"},
	{"TDY", "TDLY", "TDLY"},
	{0, 0, "TDRC"},
	{"TEN", "TENC", "TENC"},
	{0, "TEXT", "TEXT"}, 
	{"TFT", "TFLT", "TFLT"}, 
	{"TIM", "TIME", "TIME"},
	{"TT1", "TIT1", "TIT1"},
	{"TT2", "TIT2", "TIT2"},
	{"TT3", "TIT3", "TIT3"},	
	{"TKE", "TKEY", "TKEY"},
	{"TLA", "TLAN", "TLAN"},
	{"TLE", "TLEN", "TLEN"},
	{"TMT", "TMED", "TMED"},
	{0, 0, "TMOO"},
	{0, "TOAL", "TOAL"},
	
	{"TOA", "TOPE", "TOPE"},


{"TP1", "TPE1", "TPE1"},
{"TP2", "TPE2", "TPE2"},
{"TP3", "TPE3", "TPE3"},
{"TP4", "TPE4", "TPE4"},
{"TPA", "TPOS", "TPOS"},
	{"TPB", "TPUB", "TPUB"},
	{"TRK", "TRCK", "TRCK"},
	{"TRD", "TRDA", "TRDA"},

	{"TRC", "TSRC", "TSRC"},
	{"TSS", "TSSE", "TSSE"},
	{"TYE", "TYER", "TYER"},

{"TXX", "TXXX", "TXXX"},


};