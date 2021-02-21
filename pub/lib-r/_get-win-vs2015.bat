@echo off
set THIS_DIR=%~sdp0

copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\pro_rpc.dll         %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\pro_rpc.lib         %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\pro_rpc.map         %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\pro_rpc.pdb         %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\test_rpc_client.exe %THIS_DIR%windows-vs2015\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release32\test_rpc_server.exe %THIS_DIR%windows-vs2015\x86\

copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\pro_rpc.dll         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\pro_rpc.lib         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\pro_rpc.map         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\pro_rpc.pdb         %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\test_rpc_client.exe %THIS_DIR%windows-vs2015\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2015\_release64\test_rpc_server.exe %THIS_DIR%windows-vs2015\x86_64\

pause
