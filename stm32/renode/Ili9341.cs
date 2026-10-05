using System.Collections.Generic;

using Antmicro.Renode.Backends.Display;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.SPI;

namespace Antmicro.Renode.Peripherals.Video
{
    // ILI9341 TFT controller behind a 4-wire SPI bus, as on the common 240x320 modules.
    // It decodes only the commands a simple driver needs; anything else is logged and skipped.
    // GPIO inputs: 0 = chip select (active low), 1 = data/command (low: command), 2 = reset (active low).
    [GPIO(NumberOfInputs = 3)]
    public class ILI9341 : AutoRepaintingVideo, ISPIPeripheral, IGPIOReceiver
    {
        // landscape: the module is turned a quarter turn counter-clockwise from upright portrait.
        public ILI9341(IMachine machine, bool landscape = false) : base(machine)
        {
            this.landscape = landscape;
            frameMemory = new ushort[Columns * Rows];
            parameters = new List<byte>();
            Reconfigure(landscape ? Rows : Columns, landscape ? Columns : Rows, PixelFormat.RGB565);
            Reset();
        }

        public override void Reset()
        {
            System.Array.Clear(frameMemory, 0, frameMemory.Length);
            chipSelected = false;
            dataMode = false;
            ResetRegisters();
        }

        public void OnGPIO(int number, bool value)
        {
            switch(number)
            {
            case ChipSelectPin:
                chipSelected = !value;
                // Raising chip select ends the transfer, so half a pixel is dropped.
                pixelHighByte = null;
                break;
            case DataCommandPin:
                dataMode = value;
                break;
            case ResetPin:
                if(!value)
                {
                    ResetRegisters();
                }
                break;
            }
        }

        public byte Transmit(byte data)
        {
            if(!chipSelected)
            {
                this.Log(LogLevel.Warning, "Byte 0x{0:X2} sent while chip select is high; ignored", data);
            }
            else if(!dataMode)
            {
                StartCommand((Command)data);
            }
            else if(command == Command.MemoryWrite)
            {
                WritePixelByte(data);
            }
            else
            {
                parameters.Add(data);
                ApplyParameters();
            }
            // The controller's read commands aren't modeled, so it never drives MISO.
            return 0;
        }

        public void FinishTransmission()
        {
            // Transfers end on the chip select pin instead.
        }

        protected override void Repaint()
        {
            for(var y = 0; y < Height; y++)
            {
                for(var x = 0; x < Width; x++)
                {
                    // Upright portrait shows the frame memory mirrored left to right; a quarter turn of
                    // the module then lines its rows up with the screen's columns.
                    var column = landscape ? y : Columns - 1 - x;
                    var row = landscape ? x : y;
                    var pixel = (displayOn && !sleeping) ? frameMemory[row * Columns + column] : (ushort)0;
                    if((memoryAccess & BgrOrder) == 0)
                    {
                        // The panel's subpixels are blue-green-red, so RGB order swaps red and blue.
                        pixel = (ushort)((pixel & 0x07E0) | (pixel >> 11) | ((pixel & 0x1F) << 11));
                    }
                    var offset = 2 * (y * Width + x);
                    buffer[offset] = (byte)pixel;
                    buffer[offset + 1] = (byte)(pixel >> 8);
                }
            }
        }

        private void ResetRegisters()
        {
            sleeping = true;
            displayOn = false;
            memoryAccess = 0;
            pixelFormat = 0x66; // 18 bits per pixel after reset
            columnStart = 0;
            columnEnd = Columns - 1;
            pageStart = 0;
            pageEnd = Rows - 1;
            command = Command.Nop;
            pixelHighByte = null;
        }

        private void StartCommand(Command newCommand)
        {
            command = newCommand;
            parameters.Clear();
            pixelHighByte = null;

            switch(command)
            {
            case Command.Nop:
            case Command.ColumnAddressSet:
            case Command.PageAddressSet:
            case Command.MemoryAccessControl:
            case Command.PixelFormatSet:
                break;
            case Command.SoftwareReset:
                ResetRegisters();
                break;
            case Command.SleepIn:
                sleeping = true;
                break;
            case Command.SleepOut:
                sleeping = false;
                break;
            case Command.DisplayOff:
                displayOn = false;
                break;
            case Command.DisplayOn:
                displayOn = true;
                break;
            case Command.MemoryWrite:
                cursorX = columnStart;
                cursorY = pageStart;
                break;
            default:
                this.Log(LogLevel.Warning, "Command 0x{0:X2} isn't modeled; it and its parameters are ignored", (byte)command);
                break;
            }
        }

        private void ApplyParameters()
        {
            switch(command)
            {
            case Command.ColumnAddressSet when parameters.Count == 4:
                columnStart = Word(0);
                columnEnd = Word(2);
                break;
            case Command.PageAddressSet when parameters.Count == 4:
                pageStart = Word(0);
                pageEnd = Word(2);
                break;
            case Command.MemoryAccessControl when parameters.Count == 1:
                memoryAccess = parameters[0];
                break;
            case Command.PixelFormatSet when parameters.Count == 1:
                pixelFormat = parameters[0];
                break;
            }
        }

        private int Word(int index)
        {
            return (parameters[index] << 8) | parameters[index + 1];
        }

        private void WritePixelByte(byte data)
        {
            if(pixelHighByte == null)
            {
                pixelHighByte = data;
                return;
            }
            var pixel = (ushort)((pixelHighByte.Value << 8) | data);
            pixelHighByte = null;

            // Only 16-bit pixels are modeled; a panel left at its 18-bit default would garble them.
            if((pixelFormat & 0x07) != Pixel16Bits)
            {
                this.Log(LogLevel.Warning, "Pixel written in pixel format 0x{0:X2}; only 16 bits per pixel is modeled", pixelFormat);
            }
            else
            {
                StorePixel(cursorX, cursorY, pixel);
            }

            cursorX++;
            if(cursorX > columnEnd)
            {
                cursorX = columnStart;
                cursorY = (cursorY < pageEnd) ? cursorY + 1 : pageStart;
            }
        }

        private void StorePixel(int x, int y, ushort pixel)
        {
            var exchanged = (memoryAccess & RowColumnExchange) != 0;
            var column = exchanged ? y : x;
            var row = exchanged ? x : y;
            if((memoryAccess & ColumnAddressOrder) != 0)
            {
                column = Columns - 1 - column;
            }
            if((memoryAccess & RowAddressOrder) != 0)
            {
                row = Rows - 1 - row;
            }
            if(column < 0 || column >= Columns || row < 0 || row >= Rows)
            {
                this.Log(LogLevel.Warning, "Pixel at ({0}, {1}) is outside the panel; dropped", x, y);
                return;
            }
            frameMemory[row * Columns + column] = pixel;
        }

        private bool chipSelected;
        private bool dataMode;
        private bool sleeping;
        private bool displayOn;
        private byte memoryAccess;
        private byte pixelFormat;
        private int columnStart;
        private int columnEnd;
        private int pageStart;
        private int pageEnd;
        private int cursorX;
        private int cursorY;
        private byte? pixelHighByte;
        private Command command;

        private readonly bool landscape;
        private readonly ushort[] frameMemory;
        private readonly List<byte> parameters;

        private const int Columns = 240;
        private const int Rows = 320;
        private const int ChipSelectPin = 0;
        private const int DataCommandPin = 1;
        private const int ResetPin = 2;
        private const int Pixel16Bits = 0x05;
        private const byte RowAddressOrder = 0x80;
        private const byte ColumnAddressOrder = 0x40;
        private const byte RowColumnExchange = 0x20;
        private const byte BgrOrder = 0x08;

        private enum Command : byte
        {
            Nop = 0x00,
            SoftwareReset = 0x01,
            SleepIn = 0x10,
            SleepOut = 0x11,
            DisplayOff = 0x28,
            DisplayOn = 0x29,
            ColumnAddressSet = 0x2A,
            PageAddressSet = 0x2B,
            MemoryWrite = 0x2C,
            MemoryAccessControl = 0x36,
            PixelFormatSet = 0x3A,
        }
    }
}
