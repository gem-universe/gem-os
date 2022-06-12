int _start()
{
    int a = 800;
    int b = 16;
    int VgaAddr = 0xB8000;
    char CharA = 'A';
    for (int i = 0; i < 100; i++)
    {
        short Value = 0x0f << 8 + (CharA + i);
        *(short *)VgaAddr = Value;
    }
    return a + b;
}