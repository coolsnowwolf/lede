
CC = gcc
CFLAGS = -I. -fPIC -O -g -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -fstrict-aliasing

COMMON_PROG_OBJS = \
	call_qcsapi.o	\
	qcsapi_driver.o	\
	qcsapi_output.o	\
	qcsapi_sem.o	\
	qcsapi_util.o

SOCKET_PROG_OBJS = \
	$(COMMON_PROG_OBJS)				\
	qcsapi_rpc/client/socket/qcsapi_socket_rpc_client.o	\
	qcsapi_rpc_common/client/find_host_addr.o	\

SOCKET_C_SAMPLE_OBJS = \
	qcsapi_rpc_sample/c_rpc_qcsapi_sample.o	\
	qcsapi_rpc_common/client/find_host_addr.o	\

PCIE_PROG_OBJS = \
	$(COMMON_PROG_OBJS)				\
	qcsapi_rpc/client/pcie/qcsapi_pcie_rpc_client.o	\
	qcsapi_rpc_common/client/rpc_pci_clnt.o	\

SOCKET_RAW_PROG_OBJS = \
	$(COMMON_PROG_OBJS)				\
	qcsapi_rpc/client/socket_raw/qcsapi_socketraw_rpc_client.o	\
	qcsapi_rpc_common/client/rpc_raw_clnt.o	\
	qcsapi_rpc_common/common/rpc_raw.o

LIB_OBJS = \
	qcsapi_rpc/generated/qcsapi_rpc_xdr.o	\
	qcsapi_rpc/generated/qcsapi_rpc_clnt_adapter.o	\

TARGETS = c_rpc_qcsapi_sample \
	qcsapi_sockrpc \
	qcsapi_sockrpc_static \
	qcsapi_pcie \
	qcsapi_pcie_static \
	qcsapi_sockraw \
	qcsapi_sockraw_static \
	$(LIB_REALNAME)

CFLAGS += -DPCIE_RPC_TYPE=RPC_TYPE_QCSAPI_PCIE

all: $(TARGETS)

-include $(shell find . -name \*.d)

LIB_NAME = qcsapi_client
LIB_LDNAME = lib$(LIB_NAME).so
LIB_SONAME = $(LIB_LDNAME).1
LIB_REALNAME = $(LIB_LDNAME).1.0.1

c_rpc_qcsapi_sample: ${SOCKET_C_SAMPLE_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_pcie: ${PCIE_PROG_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_pcie_static: ${PCIE_PROG_OBJS:%=build/%} ${LIB_OBJS}
	${CC} $(filter %.o, $^) -o $@

qcsapi_sockrpc: ${SOCKET_PROG_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_sockrpc_static: ${SOCKET_PROG_OBJS:%=build/%} ${LIB_OBJS}
	${CC} $(filter %.o, $^) -o $@

qcsapi_sockraw: ${SOCKET_RAW_PROG_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_sockraw_static: ${SOCKET_RAW_PROG_OBJS:%=build/%} ${LIB_OBJS}
	${CC} $(filter %.o, $^) -o $@

$(LIB_REALNAME): ${LIB_OBJS:%=build/%}
	${CC} -shared -s -o $@ -Wl,-soname,$(LIB_SONAME) -lc $^
	cd ${@D} ; ln -fs $(LIB_REALNAME) $(LIB_SONAME)
	cd ${@D} ; ln -fs $(LIB_SONAME) $(LIB_LDNAME)

build/%.o: %.c
	@mkdir -p ${@D}
	${CC} ${CFLAGS} $< -c -o $@ -MD -MF $@.d

clean:
	rm -rf build $(LIB_LDNAME)* $(TARGETS) $(LIB_OBJS)

