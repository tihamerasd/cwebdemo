package main

import (
    "log"
    "crypto/tls"
    "net"
    "fmt"
    "unsafe"
)

/*
#cgo CFLAGS: -I../DyLib
#cgo LDFLAGS: -L. -lshared_cweb -lflate -lhttp_parser -lcrypto
#include "sharedinterface.c"
*/
import "C"

func main() {
    log.SetFlags(log.Lshortfile)
    C.globalinit_cache()
    C.controllercall()
	C.globalinit_cache()
	C.sqlite_init_function()
	C.init_callback_sql()
	C.my_sighandler()

	fmt.Printf("%s\n", "INIT done\n")

    cer, err := tls.LoadX509KeyPair("/root/fullchain.pem", "/root/privkey.pem")
    if err != nil {
        log.Println(err)
        return
    }

    config := &tls.Config{Certificates: []tls.Certificate{cer}}
    ln, err := tls.Listen("tcp", ":443", config) 
    if err != nil {
        log.Println(err)
        return
    }
    defer ln.Close()

    for {
        conn, err := ln.Accept()
        if err != nil {
            log.Println(err)
            continue
        }
        go handleConnection(conn)
    }
}

func handleConnection(conn net.Conn) {
    defer conn.Close()
   
	//r := bufio.NewReader(conn)
	buffer := make([]byte, 1024)
    for {
        msg, err := conn.Read(buffer)
        if err != nil {
            log.Println(err)
            return
        }

        var str string
		out_p := C.CString(str)
		in_p := C.CString(string(buffer[:msg]))

		out_p = C.packed_function(in_p)
		log.Println(string(buffer[:msg]))

		output_length :=C.sdslen(out_p)
		go_str := C.GoBytes(unsafe.Pointer(out_p), (C.int) (output_length))
		C.sdsfree(out_p)
		C.free(unsafe.Pointer(in_p))
        n, err := conn.Write([]byte(go_str))
        if err != nil {
            log.Println(n, err)
            return
        }
	conn.Close()
    }
}
