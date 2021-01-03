@echo off
set THIS_DIR=%~sdp0

copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\pro_rpc.dll            %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\pro_rpc.lib            %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\pro_rpc.map            %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\pro_rpc.pdb            %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\test_rpc_client.exe    %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug\test_rpc_server.exe    %THIS_DIR%windows-vs2015\x86\

copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\pro_rpc.dll         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\pro_rpc.lib         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\pro_rpc.map         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\pro_rpc.pdb         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\test_rpc_client.exe %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_debug64x\test_rpc_server.exe %THIS_DIR%windows-vs2015\x86_64\

pause
