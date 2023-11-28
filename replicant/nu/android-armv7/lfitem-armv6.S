.text
.align
    
.global lfitem_init
.type lfitem_init, %function
.global lfitem_get
.type lfitem_get, %function
.global lfitem_set
.type lfitem_set, %function
.global lfitem_set_if_zero
.type lfitem_set_if_zero, %function

/* r0 = item */
lfitem_init:
	mov r1, #0
	str r1, [r0]
	bx lr

/* r0 = item
   return r0 = value */
lfitem_get:
	mov r1, #0
	mov ip, r0 /* we'll need r0 for the return value so move this parameter */
	
1:
	ldrex r0, [ip] /* load the old value */
	strex r2, r1, [ip]  /* try to store 0 */
	cmp r2, #0 /* see if the store-conditional succeeded */
	bne 1b /* try again if we got pre-empted */
	bx lr /* return value is already in r0 */

lfitem_set: 
	dmb st /* let any pending stores finish */
	mov ip, r0 /* we'll need r0 for the return value so move this parameter */
	
1:
	ldrex r0, [ip] /* load the old value */
	strex r2, r1, [ip]  /* try to store new value */
	cmp r2, #0 /* see if the store-conditional succeeded */
	bne 1b /* try again if we got pre-empted */
	bx lr /* return value is already in r0 */

lfitem_set_if_zero:
	dmb st /* let any pending stores finish */
	mov ip, r0 /* we'll need r0 for the return value so move this parameter */
	
1:
	ldrex r0, [ip] /* load the old value */
	cmp r0, #0 /* if it's zero, bail out */
	movne r0, r1
	bxne lr
	strex r2, r1, [ip]  /* try to store 0 */
	cmp r2, #0 /* see if the store-conditional succeeded */
	bne 1b /* try again if we got pre-empted */
	bx lr /* return value is already in r0 */
