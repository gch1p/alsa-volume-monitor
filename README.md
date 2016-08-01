# alsa-volume-monitor

`alsa-volume-monitor` is a simple program written in C that listens to ALSA events and emits a DBus signal when something has been changed (e.g. volume).

It was created for use with Awesome WM in volume indicator widgets.

### Usage
`./alsa-volume-monitor hw:0`, where `hw:0` is the name of your audio card.

### Catch events from Awesome config

```
dbus.request_name("session", "com.ch1p.avm")
dbus.add_match("session", "interface='com.ch1p.avm',member='valueChanged'")
dbus.connect_signal("com.ch1p.avm", function(...)
        -- Update your widget here
    end
)
```

### Compiling on Ubuntu

To build the app on Ubuntu, you need to install `libasound2-dev` and `libdbus-1-dev`.

### License
GPLv2
