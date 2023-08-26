
@echo off

if not exist build mkdir build

set CompileFlags=/nologo /FC /Zi /std:c17 /Od /Oi /W4 /WX /wd4101 /wd4100 /wd4189
set LinkFlags=/incremental:no /opt:icf /opt:ref

pushd build
call cl %CompileFlags% /Fe:asteroid "..\code\win32_asteroid.c" /link %LinkFlags% kernel32.lib user32.lib gdi32.lib winmm.lib
popd
