cc = gcc
src = $(wildcard *.c)
obj = $(src:.c=.o)
exe = prayer
lib = -lcurl
curl_dir = ./vendor/curl
lib_dir = -L$(curl_dir)/lib
bin_dir = $(curl_dir)/bin
inc_dir = -I$(curl_dir)/include
dll = libcurl-x64.dll

$(exe):$(obj)
	@$(cc) $^ $(lib_dir) $(lib) -o $@

$(obj):%.o:%.c
	@$(cc) $(inc_dir) -c $< -o $@

run:$(exe) $(dll)
	@./$(exe) ||:

$(dll):$(bin_dir)/$(dll)
	@cp $< $@

clean:
	@rm -rf *.o *.exe *.dll; clear