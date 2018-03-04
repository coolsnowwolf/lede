NAME?=goflyway
SOURCE?=./cmd/goflyway/main.go
LIST?=chinalist.txt ca.pem
VERSION:=$(shell date +%y%m%d%H%M%S)

.PHONY: release windows darwin linux clean

clean:
	$(RM) -r build

build: build/goflyway

build/goflyway:
	mkdir -p build && go build -o $@ ./cmd/goflyway && cp -t build/ $(LIST)

release: windows darwin linux

# don't use ldflags -X
release = \
	sed -i -- 's/__devel__/$(VERSION)/g' $(SOURCE) && \
	GOOS=$(1) GOARCH=$(2) go build -o build/$(3) ./cmd/goflyway && \
	sed -i -- 's/$(VERSION)/__devel__/g' $(SOURCE) && \
	cp -t build/ $(LIST)
tar = cd build && tar -cvzf $(NAME)_$(1)_$(2).tar.gz $(NAME) $(LIST) && rm $(NAME)
zip = cd build && zip $(NAME)_$(1)_$(2).zip $(NAME).exe $(LIST) && rm $(NAME).exe

linux: release/linux_386 release/linux_amd64

release/linux_386: $(SOURCE)
	$(call release,linux,386,$(NAME))
	$(call tar,linux,386)

release/linux_amd64: $(SOURCE)
	$(call release,linux,amd64,$(NAME))
	$(call tar,linux,amd64)

darwin: release/darwin_amd64

release/darwin_amd64: $(SOURCE)
	$(call release,darwin,amd64,$(NAME))
	$(call tar,darwin,amd64)

windows: release/windows_386 release/windows_amd64

release/windows_386: $(SOURCE)
	$(call release,windows,386,$(NAME).exe)
	$(call zip,windows,386)

release/windows_amd64: $(SOURCE)
	$(call release,windows,amd64,$(NAME).exe)
	$(call zip,windows,amd64)