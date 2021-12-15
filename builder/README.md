# Builder Tool Command Reference

```shell
$ build.py [-h] {build,clean,configure,imports} ...

optional arguments:
  -h, --help            show this help message and exit

commands:
  {build,clean,configure,imports}
    build               builds auto sdk components (default)
    clean               cleans builder cache
    configure           builder configuration
    imports             manages external search paths    
```

## Build command

```shell
$ build.py build [-h] [--home PATH] [-v] [-p PLATFORM] [-a ARCH] [-g]
                 [-m MODULE [MODULE ...]] [-n NAME] [-y]
                 [-f PACKAGE [PACKAGE ...]] [-i PATH [PATH ...]]
                 [-o OPTION] [-s SETTING] [--with-aasb] [--no-aasb]
                 [--with-docs] [--no-docs] [--with-unit-tests]
                 [--no-unit-tests] [--with-sampleapp] [--no-sampleapp]
                 [--with-sensitive-logs] [--no-sensitive-logs]
                 [--with-latency-logs] [--no-latency-logs]
                 [--output FILE] [--no-output] [--skip-config]
```
Used to build Auto SDK modules and components.

```shell
optional arguments:
  -h, --help            show this help message and exit
  --home PATH, --builder-home PATH
                        override builder home path
  -v, --verbose         enable verbose logging
  -p PLATFORM, --platform PLATFORM
                        target platform - android,qnx,etc.
  -a ARCH, --arch ARCH  target architecture
  -g, --debug           specify debug build type
  -m MODULE [MODULE ...], --modules MODULE [MODULE ...]
                        list of modules to build
  -n NAME, --name NAME  optional package identifier
  -y, --accept-licenses
                        auto-accept licenses
  -f PACKAGE [PACKAGE ...], --force PACKAGE [PACKAGE ...]
                        force export and build package
  -i PATH [PATH ...], --include PATH [PATH ...]
                        add include path to conan configuration
  -o OPTION, --conan-option OPTION
                        specify a conan build option
  -s SETTING, --conan-setting SETTING
                        specify a conan build setting
  --with-aasb, --aasb   include aasb messages (default: True)
  --no-aasb
  --with-docs, --docs   include docs (default: True)
  --no-docs
  --with-unit-tests, --unit-tests
                        include unit tests (default: False)
  --no-unit-tests
  --with-sampleapp, --sampleapp
                        include sample app (default: False)
  --no-sampleapp
  --with-sensitive-logs, --sensitive-logs
                        emit sensitive data in debugging logs (default: False)
  --no-sensitive-logs
  --with-latency-logs, --latency-logs
                        emit latency data in debugging logs (default: False)
  --no-latency-logs
  --output FILE         filename for output build archive
  --no-output           don't create output package
  --skip-config         skip build configuration
```

## Clean command

```shell
$ build.py clean [-h] [--home PATH] [-v] [--skip-conan] [--skip-gradle] 
                 pattern
```
Used to clean packages from the Builder, Conan, and Gradle caches.

```shell
positional arguments:
  pattern        pattern or package name

optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
  --skip-conan   skips cleaning the conan cache
  --skip-gradle  skips cleaning the gradle cache
```

## Configure command
```shell
$ build.py configure [-h] [--home PATH] [-v] {init,export} ...

optional arguments:
  -h, --help     show this help message and exit

commands:
  {init,export}
    init         initializes the builder configuration
    export       exports packages from the builder configuration
```

### configure init
```shell
$ build.py configure init [-h] [--home PATH] [-v]
```
Used to initialize the Builder configuration settings.

```shell
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### configure export
```shell
$ build.py configure export [-h] [--home PATH] [-v] PATTERN
```
Used to re-export packages that are configured by the builder.

```shell
positional arguments:
  PATTERN        pattern or package name

optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

## Imports command
```shell
$ build.py imports [-h] {init,list,add,remove,enable,disable} ...

optional arguments:
  -h, --help            show this help message and exit

commands:
  {init,list,add,remove,enable,disable}
    init                initialize the imports configuration
    list                lists imports managed by the configuration
    add                 adds a new import
    remove              removes imports
    enable              enables imports
    disable             disables imports
```

### imports init
```shell
$ build.py imports init [-h] [--home PATH] [-v]
```
Used to initialize the Builder imports settings.

```shell
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### imports list
```shell
$ build.py imports list [-h] [--home PATH] [-v]
```
Used to display the imports in the Builder settings.

```shell
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### imports add
```shell
$ build.py imports add [-h] [--home PATH] [-v] NAME PATH
```
Used create a new import in the Builder settings.

```shell
positional arguments:
  NAME           import name
  PATH           import search path
  
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### imports remove
```shell
$ build.py imports remove [-h] [--home PATH] [-v] PATTERN
```
Used to remove imports from the Builder settings.

```shell
positional arguments:
  PATTERN        import name or pattern
  
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### imports enable
```shell
$ build.py imports enable [-h] [--home PATH] [-v] PATTERN
```
Used to enable imports in the Builder settings.

```shell
positional arguments:
  PATTERN        import name or pattern
  
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```

### imports disable
```shell
$ build.py imports disable [-h] [--home PATH] [-v] PATTERN
```
Used to disable imports in the Builder settings.

```shell
positional arguments:
  PATTERN        import name or pattern
  
optional arguments:
  -h, --help     show this help message and exit
  --home PATH    override builder home path
  -v, --verbose  enable verbose logging
```
