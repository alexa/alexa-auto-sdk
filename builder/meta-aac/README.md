# Alexa Auto support layer for OpenEmbedded

This layer enables Alexa Auto SDK components into the OpenEmbedded build infrastracture.

## Dependencies

This layer depends on
* [OpenEmbedded-Core](git://git.openembedded.org/openembedded-core)
  * Layers: meta
  * Branch: thud or rocko

## Available Options

The following options are available for configuration:

* `AAC_PREFIX` to specify the installation directory for AAC specific components. (Default `/opt/AAC`)
* `AAC_SENSITIVE_LOGS` to enable sensitive logs only when building with debugging options. (Default `OFF`)
* `AAC_LATENCY_LOGS` to enable user perceived latency logs only when building with debugging options. (Default `OFF`)
* `AAC_ENABLE_TESTS` to enable bulding test packages for AAC modules. (Default `OFF`)
