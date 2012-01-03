@IF NOT EXIST MIOSv10\0000000c.app @GOTO filenotfound
@cd DML
make clean
make
@cd..
@GOTO end

:filenotfound
@echo 0000000c.app not found, please read MIOSv10\info.txt

:end
@pause