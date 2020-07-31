# Common flags for AGL builds
# AGL Halibut (v8.0.0+) and above now use wireplumber audio

EXTRA_OECMAKE += "-DUSE_PIPEWIRE=ON"