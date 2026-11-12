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
echo Installing or updating Wells Audio...
rem adb shell pm path may return exit code 0 even if the package is absent.
rem ADB install -r works for both first installation and same-key updates.
%ADB% %SERIAL% install -r wells-audio.apk || exit /b 4
%ADB% %SERIAL% forward tcp:%PORT% localabstract:wells_audio || exit /b 5
echo.
echo WELLS AUDIO: Approve Android's capture prompt on the phone.
echo The Wells Audio settings screen will close after authorization.
echo Waiting for the Android audio service to connect to the Windows receiver.
echo.
%ADB% %SERIAL% shell am start -n com.wells.audio/.MainActivity --ez auto_start true || exit /b 6
exit /b 0
