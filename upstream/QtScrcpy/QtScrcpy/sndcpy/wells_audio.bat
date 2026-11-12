@echo off
setlocal EnableExtensions
set "ADB=adb.exe"
set "PORT=28200"
set "SERIAL="
if not "%~1"=="" set "SERIAL=-s %~1"
if not "%~2"=="" set "PORT=%~2"
if not exist "wells-audio.apk" (
  echo WELLS AUDIO: wells-audio.apk missing from package.
  exit /b 2
)
%ADB% %SERIAL% wait-for-device || exit /b 3
%ADB% %SERIAL% shell pm path com.wells.audio | findstr /C:"package:" >nul
if errorlevel 1 (
  echo Installing Wells Audio...
  %ADB% %SERIAL% install -r wells-audio.apk || exit /b 4
)
%ADB% %SERIAL% forward tcp:%PORT% localabstract:wells_audio || exit /b 5
echo.
echo WELLS AUDIO: On the phone, open Wells Audio, enable Transmitir audio,
echo and approve Android's screen/audio capture prompt.
echo Waiting for the Android audio service to connect to the Windows receiver.
echo.
%ADB% %SERIAL% shell am start -n com.wells.audio/.MainActivity || exit /b 6
exit /b 0
