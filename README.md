# tsxset - Set date & time on Yamaha TSX-B235

This is a command line tool for setting the date & time on the Yamaha TSX-B235 desktop audio system over Bluetooth.

This tool may work with other TSX models but is untested.

The date and time on the TSX can synced with a smart phone over Bluetooth using the DTA app provided by Yamaha, however this has to be done manually and assumes the smart phone is also time synced.

I am running the tool on a Raspberry Pi with a USB Bluetooth adapter.  A cron job runs overnight and syncs the TSX in the other room.  The code should compile and run fine on Raspbian Stretch Lite but will need libbluetooth-dev installed.

## Getting Started

Download the latest release of this program and compile.  I've compiled and tested the code successfully on both Fedora 26 on a PC, and on the Raspberry Pi running Raspbian Stretch Lite.

### Prerequisites

* gcc
* libbluetooth-dev

To compile your own Pi version, copy the Makefile and source code to the Pi and run "make".

### Compiling

```
$ make
```

### Usage

If you haven't already, you'll need to pair your bluetooth adapter with the TSX.  On the Raspberry Pi over SSH this can be done as follows:

```
	$ hcitool scan
	Scanning ...
        00:1F:47:EC:0D:AB       TSX-B235 Yamaha

	$ rfcomm connect hci0 00:1F:47:EC:0D:AB
```

One paired you can run the program.  It will send the current system date & time so make sure your system is NTP synced first. 

To start the program, enter:

```
$ ./tsxset -a xx:xx:xx:xx:xx:xx

```

Replace xx:xx:xx:xx:xx:xx with the Bluetooth address of your TSX (use hciscan).

For help enter:

```
$ ./tsxset -h
./tsxset: invalid option -- 'h'
Usage:
  txset [options]
    -a xx:xx:xx:xx:xx:xx BT address
    -v Verbose

```

If successful the TSX will briefly flash up the text "BT Pairing OK" followed by "CLOCK data from App" in the display.

### Accuracy

The program sends the system date & time.  I've assumed the system is running NTP and synchronised to a suitable NTP time server over the internet.  In testing I've been using the Chrony replacement NTP daemon.

### Prerequisites

* gcc - to complile the program
* Bluetooth 

Please let me know if this works on other models and I'll update these instructions.

## Authors

* **Mark Street** [marksmanuk](https://github.com/marksmanuk)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

