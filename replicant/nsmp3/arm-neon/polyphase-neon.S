    
	.section	.rodata
	.align	
	.type	cost32_3_4, %object
	.size	cost32_3_4, 16
cost32_3_4:
	.word	1057655764
	.word	1067924853
	.word	1060439283
	.word	0
	.type	cost32_c2, %object
	.size	cost32_c2, 16
cost32_c2:
	.word	1057128951
	.word	1058664893
	.word	1063675095
	.word	1076102863
	.type	cost32_c1, %object
	.size	cost32_c1, 32
cost32_c1:
	.word	1057005197
	.word	1057342072
	.word	1058087743
	.word	1059427869
	.word	1061799040
	.word	1065862217
	.word	1071413542
	.word	1084439708
	.type	cost32_c0, %object
	.size	cost32_c0, 64
cost32_c0:
	.word	1056974725
	.word	1057056395
	.word	1057223771
	.word	1057485416
	.word	1057855544
	.word	1058356026
	.word	1059019886
	.word	1059897405
	.word	1061067246
	.word	1062657950
	.word	1064892987
	.word	1066774581
	.word	1069414683
	.word	1073984175
	.word	1079645762
	.word	1092815430

	.text
	.align 

	.type	cost32, %function
	.global cost32
cost32:
/* r1 = f_vec (output)
	r0 = vec (input */

	/* load input into Q0-Q7 */
 	 vldm r0, {q0-q7}

	/* reverse Q4 */
	vswp		d8, d9
	vrev64.32 q4, q4
	
	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* reverse Q6 */
	vswp		d12, d13
	vrev64.32 q6, q6

	/* reverse Q6 */
	vswp		d14, d15
	vrev64.32 q7, q7

	/* ------------ Even part of transform ------------ */

	/* add vec[n] + vec[31-n] 
		results will be in Q0-Q3 (tmp0_0 - tmp0_3) */
	vadd.f32  q0, q0, q7
	vadd.f32  q1, q1, q6
	vadd.f32  q2, q2, q5
	vadd.f32  q3, q3, q4

	/* reverse Q2 */
	vswp		d4, d5
	vrev64.32 q2, q2

	/* reverse Q3 */
	vswp		d6, d7
	vrev64.32 q3, q3

	/* add vec[n] + vec[15-n] 
		results will be in Q4-Q5 (tmp1_0, tmp1_1) */
	vadd.f32  q4, q0, q3
	vadd.f32  q5, q1, q2

	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* add vec[n] + vec[7-n] 
		result will be in Q12 (final0) */
	vadd.f32  q12, q4, q5
	  
	/* Q13 will hold the next peice (final1) */
	vsub.f32  q13, q4, q5
	ldr r2, .L2
	vld1.32	{d16-d17}, [r2] /* Q8 */
	vmul.f32  q13, q13, q8

	/* next segment */
	ldr r2, .L1
	vldm r2, {q9-q10} /* Q9 (&cost_c1[0])*/ /* Q10 (&cost_c1[4])*/

	/* results will be in Q4-Q5 (tmpl1_0, tmp1_1) */
	vsub.f32  q4, q0, q3
	vmul.f32  q4, q4, q9
	vsub.f32  q5, q1, q2
	vmul.f32  q5, q5, q10

	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* Q14 will hold the next peice (final2) */
	vadd.f32  q14, q4, q5

	/* Q15 will hold the last peice (final3) */
	vsub.f32  q15, q4, q5
	vmul.f32  q15, q15, q8 /* Q8 has cost_c2 */
	
	/* rotate Q12-Q15 (final0-final3) */
	vtrn.32	q12, q13
	vtrn.32	q14, q15
	vswp	  d25, d28
	vswp	  d27, d30

	/* combine segments */
	vadd.f32 q11, q12, q15 /* a0 */
	vadd.f32 q1, q13, q14 /* a1 */

	vadd.f32 q2, q11, q1 /* b0 */

	ldr r2, .L3
	vld1.32  {d0-d1}, [r2] /* Q0 holds cost32_c3 and cost32_c4 */
	vsub.f32 q6, q11, q1 /* b2 */	
	vmul.f32 q6, q6, d1[0] /* b2 */

	vsub.f32 q4, q12, q15
	vmul.f32 q4, q4, d0[0] /* c0 */
	vsub.f32 q5, q13, q14
	vmul.f32 q5, q5, d0[1] /* c1 */
	
	vadd.f32 q3, q4, q5 /* d0 */
	vsub.f32 q7, q4, q5 /* b3 */
	vmul.f32 q7, q7, d1[0] /* b3 */
	vadd.f32 q3, q3, q7 /* b1 */
	
	 /*
	 b0 - s8:s11 (q2)
	 b1 - s12:s15 (q3)
	 b2 - s24:s27 (q6)
	 b3 - s28:s31 (q7)
	 */

	/* pre-add b0+b1, b1+b2, b2+b3 */	 
	 vadd.f32 q0, q2, q3
	 vadd.f32 q1, q3, q6
	 vadd.f32 q4, q6, q7
	 /*
	 b0+b1 - s0:s3 (q0)
	 b1+b2 - s4:s7 (q1)
	 b2+b3 - s16:s19 (q4)
	 */
	 
	fsts	s8, [r1, #0] /* f_vec[0] = b0[0] */
	fadds s0, s26, s7 /* b2[2] + (b1[3]+b2[3]) */
	fsts  s0, [r1, #8] /* f_vec[2] = b1[3]+b2[2]+b2[3] */
	fsts	s5, [r1, #16] /* f_vec[4] =  b1[1]+b2[1] */
	fadds s7, s7, s14 /* b1[2] + (b1[3]+b2[3]) */ /* DESTROY S7 */
	fsts  s7, [r1, #24] /* f_vec[6] = b1[2]+b1[3]+b2[3] */
	fsts	s12, [r1, #32] /* f_vec[8] =  b1[0] */
	fadds s14, s14, s3 /* b1[2] + (b0[3]+b1[3]) */ /* DESTROY S14 */
	fsts  s14, [r1, #40] /* f_vec[10] = b0[3]+b1[2]+b1[3] */
	fsts	s1, [r1, #48] /* f_vec[12] = b0[1]+b1[1] */
	fadds s11, s11, s3 /* b0[2] + (b0[3]+b1[3]) */ /* DESTROY S11 */
	fsts	s11, [r1, #56] /* f_vec[14] = b0[2]+b0[3]+b1[3] */
	fsts	s24, [r1, #64] /* f_vec[16] = b2[0] */
	fadds s26, s26, s19 /* b2[2] + (b2[3]+b3[3]) */ /* DESTROY S26 */
	fsts  s26, [r1, #72] /* f_vec[18] = b2[2]+b2[3]+b3[3] */
	fsts	s17, [r1, #80] /* f_vec[20] = b2[1]+b3[1] */
	fadds s19, s19, s30 /* b3[2] + (b2[3]+b3[3]) */ /* DESTROY S19 */
	fsts  s19, [r1, #88] /* f_vec[22] = b2[3]+b3[2]+b3[3] */
	fsts	s28, [r1, #96] /* f_vec[24] = b3[0] */
	fadds s30, s30, s31 /* b3[2] + b3[3] */ /* DESTROY S30 */
	fsts  s30, [r1, #104] /* f_vec[26] = b3[2]+b3[3] */
	fsts	s29, [r1, #112] /* f_vec[28] = b3[1] */	
	fsts  s31, [r1, #120] /* f_vec[30] = b3[3] */


	/* ------------ odd half ------------ */
	/* reload input into Q0-Q7 */
	 vldm r0, {q0-q7}

		/* reverse Q4 */
	vswp		d8, d9
	vrev64.32 q4, q4
	
	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* reverse Q6 */
	vswp		d12, d13
	vrev64.32 q6, q6

	/* reverse Q6 */
	vswp		d14, d15
	vrev64.32 q7, q7
	

	ldr r2, .L0
	vldm r2, {d16-d23} /* q8-q11 = cost_c0 */
	vsub.f32 q0, q0, q7
	vmul.f32 q0, q0, q8

	vsub.f32 q1, q1, q6
	vmul.f32 q1, q1, q9

	vsub.f32 q2, q2, q5
	vmul.f32 q2, q2, q10

	vsub.f32 q3, q3, q4
	vmul.f32 q3, q3, q11
  
	/* reverse Q2 */
	vswp		d4, d5
	vrev64.32 q2, q2

	/* reverse Q3 */
	vswp		d6, d7
	vrev64.32 q3, q3

	/* add vec[n] + vec[15-n] 
		results will be in Q4-Q5 (tmp1_0, tmp1_1) */
	vadd.f32  q4, q0, q3
	vadd.f32  q5, q1, q2

	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* add vec[n] + vec[7-n] 
		result will be in Q12 (final0) */
	vadd.f32  q12, q4, q5
	  
	/* Q13 will hold the next peice (final1) */
	vsub.f32  q13, q4, q5
	ldr r2, .L2
	vld1.32	{d16-d17}, [r2] /* Q8 */
	vmul.f32  q13, q13, q8

	/* next segment */
	ldr r2, .L1
	vldm r2, {q9-q10} /* Q9 (&cost_c1[0])*/ /* Q10 (&cost_c1[4])*/

	/* results will be in Q4-Q5 (tmpl1_0, tmp1_1) */
	vsub.f32  q4, q0, q3
	vmul.f32  q4, q4, q9
	vsub.f32  q5, q1, q2
	vmul.f32  q5, q5, q10

	/* reverse Q5 */
	vswp		d10, d11
	vrev64.32 q5, q5

	/* Q14 will hold the next peice (final2) */
	vadd.f32  q14, q4, q5

	/* Q15 will hold the last peice (final3) */
	vsub.f32  q15, q4, q5
	vmul.f32  q15, q15, q8 /* Q8 has cost_c2 */
	
	/* rotate Q12-Q15 (final0-final3) */
	vtrn.32	q12, q13
	vtrn.32	q14, q15
	vswp	  d25, d28
	vswp	  d27, d30

	/* combine segments */
	vadd.f32 q11, q12, q15 /* a0 */
	vadd.f32 q1, q13, q14 /* a1 */

	vadd.f32 q2, q11, q1 /* b0 */

	ldr r2, .L3
	vld1.32  {d0-d1}, [r2] /* Q0 holds cost32_c3 and cost32_c4 */
	vsub.f32 q4, q11, q1 /* b2 */	
	vmul.f32 q4, q4, d1[0] /* b2 */

	vsub.f32 q6, q12, q15
	vmul.f32 q6, q6, d0[0] /* c0 */
	vsub.f32 q5, q13, q14
	vmul.f32 q5, q5, d0[1] /* c1 */
	
	vadd.f32 q3, q6, q5 /* d0 */
	vsub.f32 q5, q6, q5 /* b3 */
	vmul.f32 q5, q5, d1[0] /* b3 */
	vadd.f32 q3, q3, q5 /* b1 */
	
	 /*
	 b0 - s8:s11 (q2)
	 b1 - s12:s15 (q3)
	 b2: s16:s19 (q4)
	 b3: s20:s23 (q5)
	 */

	/* pre-add b0+b1, b1+b2, b2+b3 */	 
	 vadd.f32 q0, q2, q3
	 vadd.f32 q1, q3, q4
	 vadd.f32 q6, q4, q5
	 /*
	 b0+b1 - s0:s3 (q0)
	 b1+b2 - s4:s7 (q1)
	 b2+b3 s24:s27 (q6)
	 */	 

	fadds s16, s16, s18
	fadds s0, s7, s16
	fsts  s0, [r1, #4] /*	f_vec[1] = b1[3]+b2[0]		+b2[2]+b2[3] */ /* s7 + s16 + s18 */
	fadds s5, s5, s7
	fadds s0, s14, s5
	fadds s5, s5, s18
	fsts  s5, [r1, #12] /*	f_vec[3] = b1[1]		+b1[3]		+b2[1]+b2[2]+b2[3] */ /* s7 + s5  + s18 */
	fsts  s0, [r1, #20] /*	f_vec[5] = b1[1]+b1[2]+b1[3]		+b2[1]		+b2[3] */ /* s5 + s7  + s14 */ 
	fadds s12, s12, s14
	fadds s7, s7, s12
	fsts  s7, [r1, #28] /*	f_vec[7] = b1[0]		+b1[2]+b1[3]						+b2[3] */ /* s7 + s14 + s12 */
	fadds s12, s12, s3
	fsts  s12, [r1, #36] /*	f_vec[9] = b0[3]+b1[0]		+b1[2]+b1[3] */ /* s3 + s12 + s14 */
	fadds s14, s14, s3
	fadds s14, s14, s1
	fsts  s14, [r1, #44] /*	f_vec[11] =		 b0[1]		+b0[3]		+b1[1]+b1[2]+b1[3] */ /* s1 + s3 + s14 */
	fadds s3, s3, s10
	fadds s1, s1, s3
	fsts  s1, [r1, #52]  /*	f_vec[13] =		 b0[1]+b0[2]+b0[3]		+b1[1]		+b1[3] */ /* s1 + s3 + s10 */
	fadds s8, s8, s3
	fsts  s8, [r1, #60] /*	f_vec[15] = b0[0]		+b0[2]+b0[3]						+b1[3] */ /* s3 + s10 + s8 */
	fadds s18, s18, s27
	fadds s16, s16, s27
	fsts  s16, [r1, #68] /*	f_vec[17] = b2[0]		+b2[2]+b2[3] +b3[3] */ /* s27 + s18 + s16 */
	fadds s18, s18, s25
	fsts  s18, [r1, #76] /*	f_vec[19] = b2[1]+b2[2]+b2[3]		+b3[1]		+b3[3] */ /* s25 + s27 + s18 */
	fadds s25, s25, s27
	fadds s25, s25, s22
	fsts  s25, [r1, #84] /*	f_vec[21] = b2[1] +b2[3] +b3[1]+b3[2]+b3[3] */ /* s25 + s27 + s22 */
	fadds s20, s20, s22
	fadds s27, s27, s20
	fsts  s27, [r1, #92] /*	f_vec[23] = b2[3]+b3[0]+b3[2]+b3[3] */ /* s27 + s20 + s22 */
	fsubs s20, s27, s19 /* for some (coincidental?) reason this is more precise than adding s20+s22+s23.  not sure why */
	fsts  s20, [r1, #100] /*	f_vec[25] = b3[0]+b3[2]+b3[3] */ /* s20 + s22 + s23 */
	fadds s21, s21, s23 /* b3[1] +b3[3] */
	fadds s22, s22, s21 /* b3[1]+b3[2]+b3[3] */
	fsts  s22, [r1, #108] /*	f_vec[27] = b3[1]+b3[2]+b3[3] */ /* s21 + s22 + s23 */
	fsts  s21, [r1, #116] /*	f_vec[29] = b3[1] +b3[3] */ /* s21 + s23 */
	fsts  s23, [r1, #124] /*	f_vec[31] = b3[3] */ /* s23 */

	bx lr
.L0: .word cost32_c0
.L1: .word cost32_c1
.L2: .word cost32_c2
.L3: .word cost32_3_4
