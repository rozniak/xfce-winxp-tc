### Installing the Windows XP start button

Run the start button installer.
` sudo start-button.sh `

If you are missing dependencies, install them and run the command again.
The start button installer script requires `fakeroot`, `cmake`, `libgarcon-gtk3-1-dev`, and `libxfce4panel-2.0-4`.

The script should compile the start button source and copy over all the project's related files to where they need to go for XFCE to recognize the plugin.

If you have any questions or problems please open an issue.
