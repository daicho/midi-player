using System;
using System.Threading;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using SharpDX.DirectInput;

namespace GamePad2Midi
{
    class Program
    {
        [DllImport("Winmm.dll")]
        public static extern uint midiOutOpen(out IntPtr hmo, uint uDeviceID, int dwCallback, int dwCallbackInstance, int dwFlags);

        [DllImport("Winmm.dll")]
        public static extern uint midiOutShortMsg(IntPtr hmo, int dwMsg);

        public static uint midiOutShortMsg(IntPtr hmo, byte status, byte channel, byte data1, byte data2)
        {
            return midiOutShortMsg(hmo, (status << 4) | channel | (data1 << 8) | (data2 << 16));
        }

        [DllImport("Winmm.dll")]
        public static extern uint midiOutReset(IntPtr hmo);

        [DllImport("Winmm.dll")]
        public static extern uint midiOutClose(IntPtr hmo);

        public const int DEVICE_ID = 2;
        public const int VELOCITY = 80;

        public static Dictionary<int, byte> map = new Dictionary<int, byte>(){
            { 0, 36 },
            { 1, 38 },
            { 2, 69 },
            { 3, 51 },
            { 6, 79 },
            { 7, 77 },
        };

        public static byte[] mapHat = { 51, 69, 36, 38 };

        static void Main(string[] args)
        {
            // MIDIデバイス取得
            IntPtr midiout;
            midiOutOpen(out midiout, DEVICE_ID, 0, 0, 0);

            // コントローラーデバイス取得
            DirectInput dinput = new DirectInput();
            Guid guid = Guid.Empty;

            foreach (DeviceInstance device in dinput.GetDevices(DeviceType.Gamepad, DeviceEnumerationFlags.AllDevices))
            {
                guid = device.InstanceGuid;
                break;
            }

            if (guid == Guid.Empty)
			{
                Console.WriteLine("Error");
                return;
			}

            Joystick joy = new Joystick(dinput, guid);
            joy.Properties.BufferSize = 128;
            joy.Acquire();

            bool[] buttons = new bool[16];
            bool[] hats = new bool[4];

            while (true)
            {
                // コントローラーの状態を取得
                joy.Poll();
                JoystickState state = joy.GetCurrentState();
                if (state == null)
                    break;

                // ボタンが押されていたらMIDIを再生
                foreach (var (button, note) in map)
				{
                    if (state.Buttons[button])
					{
                        if (!buttons[button])
                        {
                            midiOutShortMsg(midiout, 0x09, 1, note, VELOCITY);
                            buttons[button] = true;

                        }
                    }
                    else
					{
                        if (buttons[button])
                        {
                            midiOutShortMsg(midiout, 0x08, 1, note, VELOCITY);
                            buttons[button] = false;

                        }
                    }
                }

                // 十字キー
                int hat = state.PointOfViewControllers[0];
                bool release = (hat == -1);
                hat /= 4500;

                for (int i = 0; i < 4; i++)
                {
                    if (!release && (hat == i * 2 || (hat + 1) % 8 == i * 2 || hat == (i * 2 + 1) % 8))
					{
                        if (!hats[i])
                        {
                            midiOutShortMsg(midiout, 0x09, 1, mapHat[i], VELOCITY);
                            hats[i] = true;

                        }
                    }
                    else
                    {
                        if (hats[i])
                        {
                            midiOutShortMsg(midiout, 0x08, 1, mapHat[i], VELOCITY);
                            hats[i] = false;

                        }
                    }
                }

                Thread.Sleep(1);
            }

            midiOutReset(midiout);
            midiOutClose(midiout);
        }
    }
}
