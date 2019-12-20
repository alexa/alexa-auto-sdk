import subprocess, oe.path

def get_version(d):
    return subprocess.check_output([
        oe.path.join(d.getVar('AAC_SCRIPTS_DIR'), "gen-version.sh"),
        "-b"]).decode("utf-8")

def get_codename(d):
    return subprocess.check_output([
        oe.path.join(d.getVar('AAC_SCRIPTS_DIR'), "gen-version.sh"),
        "-c"]).decode("utf-8")

def find_latest_version(prefix, path):
    import glob
    import re

    pv = ""
    pvv = 0
    pattern = re.compile(prefix + '([0-9]+(?:\.[0-9]+)+)')
    for name in glob.glob(path):
        match = re.search(pattern, name)
        if match:
            mv = match.group(1)
            mvv = 0
            vers = mv.split('.')
            while len(vers) < 3: # major.minor.patch
                vers.append(str(0))
            while len(vers) > 0:
                mvv = (mvv * 1000) + int(vers.pop(0))
            if (pvv < mvv):
                pv = mv
                pvv = mvv
    if pv:
        return pv
    else:
        return None

def get_external_binary_arch(d):
    overrides = d.getVar('OVERRIDES').split(':')
    platform = "linux"
    if "android" in overrides:
        platform = "android"
    elif "qnx" in overrides:
        platform = "qnx7"

    arch = d.getVar('TARGET_ARCH')
    binary_arch = arch
    if arch == "arm":
        features = d.getVar('TUNE_FEATURES').split(' ')
        if "armv7a" in features or "armv7ve" in features:
            # armv7a implies Cortex-A + NEON available
            binary_arch = "armv7a"
        # hf suffix should be added
        if "callconvention-hard" in features:
            binary_arch = binary_arch + "hf"
    elif arch == "i686" or arch == "i586":
        binary_arch = "x86"

    return "%s-%s" % (platform, binary_arch)
