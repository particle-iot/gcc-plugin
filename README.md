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
  -fplugin-arg-particle_plugin-target-msg-file=path/to/target-messages.json \
  -fplugin-arg-particle_plugin-predef-msg-file=path/to/predefined-messages.json \
  ...
```

The plugin supports the following arguments:
* `target-msg-file`: path to a target message file.
* `predef-msg-file`: path to a predefined message file (optional).
