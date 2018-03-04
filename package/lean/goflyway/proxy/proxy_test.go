package proxy

import (
	"bytes"
	"strconv"
	"testing"
	"time"
)

func TestCipher(t *testing.T) {
	c := &Cipher{}
	c.Init(strconv.Itoa(int(time.Now().Unix())))
	t.Log("Testing Cipher")

	test := func(m byte) {
		p := make([]byte, c.Rand.Intn(20)+16)
		for i := range p {
			if p[i] = byte(c.Rand.Intn(256)); p[i] > 200 {
				p = nil
				break
			}
		}

		auth := make([]byte, c.Rand.Intn(20)+10)
		for i := range auth {
			auth[i] = byte(c.Rand.Intn(256))
		}

		s, pp := c.NewIV(Options(m), p, string(auth))
		m2, p2, auth2 := c.ReverseIV(s)
		if byte(m2) != m ||
			(!bytes.Equal(p2, p) && !bytes.Equal(p2, pp)) ||
			(!bytes.Equal(auth, auth2) && string(auth2) != c.Alias) {
			t.Error(p, auth, m)
		}
	}

	for i := 0; i < 100; i++ {
		test(byte(c.Rand.Intn(256)))
	}
}

func BenchmarkGenWord(b *testing.B) {
	r := &Cipher{}
	r.Init("12345678")

	for i := 0; i < b.N; i++ {
		r.genWord(false)
	}
}

func BenchmarkConvert(b *testing.B) {
	a := "a"
	var buf []byte
	for i := 0; i < b.N; i++ {
		buf = *unsafeStringBytes(&a)
	}
	_ = buf[0]
}

func BenchmarkConvert2(b *testing.B) {
	a := "a"
	var buf []byte
	for i := 0; i < b.N; i++ {
		buf = []byte(a)
	}
	_ = buf[0]
}
