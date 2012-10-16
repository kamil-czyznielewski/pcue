:: Convert all BMP file of the current folder

echo off

for %%f in (*.bmp) do (BMPconverter.exe %%f)

pause