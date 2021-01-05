#
# update-contributors.py
# 32blit
#
# pull contributors from github and update the list
#
import os
import requests

# ignore list, these will be removed. Pimoroni staff
ignore = ["Gadgetoid"]

# nickname list, these conversions will be made.
nicknames = {
    "lenardg": "LenardG"
}

def checkNickname(contributor):
    if contributor in nicknames:
        return nicknames[contributor]
    return contributor


# Start program
print("32blit Contributors Updater")
print("Downloading contributors from Github ...")

r = requests.get('https://api.github.com/repos/pimoroni/32blit-beta/contributors',
        headers = {"Accept": "application/vnd.github.v3+json"})

if r.status_code != 200:
    print("API returned {0} response. Aborting.".format(r.status_code))
    exit()

contributors = r.json()

contrib_hpp = "// contrib.hpp\n" \
  "// 32blit GitHub contributors\n" \
  "//\n" \
  "// AUTO GENERATED FILE, PLEASE DO NOT EDIT BY HAND\n" \
  "//\n" \
  "#pragma once\n" \
  "\n"

# Add regular contributors
contrib_hpp = contrib_hpp + "static const char * contributors[] = {\n"

for c in sorted((regular["login"] for regular in contributors if regular["contributions"] < 100),key=str.casefold):
    name = checkNickname(c)
    if name in ignore:
        continue
    contrib_hpp = contrib_hpp + "  \"{0}\",\n".format(name)
    #print("{0} with {1} commits".format( c["login"], c["contributions"]))

contrib_hpp = contrib_hpp + "  nullptr\n};\n"

# Add special contributors
contrib_hpp = contrib_hpp + "static const char * specialthanks[] = {\n"

for c in sorted((regular["login"] for regular in contributors if regular["contributions"] >= 100),key=str.casefold):
    name = checkNickname(c)
    if name in ignore:
        continue
    contrib_hpp = contrib_hpp + "  \"{0}\",\n".format(name)
    #print("{0} with {1} commits".format( c["login"], c["contributions"]))

contrib_hpp = contrib_hpp + "  nullptr\n};\n"

# Write file to disk
filename = os.path.join(os.path.dirname(os.path.realpath(__file__)), "contrib.hpp")

h = open(filename, "w")
h.write(contrib_hpp)
h.close()

print("Header '{0}' written".format(filename))
print("Done.")
