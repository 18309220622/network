.PHONY:all
all:httpd cgi
    
httpd:httpd.o main.o 
	gcc -o $@ $^ -lpthread
%.o:%.c
	gcc -c $<

.PHONY:cgi
cgi:
	cd cgi;make;make output;cd -;cd sql_connect;make

.PHONY:clean
clean:
	rm -rf *.o httpd output;cd cgi;make clean;cd -;cd sql_connect;make clean;cd -

.PHONY:output
output:all
	mkdir output
	cp httpd output/
	cp -rf log output/
	cp -rf conf output/
	cp -rf wwwroot output/
	mkdir -p output/wwwroot/cgi_bin
	cp -f cgi_math output/wwwroot/cgi_bin/
	cp -r http_ctl.sh output/
