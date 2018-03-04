package msg64

import (
	"strings"

	"github.com/coyove/goflyway/pkg/rand"
)

var crc16Table [256]uint16
var r *rand.ConcurrentRand

func init() {
	for i := 0; i < 256; i++ {
		crc16Table[i] = uint16(0x1021 * i)
	}

	r = rand.New()
}

func crc16(v interface{}) uint16 {
	crc := uint16(0)

	switch v.(type) {
	case []byte:
		buf := v.([]byte)
		for i := 0; i < len(buf); i++ {
			crc = ((crc << 8) & 0xFF00) ^ crc16Table[((crc>>8)&0xFF)^uint16(buf[i])]
		}
	case string:
		str := v.(string)
		for i := 0; i < len(str); i++ {
			crc = ((crc << 8) & 0xFF00) ^ crc16Table[((crc>>8)&0xFF)^uint16(str[i])]
		}
	}

	return crc
}

type BitsArray struct {
	idx      int
	bit8     [8]byte
	underlay []byte
}

func NewBitsArray(ln int) *BitsArray {
	return &BitsArray{
		underlay: make([]byte, 0, ln),
	}
}

func (b *BitsArray) push(bit byte) {
	b.bit8[b.idx] = bit
	b.idx++

	if b.idx == 8 {
		n := byte(0)
		for i := 0; i < 8; i++ {
			n += b.bit8[i] << byte(7-i)
		}

		b.underlay = append(b.underlay, n)
		b.idx = 0
	}
}

func (b *BitsArray) Push(bits ...byte) {
	for _, bit := range bits {
		if bit != 0 && bit != 1 {
			panic("bit can only be 0 or 1")
		}

		b.push(bit)
	}
}

func (b *BitsArray) PushByte(n byte, w int) {
	if w > 8 || w < 1 {
		return
	}

	for i := w - 1; i >= 0; i-- {
		b.push((n >> byte(i)) & 0x01)
	}
}

func (b *BitsArray) GetBytes() []byte {
	buf := b.underlay

	if b.idx != 0 {
		n := byte(0)
		for i := 0; i < b.idx; i++ {
			n += b.bit8[i] << byte(7-i)
		}
		buf = append(buf, n)
	}

	return buf
}

func (b *BitsArray) RemainingBitsToOneByte() int {
	return (8 - b.idx) % 8
}

var table = map[byte]byte{
	/* 000000 */ 0: 'a',
	/* 000001 */ 1: 'b',
	/* 000010 */ 2: 'c',
	/* 000011 */ 3: 'd',
	/* 000100 */ 4: 'e',
	/* 000101 */ 5: 'f',
	/* 000110 */ 6: 'g',
	/* 000111 */ 7: 'h',
	/* 001000 */ 8: 'i',
	/* 001001 */ 9: 'j',
	/* 001010 */ 10: 'k',
	/* 001011 */ 11: 'l',
	/* 001100 */ 12: 'm',
	/* 001101 */ 13: 'n',
	/* 001110 */ 14: 'o',
	/* 001111 */ 15: 'p',
	/* 010000 */ 16: 'q',
	/* 010001 */ 17: 'r',
	/* 010010 */ 18: 's',
	/* 010011 */ 19: 't',
	/* 010100 */ 20: 'u',
	/* 010101 */ 21: 'v',
	/* 010110 */ 22: 'w',
	/* 010111 */ 23: 'x',
	/* 011000 */ 24: 'y',
	/* 011001 */ 25: 'z',
	/* 011010 */ 26: '0',
	/* 011011 */ 27: '1',
	/* 011100 */ 28: '2',
	/* 011101 */ 29: '3',
	/* 011110 */ 30: '4',
	/* 011111 */ 31: '5',
	/* 100000 */ 32: '6',
	/* 100001 */ 33: '7',
	/* 100010 */ 34: '8',
	/* 100011 */ 35: '9',
	/* 100100 */ 36: 'A',
	/* 100101 */ 37: 'B',
	/* 100110 */ 38: 'C',
	/* 100111 */ 39: 'D',
	/* 101000 */ 40: 'E',
	/* 101001 */ 41: 'F',
	/* 101010 */ 42: '-',
	/* 101011 */ 43: '.',
	/* 101100 */ 44: '_',
	/* 101101 */ 45: 'H',
	/* 101110 */ 46: ':',
	/* 101111 */ 47: '/',
	/* 110000 */ 48: '?',
	/* 110001 */ 49: '#',
	/* 110010 */ 50: '+',
	/* 110011 */ 51: '%',
	/* 110100 */ 52: ',',
	/* 110101 */ 53: '!',
	/* 110110 */ 54: '=',
	/* 110111 */ 55: '&',
	/* 111000 */ 56: '*',
	/* 111001 */ 57: '(',
	/* 111010 */ 58: ')',
	/* 111011 */ 59: 'S',

	// control bits
	// 111100    60: HASH, followed by 15 bits
	// 111101    61: CAPS, followed by 5 bits
	// 111110    62: RAW, followed by 8 bits, a raw byte
	// 111111    63: END, end of the message
}

const (
	_HASH = 60
	_CAPS = 61
	_RAW  = 62
)

var itable = map[byte]byte{
	'a': 0, 'b': 1, 'c': 2, 'd': 3, 'e': 4, 'f': 5, 'g': 6,
	'h': 7, 'i': 8, 'j': 9, 'k': 10, 'l': 11, 'm': 12, 'n': 13,
	'o': 14, 'p': 15, 'q': 16, 'r': 17, 's': 18, 't': 19,
	'u': 20, 'v': 21, 'w': 22, 'x': 23, 'y': 24, 'z': 25,
	'0': 26, '1': 27, '2': 28, '3': 29, '4': 30,
	'5': 31, '6': 32, '7': 33, '8': 34, '9': 35,
	'A': 36, 'B': 37, 'C': 38, 'D': 39, 'E': 40, 'F': 41,
	'-': 42, '.': 43, '_': 44, 'H': 45, ':': 46, '/': 47,
	'?': 48, '#': 49, '+': 50, '%': 51, ',': 52, '!': 53,
	'=': 54, '&': 55, '*': 56, '(': 57, ')': 58, 'S': 59,
}

func charToIdx(b byte) (byte, byte, int) {
	if b > 'F' && b <= 'Z' && b != 'S' && b != 'H' {
		return _CAPS, b - 'A', 11
	}

	b2, ok := itable[b]
	if !ok {
		return _RAW, b, 14
	}

	return b2, 0, 6
}

func Compress(str string) []byte {
	ln := len(str)
	if ln == 0 {
		return []byte{}
	}

	b := NewBitsArray(ln)
	crc := crc16(str)

	if strings.HasPrefix(str, "https://") {
		b.PushByte(3, 2)
		str = str[8:]
	} else if strings.HasPrefix(str, "http://") {
		b.PushByte(2, 2)
		str = str[7:]
	} else {
		b.PushByte(0, 2)
	}

	ln = len(str)
	inserted := false
	for i := 0; i < ln; i++ {
		if !inserted && r.Intn(ln-i) == 0 {
			b.PushByte(_HASH, 6)
			b.PushByte(byte(crc>>8), 7)
			b.PushByte(byte(crc), 8)
			inserted = true
		}

		n, n2, w := charToIdx(str[i])
		switch w {
		case 6:
			b.PushByte(n, w)
		case 11:
			b.PushByte(n, 6)
			b.PushByte(n2, 5)
		case 14:
			b.PushByte(n, 6)
			b.PushByte(n2, 8)
		}
	}

	w := b.RemainingBitsToOneByte()
	b.PushByte(0xFF, w)

	return b.GetBytes()
}

func Decompress(buf []byte) string {
	idx := 0
	readidx := 0

	_read := func(w int) (byte, bool) {
		if readidx >= len(buf) {
			return 0, false
		}

		curbyte := buf[readidx]
		// j := 1
		n := byte(0)

		if idx+w > 8 {
			// we need next byte
			if readidx+1 >= len(buf) {
				return 0xFF, false
			}

			nextbyte := buf[readidx+1]

			b16 := uint16(curbyte)<<8 + uint16(nextbyte)

			// for i := idx; i < idx+w; i++ {
			// 	n += byte((b16 >> uint16(15-i) & 0x1) << byte(w-j))
			// 	j++
			// }
			n = byte((b16 >> uint(16-idx-w)) & (1<<uint(w) - 1))

			idx = idx + w - 8
			readidx++

			return n, true
		} else {
			// for i := idx; i < idx+w; i++ {
			// 	n += (curbyte >> byte(7-i) & 0x1) << byte(w-j)
			// 	j++
			// }
			n = byte((curbyte >> uint(8-idx-w)) & (1<<uint(w) - 1))

			idx += w
			if idx == 8 {
				idx = 0
				readidx++
			}

			return n, true
		}
	}

	ret := make([]byte, 0, len(buf))
	b, ok := _read(2)
	if !ok {
		return ""
	}

	if b == 3 {
		ret = append(ret, 'h', 't', 't', 'p', 's', ':', '/', '/')
	} else if b == 2 {
		ret = append(ret, 'h', 't', 't', 'p', ':', '/', '/')
	}

	var crc uint16
READ:
	for b, ok := _read(6); ok; b, ok = _read(6) {
		switch b {
		case 63:
			break READ
		case _RAW:
			b2, ok2 := _read(8)
			if !ok2 {
				break READ
			}

			ret = append(ret, b2)
		case _CAPS:
			b2, ok2 := _read(5)
			if !ok2 {
				break READ
			}
			ret = append(ret, b2+'A')
		case _HASH:
			b2, ok2 := _read(7)
			b3, ok3 := _read(8)
			if !ok3 || !ok2 {
				break READ
			}

			crc = uint16(b2)<<8 + uint16(b3)
		default:
			ret = append(ret, table[b])
		}
	}

	if (crc16(ret) & 0x7FFF) == crc {
		return string(ret)
	}

	return ""
}
