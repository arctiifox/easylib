global print_lx
global print_wn
section .text

print_lx:
    mov rax, 1 ; syscall: write
    mov rdx, rsi ; length -> rdx
    mov rsi, rdi ; str pointer -> rsi
    mov rdi, 1 ; stdout
    syscall
    ret

extern GetStdHandle
extern WriteConsoleA

print_wn:
    sub rsp, 56 ; allocate space
    mov [rsp+32], rcx ; save string pointer
    mov [rsp+40], rdx ; save length

    mov ecx, -11 ; STD_OUTPUT_HANDLE
    call GetStdHandle

    mov rcx, rax ; hConsole
    mov rdx, [rsp+32] ; str pointer
    mov r8, [rsp+40] ; len
    xor r9d, r9d ; NULL
    mov qword [rsp+32], 0 ; NULL (w/ shadow space)
    call WriteConsoleA

    add rsp, 56
    ret
