
start = 'A'
end = 'z'

endOrd = ord(end)
startOrd = ord(start)
print("{ ", end = '')
prio = 1
while startOrd < endOrd:
    if startOrd < ord('Z'):
        print(prio, ", ", end = '')
        prio =  prio + 1
    elif startOrd > ord('a'):
        print(prio + 1, ", ", end = '')
        prio =  prio + 1
    else:
        print(' 0,', end='')
    startOrd = startOrd + 1

print("}")