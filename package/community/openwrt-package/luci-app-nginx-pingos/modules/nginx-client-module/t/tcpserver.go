package main

import (
	"fmt"
	"net"
)

func handleConnection(c net.Conn) {
	b := make([]byte, 4096)

	for {
		n, err := c.Read(b)
		if err != nil {
			fmt.Print("Read Error ", err)
			c.Close()
			return
		}

		fmt.Print("recv ", n, " data:", string(b))

		c.Write(b[0:n])
	}
}

func main() {
	ln, err := net.Listen("tcp", ":10000")
	if err != nil {
		fmt.Print("Listen Error ", err)
		return
	}

	for {
		conn, err := ln.Accept()
		if err != nil {
			fmt.Print("Accept Error ", err)
			continue
		}

		go handleConnection(conn)
	}
}
