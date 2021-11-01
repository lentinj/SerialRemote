# https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_NEC.cpp#L48-L60

def reverseBits(n, bits):
    if bits != 8:
        NotImplemented
    return int('{:08b}'.format(n)[::-1], 2)

def encode(address, command):
    command = command & 0xFF;  # We only want the least significant byte of command.
    # sendNEC() sends MSB first, but protocol says this is LSB first.
    command = reverseBits(command, 8);
    command = (command << 8) + (command ^ 0xFF);  # Calculate the new command.
    if address > 0xFF:                         # Is it Extended NEC?
        address = reverseBits(address, 16);
        return ((address << 16) + command);  # Extended.
    else:
        address = reverseBits(address, 8);
        return (address << 24) + ((address ^ 0xFF) << 16) + command;  # Normal.

for c in "01 yefrnpszmud":
    print('"%s": IRsend {"Protocol":"NEC","Bits":32,"Data":0x%x}' % (
        c,
        encode(0x42, ord(c)),
    ))

for c in [0x01]:
    print('0x%x: IRsend {"Protocol":"NEC","Bits":32,"Data":0x%x}' % (
        c,
        encode(0x42, c),
    ))

