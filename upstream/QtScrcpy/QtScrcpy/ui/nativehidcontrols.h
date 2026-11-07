#pragma once
// Mensagens UHID nativas do scrcpy, compartilhando o canal de controle QtScrcpy.
// Formatos definidos pelo scrcpy oficial (Genymobile/scrcpy, Apache-2.0).
// Nenhum processo ou janela de espelhamento adicional é necessário.
#include <QByteArray>
#include <QtGlobal>

#ifdef Q_OS_WIN
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# include <Xinput.h>
# ifdef _MSC_VER
#  pragma comment(lib, "Xinput9_1_0.lib")
# endif
#endif

namespace NativeHid {
static const quint16 KeyboardId = 1; // id do teclado do scrcpy
static const quint16 GamepadId = 0x4740;
static const unsigned char KeyboardDescriptor[] = {
    0x05,0x01,0x09,0x06,0xA1,0x01,
    0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,
    0x75,0x01,0x95,0x08,0x81,0x02,
    0x75,0x08,0x95,0x01,0x81,0x01,
    0x05,0x08,0x19,0x01,0x29,0x05,0x75,0x01,0x95,0x05,0x91,0x02,
    0x75,0x03,0x95,0x01,0x91,0x01,
    0x05,0x07,0x19,0x00,0x29,0x65,0x15,0x00,0x25,0x65,
    0x75,0x08,0x95,0x06,0x81,0x00,0xC0
};
static const unsigned char GamepadDescriptor[] = {
    0x05,0x01,0x09,0x05,0xA1,0x01,0xA1,0x00,
    0x05,0x01,0x09,0x30,0x09,0x31,0x09,0x33,0x09,0x34,
    0x15,0x00,0x27,0xFF,0xFF,0x00,0x00,
    0x75,0x10,0x95,0x04,0x81,0x02,
    0x05,0x01,0x09,0x32,0x09,0x35,0x15,0x00,0x26,0xFF,0x7F,
    0x75,0x10,0x95,0x02,0x81,0x02,
    0x05,0x09,0x19,0x01,0x29,0x10,0x15,0x00,0x25,0x01,
    0x95,0x10,0x75,0x01,0x81,0x02,
    0x05,0x01,0x09,0x39,0x15,0x01,0x25,0x08,
    0x75,0x04,0x95,0x01,0x81,0x42,
    0xC0,0xC0
};
inline void appendBe16(QByteArray &bytes, quint16 value) {
    bytes.append(char(value >> 8));
    bytes.append(char(value & 0xFF));
}
inline void appendLe16(QByteArray &bytes, quint16 value) {
    bytes.append(char(value & 0xFF));
    bytes.append(char(value >> 8));
}
inline QByteArray create(quint16 id, quint16 vendor, quint16 product,
                         const QByteArray &name, const unsigned char *descriptor, int length) {
    QByteArray message;
    message.append(char(12)); // UHID_CREATE
    appendBe16(message,id);
    appendBe16(message,vendor);
    appendBe16(message,product);
    const QByteArray safeName = name.left(127);
    message.append(char(safeName.size()));
    message.append(safeName);
    appendBe16(message,quint16(length));
    message.append(reinterpret_cast<const char *>(descriptor),length);
    return message;
}
inline QByteArray createGamepad() {
    return create(GamepadId,0x045E,0x028E,"Microsoft X-Box 360 Pad",
                  GamepadDescriptor,int(sizeof(GamepadDescriptor)));
}
inline QByteArray createKeyboard() {
    return create(KeyboardId,0x18D1,0x4EE7,"QtScrcpy Keyboard",
                  KeyboardDescriptor,int(sizeof(KeyboardDescriptor)));
}
inline QByteArray destroy(quint16 id) {
    QByteArray message;
    message.append(char(14)); // UHID_DESTROY
    appendBe16(message,id);
    return message;
}
inline QByteArray input(quint16 id,const QByteArray &report) {
    QByteArray message;
    message.append(char(13)); // UHID_INPUT
    appendBe16(message,id);
    appendBe16(message,quint16(report.size()));
    message.append(report);
    return message;
}
#ifdef Q_OS_WIN
inline bool readGamepad(XINPUT_STATE &state) {
    ZeroMemory(&state,sizeof(state));
    return XInputGetState(0,&state)==ERROR_SUCCESS;
}
inline QByteArray gamepadReport(const XINPUT_GAMEPAD &state) {
    QByteArray report;
    report.reserve(15);
    appendLe16(report,quint16(int(state.sThumbLX)+32768));
    appendLe16(report,quint16(32767-int(state.sThumbLY)));
    appendLe16(report,quint16(int(state.sThumbRX)+32768));
    appendLe16(report,quint16(32767-int(state.sThumbRY)));
    appendLe16(report,quint16(quint32(state.bLeftTrigger)*32767/255));
    appendLe16(report,quint16(quint32(state.bRightTrigger)*32767/255));
    quint16 buttons=0;
    if(state.wButtons & XINPUT_GAMEPAD_A)buttons|=0x0001;
    if(state.wButtons & XINPUT_GAMEPAD_B)buttons|=0x0002;
    if(state.wButtons & XINPUT_GAMEPAD_X)buttons|=0x0008;
    if(state.wButtons & XINPUT_GAMEPAD_Y)buttons|=0x0010;
    if(state.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)buttons|=0x0040;
    if(state.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)buttons|=0x0080;
    if(state.wButtons & XINPUT_GAMEPAD_BACK)buttons|=0x0400;
    if(state.wButtons & XINPUT_GAMEPAD_START)buttons|=0x0800;
    if(state.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)buttons|=0x2000;
    if(state.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)buttons|=0x4000;
    appendLe16(report,buttons);
    const bool up=state.wButtons & XINPUT_GAMEPAD_DPAD_UP;
    const bool down=state.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
    const bool left=state.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
    const bool right=state.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
    const quint8 hat=up?(left?8:right?2:1)
                     :down?(left?6:right?4:5)
                     :left?7:right?3:0;
    report.append(char(hat));
    return report;
}
#endif
} // namespace NativeHid
