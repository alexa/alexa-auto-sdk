#!/usr/bin/python3
import os, subprocess


def findRecipes(root, search_paths, recipes=[]):
    for nextSearchPath in search_paths:
        for (path, dirnames, filenames) in os.walk(os.path.join(root, nextSearchPath)):
            if not path.endswith("/test_package"):
                for name in filenames:
                    if name == "conanfile.py":
                        file = os.path.join(path, name)
                        if not file in recipes:
                            recipes.append(file)
    return recipes


# root sdk path to search for recipe in
sdk_root = os.path.abspath(os.path.join(__file__, "../.."))
# aac-sdk-tools must always be exported first, since it is
# used as a python require in other recipes!
recipes = [os.path.join(sdk_root, "conan/recipes/aac-sdk-tools")]
# search the specified paths for conan recipes
findRecipes(sdk_root, ["conan/recipes", "modules", "extensions", "tools", "platforms"], recipes)
# iterate through each recipe and export it
for nextRecipe in recipes:
    subprocess.call(["conan", "export", nextRecipe])
# exort the configuration data
config = os.path.abspath(os.path.join(__file__, "../config"))
if os.path.exists(config):
    subprocess.call(["conan", "config", "install", "-t", "dir", config])
