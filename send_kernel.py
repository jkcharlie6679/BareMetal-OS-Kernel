from serial import Serial    # pyserial
import argparse
import time

parser = argparse.ArgumentParser(description='NYCU OSDI kernel sender')
parser.add_argument('--filename', metavar='PATH', default='kernel8.img', type=str, help='path to kernel8.img')
parser.add_argument('--device', metavar='TTY',default='/dev/tty.usbserial-0001', type=str,  help='path to UART device')
parser.add_argument('--baud', metavar='Hz',default=115200, type=int,  help='baud rate')
args = parser.parse_args()

with open(args.filename,'rb') as fd:
  kernel_raw = fd.read()
  length = len(kernel_raw)
  print("Kernel image size : ", hex(length))
  with Serial(args.device, args.baud) as ser:
    for i in range(4):
      ser.write(bytes(hex(length)[5-i:5-i+1], 'ascii'))
      ser.flush()
    print("Start sending kernel img by uart...")
    for i in range(length):
      ser.write(kernel_raw[i: i+1])
      ser.flush()
    print("{:>6}/{:>6} bytes".format(length, length))
    while(1):
      print(ser.readline())
    # time.sleep(2)
    print("Transfer finished!")
