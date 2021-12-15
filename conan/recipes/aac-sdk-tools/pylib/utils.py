import os, logging

# merges the values of two dictionaries or sets and returns a new instance
def merge_values( a, b ): 
    out = a.copy()
    out.update(b)
    return out

def bool_value( value, true_value = True, false_value = False ):
    return true_value if value else false_value

def get_os_lib_name( os, name ):
    return f"{get_os_lib_prefix(os)}{name}{get_os_lib_ext(os)}"

def get_os_lib_prefix( os ):
    return "lib"

def get_os_lib_ext( os ):
    if os == "Macos":
        return ".dylib"
    elif os == "Windows":
        return ".dll"
    else:
        return ".so"

def list_files( root_folder, search_path, ext, use_relpath = True, deep = True ):
    file_list = []
    normalized_search_path = os.path.abspath( os.path.join( root_folder, search_path ) )
    for (dirpath, dirnames, filenames) in os.walk( normalized_search_path ):
        for name in filenames:
            if name.endswith( f".{ext}" ):
                file_path = os.path.join( dirpath, name )
                file_list.append( os.path.relpath( file_path, root_folder ) if use_relpath else file_path )
        if not deep: break
    return file_list

def get_android_abi( arch ):
    return {
        "x86": "x86",
        "x86_64": "x86_64",
        "armv7": "armeabi-v7a",
        "armv8": "arm64-v8a"
    }.get( arch )

def chmod_mode( filename, mode ):
    if os.name == "posix":
        os.chmod(filename, os.stat(filename).st_mode | mode)

def fix_permissions( dir, mode ):
    if os.name != "posix":
        return
    for root, _, files in os.walk(dir):
        for filename in files:
            filename = os.path.join(root, filename)
            with open(filename, "rb") as f:
                sig = f.read(4)
                if type(sig) is str:
                    sig = [ord(s) for s in sig]
                else:
                    sig = [s for s in sig]
                if len(sig) > 2 and sig[0] == 0x23 and sig[1] == 0x21:
                    logging.info(f"chmod: {filename}")
                    chmod_mode(filename, mode)
                elif sig == [0x7F, 0x45, 0x4C, 0x46]:
                    logging.info(f"chmod: {filename}")
                    chmod_mode(filename, mode)
                elif sig == [0xCA, 0xFE, 0xBA, 0xBE] or \
                        sig == [0xBE, 0xBA, 0xFE, 0xCA] or \
                        sig == [0xFE, 0xED, 0xFA, 0xCF] or \
                        sig == [0xCF, 0xFA, 0xED, 0xFE] or \
                        sig == [0xFE, 0xEF, 0xFA, 0xCE] or \
                        sig == [0xCE, 0xFA, 0xED, 0xFE]:
                    logging.info(f"chmod: {filename}")
                    chmod_mode(filename, mode)
