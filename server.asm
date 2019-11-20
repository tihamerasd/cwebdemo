;; created by Tihamer Darai
;; Web server engine based on linux syscall, the main is in the c source
;;nasm -felf64 -o server.o server.asm
;;ld server.o -o server
;;My job is based on this github project: https://gist.github.com/bobbo/e1e980262f2ddc8db3b8

;;TODO move the .bss and .data section to c for PIE compile

global _start

;; Data definitions
struc sockaddr_in
    .sin_family resw 1
    .sin_port resw 1
    .sin_addr resd 1
    .sin_zero resb 8
endstruc

section .bss
    sock resw 2
    client resw 2
    echobuf resb 2048
    read_count resw 2

section .data
    sock_err_msg        db "Failed to initialise socket", 0x0a, 0
    sock_err_msg_len    equ $ - sock_err_msg

    bind_err_msg        db "Failed to bind socket to listening address", 0x0a, 0
    bind_err_msg_len    equ $ - bind_err_msg

    lstn_err_msg        db "Failed to listen on socket", 0x0a, 0
    lstn_err_msg_len    equ $ - lstn_err_msg

    accept_err_msg      db "Could not accept connection attempt", 0x0a, 0
    accept_err_msg_len  equ $ - accept_err_msg

    accept_msg          db "<!DOCTYPE html> <html><body><h1>asm everywhere</h1></body></html>", 0x0a, 0
    accept_msg_len      equ $ - accept_msg

    response_header     db 'HTTP/1.1 200 OK', 10, 13, 'Server: asm_server', 10, 13, 'Content-Type: text/html', 10, 13, 'Connection: Closed', 10, 13, 10, 13
    response_header_len equ $ - response_header
    
    ;; sockaddr_in structure for the address the listening socket binds to
    pop_sa istruc sockaddr_in
        at sockaddr_in.sin_family, dw 2            ; AF_INET
        ;at sockaddr_in.sin_port, dw 0x921f        ; port 8080
        at sockaddr_in.sin_port, dw 0x31d4        ; port 54321
        at sockaddr_in.sin_addr, dd 0             ; localhost
        at sockaddr_in.sin_zero, dd 0, 0
    iend
    sockaddr_in_len     equ $ - pop_sa

section .text

GLOBAL _socket
GLOBAL _accept
GLOBAL _accept_fail
GLOBAL _close_sock
GLOBAL _echo
GLOBAL _echo_header
GLOBAL _exit
GLOBAL _fail
GLOBAL _listen
GLOBAL _listen_fail
GLOBAL _read
GLOBAL _socket_fail
GLOBAL initcode
GLOBAL closesock
GLOBAL client_to_rdi

client_to_rdi:
	mov		rax, 1
	mov     rdi, [client]
	syscall
	ret

initcode:
	mov      word [sock], 0
	mov      word [client], 0
	jmp _socket

; It's for the correct close after response
closesock:
	mov    rdi, [client]
	mov     rax, 3        ; SYS_CLOSE
    syscall
	mov    word [client], 0

; It's a close on error; TODO maybe we could do it better
_close_sock:
	mov     rax, 3        ; SYS_CLOSE
    syscall
	ret

;; Performs a sys_socket call to initialise a TCP/IP listening socket, storing 
;; socket file descriptor in the sock variable
_socket:
    mov         rax, 41     ; SYS_SOCKET
    mov         rdi, 2      ; AF_INET
    mov         rsi, 1      ; SOCK_STREAM
    mov         rdx, 0    
    syscall

    ;; Check socket was created correctly
    cmp        rax, 0
    jle        _socket_fail

    ;; Store socket descriptor in variable
    mov        [sock], rax

    ret

;; Calls sys_bind and sys_listen to start listening for connections
_listen:
    mov        rax, 49                  ; SYS_BIND
    mov        rdi, [sock]              ; listening socket fd
    mov        rsi, pop_sa              ; sockaddr_in struct
    mov        rdx, sockaddr_in_len     ; length of sockaddr_in
    syscall

    ;; Check call succeeded
    cmp        rax, 0
    jl         _bind_fail

    ;; Bind succeeded, call sys_listen
    mov        rax, 50          ; SYS_LISTEN
    mov        rsi, 1           ; backlog (dummy value really)
    syscall

    ;; Check for success
    cmp        rax, 0
    jl         _listen_fail
	
    ret

;; Accepts a connection from a client, storing the client socket file descriptor
;; in the client variable and logging the connection to stdout
_accept:
   
    ;; Call sys_accept
    mov       rax, 43         ; SYS_ACCEPT
    mov       rdi, [sock]     ; listening socket fd
    mov       rsi, 0          ; NULL sockaddr_in value as we don't need that data
    mov       rdx, 0          ; NULLs have length 0
    syscall

    ;; Check call succeeded
    cmp       rax, 0
    jl        _accept_fail

    ;; Store returned fd in variable
    mov     [client], rax

    ret

;; Reads up to 2048 bytes from the client into echobuf and sets the read_count variable
;; to be the number of bytes read by sys_read
_read:
    ;; Call sys_read
    mov     rax, 0          ; SYS_READ
    mov     rdi, [client]   ; client socket fd
    mov     rsi, echobuf    ; buffer
    mov     rdx, 2048       ; read 2048 bytes 
    syscall 

    ;; Copy number of bytes read to variable
    ;mov     [read_count], rax
    ret 

;; Sends up to the value of http body to the client socket
;; using sys_write 
_echo:
    mov     rax, 1               ; SYS_WRITE
    mov     rdi, [client]        ; client socket fd
    mov     rsi, accept_msg         ; buffer
    mov     rdx, accept_msg_len    ; number of bytes received in _read
    syscall

    ret

;; Sends up to the value of header
;; using sys_write 
_echo_header:
    mov     rax, 1               ; SYS_WRITE
    mov     rdi, [client]        ; client socket fd
    mov     rsi, response_header         ; buffer
    mov     rdx, response_header_len    ; number of bytes received in _read
    syscall

    ret

;; Error Handling code
;; _*_fail handle the population of the rsi and rdx registers with the correct
;; error messages for the labelled situation. They then call _fail to show the
;; error message and exit the application.
_socket_fail:
    mov     rsi, sock_err_msg
    mov     rdx, sock_err_msg_len
    call    _fail

_bind_fail:
    mov     rsi, bind_err_msg
    mov     rdx, bind_err_msg_len
    call    _fail

_listen_fail:
    mov     rsi, lstn_err_msg
    mov     rdx, lstn_err_msg_len
    call    _fail

_accept_fail:
    mov     rsi, accept_err_msg
    mov     rdx, accept_err_msg_len
    call    _fail

;; Calls the sys_write syscall, writing an error message to stderr, then exits
;; the application. rsi and rdx must be populated with the error message and
;; length of the error message before calling _fail
_fail:
    mov        rax, 1 ; SYS_WRITE
    mov        rdi, 2 ; STDERR
    syscall

    mov        rdi, 1
    call       _exit

;; Exits cleanly, checking if the listening or client sockets need to be closed
;; before calling sys_exit
_exit:
    mov        rax, [sock]
    cmp        rax, 0
    je         .client_check
    mov        rdi, [sock]
    call       _close_sock

    .client_check:
    mov        rax, [client]
    cmp        rax, 0
    je         .perform_exit
    mov        rdi, [client]
    call       _close_sock

    .perform_exit:
    mov        rax, 60
    syscall
