setlocal EnableDelayedExpansion
call copyToAll.bat
for /D %%A in ("*") do (
    echo Flag > colorRunning.%%A
    cd %%A
    colorize.bat
    cd ..
)

:wait
if exist colorRunning.* goto wait

echo All colorize processes have finished here