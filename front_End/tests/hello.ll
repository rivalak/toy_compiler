; ModuleID = 'hello'
source_filename = "hello"

define i32 @hello1(i32 %a, i32 %b) {
entry:
  %multmp = mul i32 %b, %a
  %addtmp = add i32 %multmp, 1
  ret i32 %addtmp
}

define i32 @hello2(i32 %c, i32 %d) {
entry:
  %calltmp = call i32 @hello1(i32 %c, i32 %d)
  ret i32 %calltmp
}
