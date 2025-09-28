#const test str "test"
#const mynuts str " my nuts!\n"
#const icles str "icles!\n"

#const key str "key"
#const value str "value\n"

const test ; 0

const mynuts ; 1
str.add 0 1 ; 2
str.print 2
pop 2

const icles ; 1
str.add 0 1 ; 2
str.print 2
pop 3

obj.new ; 0
const key ; 1
const value ; 2
obj.set 0 1 2
pop 1

obj.get 0 1 ; 2
str.print 2