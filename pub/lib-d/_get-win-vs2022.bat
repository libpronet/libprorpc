@echo off
set THIS_DIR=%~sdp0

copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\pro_rpc.dll         %THIS_DIR%windows-vs2022\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\pro_rpc.lib         %THIS_DIR%windows-vs2022\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\pro_rpc.map         %THIS_DIR%windows-vs2022\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\pro_rpc.pdb         %THIS_DIR%windows-vs2022\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\test_rpc_client.exe %THIS_DIR%windows-vs2022\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug32\test_rpc_server.exe %THIS_DIR%windows-vs2022\x86\

copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\pro_rpc.dll         %THIS_DIR%windows-vs2022\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\pro_rpc.lib         %THIS_DIR%windows-vs2022\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\pro_rpc.map         %THIS_DIR%windows-vs2022\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\pro_rpc.pdb         %THIS_DIR%windows-vs2022\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\test_rpc_client.exe %THIS_DIR%windows-vs2022\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2022\_debug64\test_rpc_server.exe %THIS_DIR%windows-vs2022\x86_64\

pause
