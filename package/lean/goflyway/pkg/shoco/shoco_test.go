package shoco

import (
	"math/rand"
	"testing"
	"time"
)

func randomString() string {
	ret := ""
	_rand := rand.New(rand.NewSource(time.Now().UnixNano()))
	ln := _rand.Intn(64) + 64

	for i := 0; i < ln; i++ {
		ret += string(32 + byte(_rand.Intn(90)))
	}

	return ret
}

func randomBytes() []byte {
	_rand := rand.New(rand.NewSource(time.Now().UnixNano()))
	ln := _rand.Intn(64) + 64
	ret := make([]byte, ln)

	for i := 0; i < ln; i++ {
		ret[i] = byte(_rand.Intn(256))
	}

	return ret
}

func TestShoco(t *testing.T) {
	t.Log("Testing shoco compressing and decompressing")

	for i := 0; i < 65536; i++ {
		v := randomString()
		if v != Decompress(Compress(v)) {
			t.Error("Shoco failed", v)
		}
	}
}

func TestShocoRandomDecompression(t *testing.T) {
	t.Log("Testing shoco random decompressing")

	for i := 0; i < 65536; i++ {
		v := randomBytes()
		Decompress(v)
	}
}
