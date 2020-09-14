Chrome OS (Chromebook) uses a Linux virtual machine to run Linux software. Once the virtual machine is set up you can follow the the [General Readme](../README.md) and the [Linux](Linux.md) instructions.

# Installing the Linux Beta

Go to settings find Linux (Beta) and switch it on. This will install a Debian Linux virtual machine on your system.
When finished this will bring up a terminal for your virtual machine. The terminal will also appear as an app in your app list.

You may need to do a `sudo apt update` to update the package list before following the steps in the other readme files.

# Sharing folders with the Linux virtual machine

(Useful for install IDE's etc that can not be installed from the package manager)

If you want to download anything and allow your virtual machine to access the files you will need to create a share so that 
Linux has access to the Chrome OS folder. To do this right click a folder in the Files app on Chrome OS and select share with Linux.
You can either directly share the downloads folder or create a new one to puts files destined for your virtual machine into it.

Any shared folders will be avaliable under `/mnt/chromeos/`.

Alternatively you could install a browser within the virtual machine itself.

# Flashing the firmware (DFU mode) and USB access 

By default Chrome OS prevents Linux from having access to most USB devices.
This means an extra step is required to flash the firmware, once this is done you can follow the Linux instructions for flashing firmware.

On your Chromebook in chrome visit `chrome://flags/#crostini-usb-allow-unsupported` and enable the flag. These flags are experimental features that can be turned on and off and some like this one affect what the Linux Beta can do. Enabling this flag allows you to give Linux access to any USB device.

You will need to restart Linux for this to take effect. The easiest way to do this is to restart the Chromebook. 
Shutdown from terminal does not seem to close gracefully and you lose your terminal history.

Once Linux is back up and running, performing an update should pull down some new packages thanks to the flag that has just been enabled. 

You can do this with `sudo apt update && sudo apt upgrade`

Linux should now be ready to flash the 32Blit.

When you put the 32blit into DFU mode you will get a toast popup in the bottom right hand corner saying "USB device Detected" . Clicking this and enabling the option will allow Linux instant access to the device. If you miss this then you can still give the device access from the Linux beta area in settings.

When it comes to putting the 32blit into mass storage mode it should work without any special measures as Chrome OS seems to allow storage devices access by default.

From here on in follow the [General Readme](../README.md) and the [Linux](Linux.md) instructions using the terminal app.
