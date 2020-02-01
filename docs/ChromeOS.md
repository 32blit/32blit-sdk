Chrome OS (Chromebook) uses a Linux virtual machine to run Linux software. Once the virtual machine is set up you can follow the the root README.md and the * [Linux](docs/Linux.md) instructions

# Installing the Linux Beta

Go to settings find Linux (Beta) and switch it on. This will install a Debian Linux virtual machine on your system.
When finished this will bring up a terminal for your virtual machine. The terminal will also appear as an app in your app list.

# Sharing folders with the Linux virtual machine

(Useful for install IDE's etc that can not be installed from the package manager)

If you want to download anything and allow your virtual machine to access the files you will need to create a share so that 
Linux has access to the Chrome OS folder. To do this right click a folder in the Files app on Chrome OS and select share with Linux.
You can either directly share the downloads folder or create a new one to puts files destined for your virtual machine into it.

Any shared folders will be avaliable under /mnt/chromeos/.

Alternatively you could install a browser in the virtual machine itself.

From here on in follow the general docs and the Linux ones using the terminal app.
