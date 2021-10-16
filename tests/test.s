.global main
.global puts
msg:	
	.string "Hello World!"
main:
	xorq %rax, %rax
	movq $msg, %rdi
	movq $puts, %rbx
	callq *%rbx
	xorq %rax, %rax
	ret
