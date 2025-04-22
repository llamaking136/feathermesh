# feathermesh
Attempting world domination

## Legal

I, llamaking136, am not affiliated with Meshtastic LLC or their projects. My projects, including "feathermesh"
and I are independent; however, for "feathermesh" to communicate with other mesh nodes that use Meshtastic (R)
firmware, it relies on the firmware's protocol buffers (or protobufs) which can be found [here](https://github.com/meshtastic/protobufs)
to relay communication within a mesh network.

All other code, except that in the `protobufs` directory, is written by me and under license of the GPL-v3
license.

Find more about Meshtastic LLC at https://github.com/meshtastic and https://meshtastic.org. Meshtastic (R) is a
registered trademark of Meshtastic LLC. Meshtastic software components are released under various licenses
which can be found at the links above.

## Building

`feathermesh` is intended to work on the Heltec CubeCell V2 which is has an ARM Cortex M0+ with an onboard SX1262
radio that transmits LoRa packets. If you intend to use this on another device, good luck.

To build, first generate the protobufs that you downloaded as a submodule by running

```sh
./compile_protobufs.sh
```


Second, copy `include/node_info_template.h` to `include/node_info.h` and customize the latter to your needs.


Third, make a new file `include/channel_keys.txt`. Enter your channel names and keys into this file with the
following format:

```c++
// an example channel
// channel name first, then key in base64
add_channel_b64("ChannelName", "Base64KeyGoesHere");
// LongFast
add_channel_b64("LongFast", "1PG7OiApB1nwvP+rz05pAQ==");
```

Make sure there are no spaces in the channel names.


Fourth, plug in the CubeCell of your choice, upload the code, and pray that it works.


Finally, use the serial monitor of your choice at 115200 baud and send `help` from the monitor for a list of
commands.