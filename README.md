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
  -fplugin-arg-particle_plugin-msg-index=path/to/current-index.json \
  -fplugin-arg-particle_plugin-msg-index-predef=path/to/predefined-index.json \
  ...
```

The plugin supports the following arguments:
* `msg-index`: path to a _current_ message index file.
* `msg-index-predef`: path to a _predefined_ message index file (optional).
