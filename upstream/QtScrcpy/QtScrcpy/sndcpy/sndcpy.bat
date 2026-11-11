@echo off

echo Begin Runing...
set SNDCPY_PORT=28200
set SNDCPY_APK=sndcpy.apk
set ADB=adb.exe

if not "%1"=="" (
    set serial=-s %1
)
if not "%2"=="" (
    set SNDCPY_PORT=%2
)

echo Waiting for device %1...
%ADB% %serial% wait-for-device || goto :error
echo Find device %1

rem Reinstall our custom icon APK if the device has the original sndcpy
rem signature. Package ID stays com.rom1v.sndcpy; no unrelated apps touched.
set "sndcpy_installed="
for /f "delims=" %%i in ('%ADB% %serial% shell pm path com.rom1v.sndcpy 2^>nul') do set "sndcpy_installed=%%i"
if not defined sndcpy_installed (
    echo Install %SNDCPY_APK%...
    %ADB% %serial% install -t -r -g %SNDCPY_APK% || goto :error
    echo Install %SNDCPY_APK% success
) else (
    rem A different signing key cannot be updated in place; do not silently
    rem remove an existing Android app or its data without user confirmation.
    echo sndcpy already installed. If its old icon remains, uninstall
    echo com.rom1v.sndcpy manually and start Wells audio again.
)

echo Request PROJECT_MEDIA permission...
%ADB% %serial% shell appops set com.rom1v.sndcpy PROJECT_MEDIA allow

echo Forward port %SNDCPY_PORT%...
%ADB% %serial% forward tcp:%SNDCPY_PORT% localabstract:sndcpy || goto :error

echo Start %SNDCPY_APK%...
%ADB% %serial% shell am start com.rom1v.sndcpy/.MainActivity || goto :error

:check_start
set "sndcpy_started="
echo Waiting %SNDCPY_APK% start...
::timeout /T 1 /NOBREAK > nul
%ADB% %serial% shell sleep 0.1
for /f "delims=" %%i in ('%ADB% %serial% shell pidof com.rom1v.sndcpy 2^>nul') do set "sndcpy_started=%%i"
if "%sndcpy_started%"=="" (
    goto :check_start
)
echo %SNDCPY_APK% started...

echo Ready playing...
::vlc.exe -Idummy --demux rawaud --network-caching=0 --play-and-exit tcp://localhost:%SNDCPY_PORT%
::ffplay.exe -nodisp -autoexit -probesize 32 -sync ext -f s16le -ar 48k -ac 2 tcp://localhost:%SNDCPY_PORT%
goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
