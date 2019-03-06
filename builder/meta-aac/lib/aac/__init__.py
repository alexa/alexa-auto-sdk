import subprocess, oe.path

def get_version(d):
    return subprocess.check_output([
        oe.path.join(d.getVar('AAC_SCRIPTS_DIR'), "gen-version.sh"),
        "-b"]).decode("utf-8")

def get_codename(d):
    return subprocess.check_output([
        oe.path.join(d.getVar('AAC_SCRIPTS_DIR'), "gen-version.sh"),
        "-c"]).decode("utf-8")