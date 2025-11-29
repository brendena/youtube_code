# Getting started with VHS-decode 

**constructed Nov 29 2025**

This is going to describe my basic setup to capture VHS tapes using the [vhs-decode](https://github.com/oyvindln/vhs-decode) software suit.  More specifically in this example were only going to be capturing the video using vhs-decode.  Audio in theory could be captured with many usb capture device straight from the RCA connection.



## 1 get a VHS player

When choosing a VHS player you want one that has easy access to RF signals generated from the players head.

* [List here](https://github.com/oyvindln/vhs-decode/wiki/004-The-Tap-List#sony-slv-677hf)

But if you already have a vhs player and it doesn't happen to be on the list, i would first look at similar models from the same brand to see if it has a connector that's easily accessible.

The few sony VHS players i have both have a connector with the words
* PB RF - video
* GND- Ground

Since were going to be avoiding most of the circuitry and tapping into the RF signals very close to the source, we can capture very high quality signals from a medium grade VHS player. This will probably not make a low VHS player look perfect.  I tried this with a lower end dual VHS player and DVD and the quality didn't seem to be that good when i captured it.  But the quality it produced for the tv was also very bad.


## 2 get a capture device
There are many different dvices, but i went with CXADC because it seemed liked the cheapest and most versatile capture setup, if you have a full tower pc.
* [All Available devices](https://github.com/oyvindln/vhs-decode/wiki/RF-Capture-Hardware)




#### CXADC (CX - Analogue-Digital Converter)
* [Links on where to grab one](https://github.com/happycube/cxadc-linux3?tab=readme-ov-file#where-to-find-current-pcie-1x-cx2388x-cards--notes)

### [Drivers and software for CXADC](https://github.com/happycube/cxadc-linux3)

* [Installing instructions](https://github.com/happycube/cxadc-linux3?tab=readme-ov-file#install-cxadc)

### Simple modifications
To get the most out of your CXADC it's recommended to do a few modifications.  I'm going to just list the very basic ones that don't require to much time or money
* [Remove capacitor](https://github.com/happycube/cxadc-linux3/wiki/Modifications#c31-removal)
* [Add heatsinks](https://github.com/happycube/cxadc-linux3/wiki/assets/images/Hardware/External-Clock-CX-Card/CX-White-Card-Heatsync-Jig-Horisontal-SMA-Jitterbug.jpg) 
* [Adding a BNC connector](https://github.com/happycube/cxadc-linux3/wiki/assets/images/Hardware/External-Clock-CX-Card/CX-White-Card-Heatsync-Jig-Horisontal-SMA-Jitterbug.jpg)  Not needed but most of the cables used will be BNC.
* [Adding a 40Mhz crystal](https://github.com/happycube/cxadc-linux3/wiki/Modifications#crystal-mod), if you do this you'll have to configure it in the software.  I won't be talking about this here.  But its a fairly cheap mod to remove quality.

### More involved modifications

### [External Amplifier](https://github.com/oyvindln/vhs-decode/wiki/RF-Capture-Hardware#amplifyers)

The signal the CXADC is expecting is a lot stronger then the one were providing it so the CXADC will boost the signal on the device internally.  The problem with that is it doesn't do a good job at it, so it produces extra noise.  So a simple way around that is boosting the signal before it get into it.  So then you don't need to use the internal gain. 

### [Clockgen Mod](https://github.com/happycube/cxadc-linux3/wiki/Modifications#clockgen-mod---external-clock)
You'll see a lot of information on the Clockgen Mod because it one of the current best way to both improve visual quality and sync audio and visual's together using a signal clock source.  This route requires a lot more hardware to setup.  So i'll be saving that topic for a different post.  In general before going down that road i would sudgest making sure you get the video card working and then look into the clockgen mod because it uses all the same base parts just more to improve the quality of the capture.

## 3 getting the software
I'll be using the AppImages to make setup "easier".

You'll want to make a folder in your main directory called 

* /home/$user/**decode**

Here you'll install 
* [tbc-tools_linux](https://github.com/oyvindln/vhs-decode/releases/tag/0.3.7.1)

You'll put the vhs-docde-x86_64.AppImage inside of it

You'll also need tvc-video-export's AppImage and put it into the same directory.
* [tc-export download](https://github.com/JuniorIsAJitterbug/tbc-video-export/releases)

Then to make accessing it easier in your ~/.bashrc add a link to the location of it. 
```bash
export PATH="$PATH:~/decode/tbc-tools"
```

## 4 capturing commands 

Below will talk about capturing raw data that will need to be processed before it can be visiblely seen.  

**note** if any of these commands don't work you can either run them with sudo or adjust your privildeges on your system

In your CXADC driver folder run
* ./leveladj

This will adjust the gain level on your card.

### capture

```bash
#Then you can do a simple captures
* cat /dev/cxadc0 | pv > fileName.u8
#Or you can do a lossless compressed capture
* cat /dev/cxadc0 | flac --fast -16 --sample-rate=28636 --sign=unsigned --channels=1 --endian=little --bps=8 --blocksize=65535 --lax -f - -o filename.flac
```
**Important notes**
* to stop capture use ctrl+c
* A uncompressed capture will take around 2GB a minutes.  Even the compressed format will take up a decent amount of space.

## 5 convert files to a tbc

The [TBC file format](https://github.com/oyvindln/vhs-decode/wiki/TBC-to-Analogue) is a inbetween format from raw adc values and a easily watchable format like MP4.  This format is lossless and will allow us later to easily encode it into a easier format to view.

To convert a NTSC VHS into a TBC file you'll use the below command.  
```bash 
# If you in a pal region change ntsc to pal after --system
vhs-decode-x86_64.AppImage vhs --cxadc --threads 8  --system ntsc ./filename.flac my-tape-decoded  --overwrite
```

This program will find all the frames inside the raw data and format them accordingly 

### view a tbc file
* tbc-tools-x86_64.AppImage
Then open the tbc file that's not the choma one.

In this application you'll be able to scrub through the capture and analyse it to see how well it captured the signal.


## 6 convert to MP4

Lastly if you happy with the outcome of the video you can convert the video to a mp4.
```bash
tbc-video-export.AppImage --tbc-tools-appimage tbc-tools-x86_64.AppImage my-tape-decoded.tbc --overwrite
```

You should now have a mp4 image.  Now there are more things you can do to make the image better.  Like removing interlacing and adjusting the images color range to better work with monitors, but thats out of the scope for this beginners guide to getting vhs-decode up and running.