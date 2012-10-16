@echo off
echo Enter IP address to connect to (it should be shown in VisualDSP console):
set /p __IP=
echo Enter server's port number:
echo Some defaults are:
echo     7 for echo server
echo     9 for discard server
echo    19 for chargen server
set /p __PORT=
echo Running InetdClient...
start .\Release\InetdClient.exe %__IP% %__PORT%
cls