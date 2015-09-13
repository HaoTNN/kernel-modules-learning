obj-m := hello-world.o
obj-m := get-sys-call-table.o
obj-m := hijack-open.o
obj-m := hijack-close.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
