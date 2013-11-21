src = UDPServer.c
dep = $(src)
bin = server

$(bin): $(dep)
	gcc -Wall $(src) -o $(bin);

all: $(bin)

clean:
	rm $(bin);