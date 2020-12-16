# Alexa Auto Navigation

This library serves the following purposes:

* It handles navigation-related directives. By parsing a navigation directive and interfacing
   with the selected map provider, it performs the action required by the directive. The map provider used by the AACS Sample App is Google Maps.
   However, you can extend the library to use other map providers.
   This library supports navigating to a single waypoint and canceling an ongoing navigation.

* It handles local search template runtime directives. This library is one of the subscribers for
   the `TemplateRuntime::RenderTemplate` directive. It parses the incoming directive and renders
   local search cards in the current voice session provided by the Alexa Voice Interaction Service (VIS).

  ## Prerequisites
  To see local search templates your device type needs to be navigation capable.
