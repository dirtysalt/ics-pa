all:test_elf.exe

%.exe:%.c
	gcc $< -o $@ -O0 -g

clean:
	rm test_elf.exe