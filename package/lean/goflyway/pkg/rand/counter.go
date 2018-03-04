//+build !windows,!linux

package rand

import (
	"time"
)

func GetCounter() int64 {
	return time.Now().UnixNano()
}
