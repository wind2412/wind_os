.text
.globl switch_to
switch_to:

    movl 4(%esp), %eax		//两个参数刚刚推入栈中。由于cdecl，从右往左入栈。于是current会在后边，next会在前边。			esp指向一个“栈位置”，*(esp+4)能得到cur->context的真正地址。放入%eax中。

    popl 0(%eax)			//由于call switch_to的时候，会把call switch_to的下一个指令的eip压栈。由于不是c函数，既没有参数，也不需要ebp和esp(别忘了ebp和esp就是C语言为了方便寻找参数弄的)。因此此时esp指向call时压入的eip，直接pop出来就好。
    movl %ebx, 4(%eax)
    movl %ecx, 8(%eax)
    movl %edx, 12(%eax)
    movl %esi, 16(%eax)
    movl %edi, 20(%eax)
    movl %ebp, 24(%eax)
    movl %esp, 28(%eax)

    movl 4(%esp), %eax		//由于%eip在一开始的时候就被pop出来了，因此这里其实是 + 4 就够了。而不是+8.

    movl 28(%eax), %esp
    movl 24(%eax), %ebp
    movl 20(%eax), %edi
    movl 16(%eax), %esi
    movl 12(%eax), %edx
    movl 8(%eax), %ecx
    movl 4(%eax), %ebx

    pushl 0(%eax)         	//push进去待转换next进程的eip。从next->context.eip那里push，由ret直接隐式执行pop %eip来跳到另一个进程。

    ret
