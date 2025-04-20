%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat
extern strlen
extern strcpy

; lista vacia
string_proc_list_create_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 16             
    mov     edi, 16    ; 16 bytes first y last 
            
    call    malloc     
    test rax, rax      ; malloc check
    je .null_return


    xor     rdx, rdx   ; punteros a null      
    mov     [rax], rdx       
    mov     [rax + 8], rdx       
    add     rsp, 16
    pop     rbp
    ret

.null_return:
    xor rax, rax
    add rsp, 16
    pop rbp
    ret

; crear nodo
string_proc_node_create_asm:
    push    rbp
    mov     rbp, rsp

    ; type, hash, node 
    mov     r12b, dil            
    mov     r13,  rsi          
    mov     edi, 32

    call    malloc            
    test    rax, rax
    je      .null_return

    xor     r14, r14        ; prev y next a null     
    mov     [rax], r14       
    mov     [rax + 8], r14      
    mov     byte [rax + 16], r12b 
    mov     [rax + 24], r13

    pop     rbp
    ret

.null_return:
    xor rax, rax
    pop rbp
    ret

; agergar node a list
string_proc_list_add_node_asm:
    mov rbx, rdi            

    ; Creamos el nodo
    mov rdi, rsi
    mov rsi, rdx
    call string_proc_node_create_asm    ; funcion previa
    test rax, rax
    je .end                 

    mov rcx, [rbx]          
    test rcx, rcx
    jnz .not_empty         ; Si no es 0, lista no vacía

    ; caso lista vacia
    mov [rbx], rax
    mov [rbx + 8], rax
    jmp .end

.not_empty:
    mov rcx, [rbx + 8]      ; rcx = list->last

    ; updateo la lista y el nuevo nodo
    mov [rax + 8], rcx      ; nuevo->prev = last
    mov [rcx], rax          ; last->next = nuevo
    mov [rbx + 8], rax      ; list->last = nuevo
    ret

.end:
    ret

; concatenar los hash si coincide tipo
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    sub rsp, 40             ; reserva

    mov [rsp], rdi
    mov [rsp+8], rsi
    mov [rsp+16], rdx        ; base

    
    mov rdi, rdx
    call strlen
    lea rdi, [rax+1]
    call malloc
    test rax, rax
    jz .null_return

    mov [rsp+24], rax       
    mov rdi, rax
    mov rsi, [rsp+16]
    call strcpy

    mov rdi, [rsp]
    mov rcx, [rdi]          
    mov [rsp+32], rcx

.loop:
    mov rcx, [rsp+32]
    test rcx, rcx
    jz .end                

    ; type check
    mov dl, [rcx+16]
    cmp dl, byte [rsp+8]
    jne .nodo_update


    mov rdi, [rsp+24]
    mov rsi, [rcx+24]
    call str_concat


    mov rdi, [rsp+24]
    mov [rsp+24], rax
    call free

.nodo_update:
    mov rcx, [rsp+32]
    mov rcx, [rcx]
    mov [rsp+32], rcx
    jmp .loop

.end:
    mov rax, [rsp+24] 
    add rsp, 40
    pop rbp
    ret


.null_return:
    mov rax, 0
    add rsp, 40
    pop rbp
    ret