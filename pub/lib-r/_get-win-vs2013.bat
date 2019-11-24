@echo off
set THIS_DIR=%~sdp0

copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\pro_rpc.dll            %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\pro_rpc.lib            %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\pro_rpc.map            %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\pro_rpc.pdb            %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\test_rpc_client.exe    %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release\test_rpc_server.exe    %THIS_DIR%windows-vs2013\x86\

copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\pro_rpc.dll         %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\pro_rpc.lib         %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\pro_rpc.map         %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\pro_rpc.pdb         %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\test_rpc_client.exe %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_release64x\test_rpc_server.exe %THIS_DIR%windows-vs2013\x86_64\

pause
