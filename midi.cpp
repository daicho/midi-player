#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define INPUT_DEVICE_ID 0

void CALLBACK MidiInProc(HMIDIIN midi_in_handle, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    unsigned char channel;
    unsigned char status;
    unsigned char note;
    unsigned char velocity;
    char cmd[256];

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

            if (status == 0x0009) {
                sprintf(cmd, "play wav/%d.wav", (int)note);
            } else if (status == 0x0008) {
                sprintf(cmd, "stop wav/%d.wav", (int)note);
            }

            mciSendString(cmd, NULL, 0, NULL);
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
    char cmd[256];

    // 入力デバイスオープン
    res = midiInOpen(&hmidiin, INPUT_DEVICE_ID, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);

    if (res == MMSYSERR_NOERROR) {
        printf("Successfully opened a MIDI input device %d\n", INPUT_DEVICE_ID);
    } else {
        printf("Cannot open MIDI input device %d\n", INPUT_DEVICE_ID);
        return -1;
    }

    // 音源オープン
    // for (int i = 0; i < 128; i++) {
    //     sprintf(cmd, "open wav/%d.wav alias piano%d", i, i);
    //     mciSendString(cmd, NULL, 0, NULL);
    // }

    // printf("Sound files were opened.\n");

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

    // // 音源クローズ
    // for (int i = 0; i < 128; i++) {
    //     sprintf(cmd, "close piano%d", i);
    //     mciSendString(cmd, NULL, 0, NULL);
    // }

    return 0;
}
