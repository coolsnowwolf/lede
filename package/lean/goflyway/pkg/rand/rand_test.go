package rand

import (
	"math/rand"
	"sync"
	"testing"
)

func TestRandSingle(t *testing.T) {
	r := New()
	r2 := rand.New(rand.NewSource(r.seed))

	for i := 0; i < (1 << 23); i++ {
		if r.Uint64() != r2.Uint64() {
			t.Errorf("un-matched result in single goroutine")
		}
	}

	for i := 1; i < (1 << 35); i += (1 << 15) {
		if r.Intn(i) != r2.Intn(i) {
			t.Errorf("un-matched result in single goroutine")
		}
	}

	t.Log(r.Perm(5), r2.Perm(5))
}

func TestRandMulti(t *testing.T) {
	r := New()
	r2 := rand.New(rand.NewSource(r.seed))

	const con = 128

	res := make([]uint64, con)
	for i := 0; i < (1 << 23); i += con {
		wg := &sync.WaitGroup{}
		for j := 0; j < con; j++ {
			wg.Add(1)
			go func(j int) {
				res[j] = r.Uint64()
				wg.Done()
			}(j)
		}
		wg.Wait()
	}

	flag := false
	for i := 0; i < (1 << 23); i += con {
		wg := &sync.WaitGroup{}
		for j := 0; j < con; j++ {
			wg.Add(1)
			go func(j int) {
				// high chance we will have panic: index out of range
				defer func() {
					if recover() != nil {
						flag = true
					}

					wg.Done()
				}()

				res[j] = r2.Uint64()
			}(j)
		}
		wg.Wait()
	}

	if !flag {
		t.Error("why there was no panic?")
	}
}
