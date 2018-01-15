# Particle plugin for GCC

## Building the plugin

Ensure that the following dependencies are installed:
* [GCC Plugin API](https://gcc.gnu.org/wiki/plugins) 5.3.x or later.
* [Boost](http://boost.org) 1.58.x or later.
* [RapidJSON](http://rapidjson.org) 1.1.x or later.

Invoke GNU make:
```
$ make all
```

If necessary, dependency paths can be specified via make variables:
```
$ make all \
  GCC_PLUGIN_INCLUDE_PATH=... \
  BOOST_INCLUDE_PATH=... \
  BOOST_LIB_PATH=... \
  RAPIDJSON_INCLUDE_PATH=...
```

## Using the plugin

Example:
```
$ gcc -iplugindir=path/to/plugin/directory -fplugin=particle_plugin \
  -fplugin-arg-particle_plugin-dest-msg-file=path/to/dest-messages.json \
  -fplugin-arg-particle_plugin-src-msg-file=path/to/src-messages.json \
  ...
```

The plugin supports the following arguments:
* `dest-msg-file`: path to a destination message file.
* `src-msg-file`: path to a source message file (optional).
