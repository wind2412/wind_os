.text


my_push:
	pusha				//详见idtframe结构体的说明。
	movw %ds, %ax
	pushl %eax

	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	pushl %esp

	call trap

	addl $0x04, %esp

	popl %eax

	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	popa

	addl $0x08, %esp

	iret


.macro my_int_no_error 	arg1
//.globl my_int_no_error_\arg1
intr_\@:
pushl $0
pushl \arg1
jmp my_push
.endm

.macro my_int_error		arg1
//.globl my_int_error_arg1
intr_\@:			//使用GAS  \@:生成全局标签。\@是gas内部macro的数量。 详见:http://www.coranac.com/files/gba/re-ejected-gasref.pdf
					//以及：https://sourceware.org/binutils/docs/as/Macro.html#Macro
pushl \arg1
jmp my_push
.endm

my_int_no_error 0
my_int_no_error 1
my_int_no_error 2
my_int_no_error 3
my_int_no_error 4
my_int_no_error 5
my_int_no_error 6
my_int_no_error 7

my_int_error 	8
my_int_error 	9
my_int_error 	10
my_int_error 	11
my_int_error 	12
my_int_error 	13
my_int_error 	14

my_int_no_error 15
my_int_no_error 16
my_int_no_error 17
my_int_no_error 18
my_int_no_error 19
my_int_no_error 20
my_int_no_error 21
my_int_no_error 22
my_int_no_error 23
my_int_no_error 24
my_int_no_error 25
my_int_no_error 26
my_int_no_error 27
my_int_no_error 28
my_int_no_error 29
my_int_no_error 30
my_int_no_error 31
my_int_no_error 32
my_int_no_error 33
my_int_no_error 34
my_int_no_error 35
my_int_no_error 36
my_int_no_error 37
my_int_no_error 38
my_int_no_error 39
my_int_no_error 40
my_int_no_error 41
my_int_no_error 42
my_int_no_error 43
my_int_no_error 44
my_int_no_error 45
my_int_no_error 46
my_int_no_error 47
my_int_no_error 48

.data
.globl _intrs
_intrs:
	.long intr_0
	.long intr_1
	.long intr_2
	.long intr_3
	.long intr_4
	.long intr_5
	.long intr_6
	.long intr_7

	.long intr_8
	.long intr_9
	.long intr_10
	.long intr_11
	.long intr_12
	.long intr_13
	.long intr_14

	.long intr_15
	.long intr_16
	.long intr_17
	.long intr_18
	.long intr_19
	.long intr_20
	.long intr_21
	.long intr_22
	.long intr_23
	.long intr_24
	.long intr_25
	.long intr_26
	.long intr_27
	.long intr_28
	.long intr_29
	.long intr_30
	.long intr_31
	.long intr_32
	.long intr_33
	.long intr_34
	.long intr_35
	.long intr_36
	.long intr_37
	.long intr_38
	.long intr_39
	.long intr_40
	.long intr_41
	.long intr_42
	.long intr_43
	.long intr_44
	.long intr_45
	.long intr_46
	.long intr_47
	.long intr_48
