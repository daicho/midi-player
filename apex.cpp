#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define midi_msg(channel, status, data1, data2) ((DWORD)(channel | (status << 4) | (data1 << 8) | (data2 << 16)))

#define INPUT_DEVICE 0
#define OUTPUT_DEVICE 0

HMIDIOUT hmidiout;

void CALLBACK MidiInProc(HMIDIIN midi_in_handle, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    unsigned char channel;
    unsigned char status;
    unsigned char note;
    unsigned char velocity;
    INPUT in;

    switch (wMsg) {
        case MIM_OPEN:
            printf("MIDI device was opened\n");
            break;

        case MIM_CLOSE:
            printf("MIDI device was closed\n");
            break;

        case MIM_DATA:
            channel = dwParam1 & 0x0000000f;
            status = (dwParam1 & 0x000000f0) >> 4;
            note = (dwParam1 & 0x0000ff00) >> 8;
            velocity = (dwParam1 & 0x00ff0000) >> 16;

            printf("channel = %d, status = 0x%04x, note = %u, velocity = %u\n", channel, status, note, velocity);
            midiOutShortMsg(hmidiout, midi_msg(channel, status, note, 100));

            if (status == 0x08 || status == 0x09) {
                BYTE code;

                switch (note) {
                    case 65: code = 'Z'; break;
                    case 66: code = VK_ESCAPE; break;
                    case 67: code = VK_LSHIFT; break;
                    case 68: code = 'Q'; break;
                    case 69: code = 'A'; break;
                    case 70: code = 'W'; break;
                    case 71: code = 'S'; break;
                    case 72: code = 'D'; break;
                    case 73: code = 'R'; break;
                    case 74: code = 'E'; break;
                    case 75: code = '3'; break;
                    case 76: code = VK_SPACE; break;
                    case 77: code = 'G'; break;
                    default: return; break;
                }

                in.type = INPUT_KEYBOARD;
                in.ki.wVk = code;
                in.ki.wScan = (short)MapVirtualKey(code, 0);
                in.ki.dwFlags = (status == 0x08) ? KEYEVENTF_KEYUP : 0;
                in.ki.dwExtraInfo = 0;
                in.ki.time = 0;

                SendInput(1, &in, sizeof(INPUT));
            }

            break;

        case MIM_LONGDATA:
        case MIM_ERROR:
        case MIM_LONGERROR:
        case MIM_MOREDATA:
        default:
            break;
    }
}

int main() {
    HMIDIIN hmidiin;
    MMRESULT res;

    // 入力デバイスオープン
    res = midiInOpen(&hmidiin, INPUT_DEVICE, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);

    if (res == MMSYSERR_NOERROR) {
        printf("Successfully opened a MIDI input device %d\n", INPUT_DEVICE);
    } else {
        printf("Cannot open MIDI input device %d\n", INPUT_DEVICE);
        return -1;
    }

    // 出力デバイスオープン
    res = midiOutOpen(&hmidiout, OUTPUT_DEVICE, NULL, 0, CALLBACK_NULL);

    if (res == MMSYSERR_NOERROR) {
        printf("Successfully opened a MIDI output device %d\n", OUTPUT_DEVICE);
    } else {
        printf("Cannot open MIDI output device %d\n", OUTPUT_DEVICE);
        return -1;
    }

    midiInStart(hmidiin);

    // qで終了
    while (true) {
        if (getchar() == 'q')
            break;
    }

    // 解放
    midiInStop(hmidiin);
    midiInReset(hmidiin);
    midiInClose(hmidiin);

    midiOutReset(hmidiout);
    midiOutClose(hmidiout);

    return 0;
}
