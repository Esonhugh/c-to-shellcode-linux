package main

import "C"
import (
	"fmt"
)


func init() {
	fmt.Println("Helloworld here is golang");
}


func main() {
	Main()
}

//export Main
func Main() {
	fmt.Println("here is go!");
}