package logg

import (
	"bytes"
	"fmt"
	"net"
	"os"
	"runtime"
	"strings"
	"syscall"
	"time"
)

const (
	LvDebug = iota - 1
	LvLog
	LvWarning
	LvError
	LvOff
	LvPrint = 99
)

var (
	logLevel     = 0
	fatalAsError = false
	started      = false
	logFileOnly  = false
	logFile      *os.File
	logCallback  func(ts int64, msg string)
)

func SetLevel(lv string) int {
	switch lv {
	case "dbg":
		logLevel = -1
	case "log":
		logLevel = 0
	case "warn":
		logLevel = 1
	case "err":
		logLevel = 2
	case "off":
		logLevel = 3
	case "pp":
		logLevel = 99
	default:
		panic("unexpected log level: " + lv)
	}

	return logLevel
}

func GetLevel() int {
	return logLevel
}

func Redirect(dst interface{}) {
	switch dst.(type) {
	case func(int64, string):
		logCallback = dst.(func(int64, string))
	case string:
		fn := dst.(string)
		if fn[0] == '*' {
			fn = fn[1:]
			logFileOnly = false
		} else {
			logFileOnly = true
		}

		logFile, _ = os.Create(fn)
	}
}

func TreatFatalAsError(flag bool) {
	fatalAsError = flag
}

func timestamp() string {
	t := time.Now()
	mil := t.UnixNano() % 1e9
	mil /= 1e6

	return fmt.Sprintf("%02d%02d/%02d%02d%02d.%03d", t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second(), mil)
}

func trunc(fn string) string {
	idx := strings.LastIndex(fn, "/")
	if idx == -1 {
		idx = strings.LastIndex(fn, "\\")
	}
	return fn[idx+1:]
}

// Widnows WSA error messages are way too long to print
// ex: An established connection was aborted by the software in your host machine.write tcp 127.0.0.1:8100->127.0.0.1:52466: wsasend: An established connection was aborted by the software in your host machine.
func tryShortenWSAError(err interface{}) (ret string) {
	defer func() {
		if recover() != nil {
			ret = fmt.Sprintf("%v", err)
		}
	}()

	if e, sysok := err.(*net.OpError).Err.(*os.SyscallError); sysok {
		errno := e.Err.(syscall.Errno)
		if msg, ok := WSAErrno[int(errno)]; ok {
			ret = msg
		} else {
			// messages on linux are short enough
			ret = fmt.Sprintf("C%d, %s", uintptr(errno), e.Error())
		}

		return
	}

	ret = err.(*net.OpError).Err.Error()
	return
}

type msg_t struct {
	dst     string
	lead    string
	ts      int64
	message string
}

var msgQueue = make(chan *msg_t)

func print(l string, params ...interface{}) {
	if !started {
		return
	}

	_, fn, line, _ := runtime.Caller(2)
	m := msg_t{lead: fmt.Sprintf("[%s%s:%s(%d)] ", l, timestamp(), trunc(fn), line), ts: time.Now().UnixNano()}

	for _, p := range params {
		switch p.(type) {
		case *net.OpError:
			op := p.(*net.OpError)

			if op.Source == nil && op.Addr == nil {
				m.message += fmt.Sprintf("%s, %s", op.Op, tryShortenWSAError(p))
			} else {
				m.message += fmt.Sprintf("%s %v, %s", op.Op, op.Addr, tryShortenWSAError(p))

				if op.Source != nil && op.Addr != nil {
					m.dst, _, _ = net.SplitHostPort(op.Addr.String())
				}
			}
		case *net.DNSError:
			op := p.(*net.DNSError)

			if m.message += fmt.Sprintf("DNS lookup err"); op.IsTimeout {
				m.message += ", timed out"
			}

			if op.Name == "" {
				m.message += ": " + op.Name
			} else {
				m.message += ", but with an empty name (?)"
			}
		default:
			m.message += fmt.Sprintf("%+v", p)
		}
	}

	if l == "X" {
		// immediately print fatal message
		printRaw(0, m.lead+m.message, nil)
		return
	}

	msgQueue <- &m
}

func printRaw(ts int64, str string, buf *bytes.Buffer) {
	if logFile != nil {
		if buf != nil {
			buf.WriteString(str)
			buf.WriteString("\n")

			if buf.Len() > 16*1024 {
				logFile.Write(buf.Bytes())
				buf.Reset()
			}
		} else {
			logFile.Write([]byte(str))
		}
	}

	if logCallback != nil {
		logCallback(ts, str)
	} else if !logFileOnly {
		fmt.Println(str)
	}
}

func printLoop() {
	var count, nop int
	var lastMsg *msg_t
	var lastTime = time.Now()
	logBuffer := &bytes.Buffer{}

	print := func(m *msg_t) {
		if lastMsg != nil && m != nil {
			// this message is similar to the last one
			if (m.dst != "" && m.dst == lastMsg.dst) || m.message == lastMsg.message {
				if time.Now().Sub(lastTime).Seconds() < 5.0 {
					count++
					return
				}

				// though similar, 5s timeframe is over, we should print this message anyway
			}
		}

		if count > 0 {
			printRaw(lastMsg.ts, fmt.Sprintf(strings.Repeat(" ", len(lastMsg.lead))+"... %d similar message(s)", count), logBuffer)
		}

		if lastMsg == nil && m == nil {
			return
		}

		if m != nil {
			printRaw(m.ts, m.lead+m.message, logBuffer)
			lastMsg = m
		}

		lastTime, count, nop = time.Now(), 0, 0
	}

	for {
	L:
		for {
			select {
			case m := <-msgQueue:
				print(m)
			default:
				if nop++; nop > 10 {
					print(nil)
				}
				// nothing in queue to print, quit loop
				break L
			}
		}

		time.Sleep(100 * time.Millisecond)
	}
}

func Start() {
	if started {
		return
	}

	started = true
	go printLoop()
}

func D(params ...interface{}) {
	if logLevel <= -1 {
		print("_", params...)
	}
}

func L(params ...interface{}) {
	if logLevel <= 0 {
		print("_", params...)
	}
}

func W(params ...interface{}) {
	if logLevel <= 1 {
		print("W", params...)
	}
}

func E(params ...interface{}) {
	if logLevel <= 2 {
		print("E", params...)
	}
}

func P(params ...interface{}) {
	if logLevel == 99 {
		print("P", params...)
	}
}

func F(params ...interface{}) {
	print("X", params...)

	if !fatalAsError {
		os.Exit(1)
	}
}
