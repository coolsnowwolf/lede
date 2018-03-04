package proxy

import (
	"github.com/coyove/goflyway/pkg/logg"
	"github.com/coyove/goflyway/pkg/msg64"
	"github.com/coyove/goflyway/pkg/rand"

	"crypto/aes"
	"crypto/cipher"
	"encoding/base64"
	"encoding/binary"
	"strings"
)

const (
	ivLen            = 16
	sslRecordLen     = 18 * 1024 // 18kb
	streamBufferSize = 512
)

var primes = []int16{
	11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
	73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
	157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
	241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
	347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
	439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
	547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641,
	643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
	751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857,
	859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971,
	977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
	1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193,
	1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301,
	1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439,
	1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549,
	1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
}

type Cipher struct {
	IO io_t

	Key       []byte
	KeyString string
	Block     cipher.Block
	Rand      *rand.ConcurrentRand
	Partial   bool
	Alias     string
}

type inplace_ctr_t struct {
	b       cipher.Block
	ctr     []byte
	out     []byte
	outUsed int
}

// From src/crypto/cipher/ctr.go
func (x *inplace_ctr_t) XorBuffer(buf []byte) {
	for i := 0; i < len(buf); i++ {
		if x.outUsed >= len(x.out)-x.b.BlockSize() {
			// refill
			remain := len(x.out) - x.outUsed
			copy(x.out, x.out[x.outUsed:])
			x.out = x.out[:cap(x.out)]
			bs := x.b.BlockSize()
			for remain <= len(x.out)-bs {
				x.b.Encrypt(x.out[remain:], x.ctr)
				remain += bs

				// Increment counter
				for i := len(x.ctr) - 1; i >= 0; i-- {
					x.ctr[i]++
					if x.ctr[i] != 0 {
						break
					}
				}
			}
			x.out = x.out[:remain]
			x.outUsed = 0
		}

		buf[i] ^= x.out[x.outUsed]
		x.outUsed++
	}
}

func dup(in []byte) (out []byte) {
	out = make([]byte, len(in))
	copy(out, in)
	return
}

func xor(blk cipher.Block, iv, buf []byte) []byte {
	iv = dup(iv)
	bsize := blk.BlockSize()
	x := make([]byte, len(buf)/bsize*bsize+bsize)

	for i := 0; i < len(x); i += bsize {
		blk.Encrypt(x[i:], iv)

		for i := len(iv) - 1; i >= 0; i-- {
			if iv[i]++; iv[i] != 0 {
				break
			}
		}
	}

	for i := 0; i < len(buf); i++ {
		buf[i] ^= x[i]
	}

	return buf
}

func (gc *Cipher) getCipherStream(key []byte) *inplace_ctr_t {
	if key == nil {
		return nil
	}

	if len(key) != ivLen {
		logg.E("iv is not 128bit long: ", key)
		return nil
	}

	return &inplace_ctr_t{
		b: gc.Block,
		// key must be duplicated because it gets modified during XorBuffer
		ctr:     dup(key),
		out:     make([]byte, 0, streamBufferSize),
		outUsed: 0,
	}
}

func (gc *Cipher) Init(key string) (err error) {
	gc.KeyString = key
	gc.Key = []byte(key)

	for len(gc.Key) < 32 {
		gc.Key = append(gc.Key, gc.Key...)
	}

	gc.Block, err = aes.NewCipher(gc.Key[:32])
	gc.Rand = rand.New(int64(binary.BigEndian.Uint64(gc.Key[:8])))
	gc.Alias = gc.genWord(false)

	return
}

func (gc *Cipher) genWord(random bool) string {
	const (
		vowels = "aeiou"
		cons   = "bcdfghlmnprst"
	)

	ret := make([]byte, 16)
	i, ln := 0, 0

	if random {
		ret[0] = (vowels + cons)[gc.Rand.Intn(18)]
		i, ln = 1, gc.Rand.Intn(6)+3
	} else {
		gc.Block.Encrypt(ret, gc.Key)
		ret[0] = (vowels + cons)[ret[0]/15]
		i, ln = 1, int(ret[15]/85)+6
	}

	link := func(prev string, this string, thisidx byte) {
		if strings.ContainsRune(prev, rune(ret[i-1])) {
			if random {
				ret[i] = this[gc.Rand.Intn(len(this))]
			} else {
				ret[i] = this[ret[i]/thisidx]
			}

			i++
		}
	}

	for i < ln {
		link(vowels, cons, 20)
		link(cons, vowels, 52)
		link(vowels, cons, 20)
		link(cons, vowels+"tr", 37)
	}

	if !random {
		ret[0] -= 32
	}

	return string(ret[:ln])
}

func (gc *Cipher) genIV(ss ...byte) []byte {
	if len(ss) == ivLen {
		return ss
	}

	ret := make([]byte, ivLen)

	var mul uint32 = 1
	for _, s := range ss {
		mul *= uint32(primes[s])
	}

	var seed uint32 = binary.LittleEndian.Uint32(gc.Key[:4])

	for i := 0; i < ivLen/4; i++ {
		seed = (mul * seed) % 0x7fffffff
		binary.LittleEndian.PutUint32(ret[i*4:], seed)
	}

	return ret
}

func (gc *Cipher) Encrypt(buf []byte, ss ...byte) []byte {
	if ss == nil || len(ss) < 2 {
		b, b2 := byte(gc.Rand.Intn(256)), byte(gc.Rand.Intn(256))
		return append(xor(gc.Block, gc.genIV(b, b2), buf), b, b2)
	}

	return xor(gc.Block, gc.genIV(ss...), buf)
}

func (gc *Cipher) Decrypt(buf []byte, ss ...byte) []byte {
	if buf == nil || len(buf) == 0 {
		return []byte{}
	}

	if ss == nil || len(ss) < 2 {
		if len(buf) < 2 {
			return []byte{}
		}

		b, b2 := byte(buf[len(buf)-2]), byte(buf[len(buf)-1])
		return xor(gc.Block, gc.genIV(b, b2), buf[:len(buf)-2])
	}

	return xor(gc.Block, gc.genIV(ss...), buf)
}

func (gc *Cipher) EncryptString(text string, rkey ...byte) string {
	return base32Encode(gc.Encrypt([]byte(text), rkey...), true)
}

func (gc *Cipher) DecryptString(text string, rkey ...byte) string {
	buf, _ := base32Decode(text, true)
	return string(gc.Decrypt(buf, rkey...))
}

func (gc *Cipher) EncryptCompress(str string, rkey ...byte) string {
	return base32Encode(gc.Encrypt(msg64.Compress(str), rkey...), false)
}

func (gc *Cipher) DecryptDecompress(str string, rkey ...byte) string {
	buf, _ := base32Decode(str, false)
	return msg64.Decompress(gc.Decrypt(buf, rkey...))
}

func checksum1b(buf []byte) byte {
	s := int16(1)
	for _, b := range buf {
		s *= primes[b]
	}
	return byte(s>>12) + byte(s&0x00f0)
}

func (gc *Cipher) NewIV(o Options, payload []byte, auth string) (string, []byte) {
	ln := ivLen

	// +------------+-------------+-----------+-- -  -   -
	// | Options 1b | checksum 1b | iv 128bit | auth data ...
	// +------------+-------------+-----------+-- -  -   -

	var retB, ret []byte

	if auth == "" {
		auth = gc.Alias
	}
	if !o.IsSet(doDNS) {
		if payload == nil {
			ret = make([]byte, ln)
			retB = make([]byte, 1+1+ln+len(auth))

			for i := 2; i < ln+2; i++ {
				retB[i] = byte(gc.Rand.Intn(255) + 1)
				ret[i-2] = retB[i]
			}
		} else {
			ret = payload
			retB = make([]byte, 1+1+ln+len(auth))
			copy(retB[2:], payload)
		}
	} else {
		// +-------+-------------+------------+------+-- -  -   -
		// | doDNS | checksum 1b | hostlen 1b | host | auth data ...
		// +-------+-------------+------------+------+-- -  -   -
		retB = make([]byte, 1+1+1+len(payload)+len(auth))
		if len(payload) > 255 {
			logg.W("loss of data: ", string(payload))
		}

		retB[2] = byte(len(payload))
		copy(retB[3:], payload)
		ln = len(payload) + 1
	}

	copy(retB[2+ln:], auth)

	retB[0], retB[1] = o.Val(), checksum1b(retB[2:])
	s1, s2, s3, s4 := byte(gc.Rand.Intn(256)), byte(gc.Rand.Intn(256)),
		byte(gc.Rand.Intn(256)), byte(gc.Rand.Intn(256))

	return base64.StdEncoding.EncodeToString(
		append(
			xor(
				gc.Block, gc.genIV(s1, s2, s3, s4), retB,
			), s1, s2, s3, s4,
		),
	), ret
}

func (gc *Cipher) ReverseIV(key string) (o Options, iv []byte, auth []byte) {
	o = Options(0xff)
	if key == "" {
		return
	}

	buf, err := base64.StdEncoding.DecodeString(key)
	if err != nil || len(buf) < 5 {
		return
	}

	b, b2, b3, b4 := buf[len(buf)-4], buf[len(buf)-3], buf[len(buf)-2], buf[len(buf)-1]
	buf = xor(gc.Block, gc.genIV(b, b2, b3, b4), buf[:len(buf)-4])

	if len(buf) < 3 {
		return
	}

	if buf[1] != checksum1b(buf[2:]) {
		return
	}

	if (buf[0] & doDNS) == 0 {
		if len(buf) < ivLen+2 {
			return
		}

		return Options(buf[0]), buf[2 : 2+ivLen], buf[2+ivLen:]
	}

	ln := buf[2]
	if 3+int(ln) > len(buf) {
		return
	}

	return Options(buf[0]), buf[3 : 3+ln], buf[3+ln:]
}
