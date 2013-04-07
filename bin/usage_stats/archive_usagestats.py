import os, sys, shutil, subprocess

if len(sys.argv) < 3:
    print("Usage: {0} month year".format(sys.argv[0]))
    print("Example: {0} january 2012".format(sys.argv[0]))
    raise SystemExit
else:
    month=sys.argv[1].capitalize()
    year=sys.argv[2]
    target = "{0}-{1}".format(month.lower(), year)
    new_title = "Usage statistics for {0} {1}".format(month, year)
if "/" in target:
    print("archive-dir-name can not contains path separators")
    raise SystemExit

dir = os.path.join(os.path.dirname(__file__), "formatted")

copy_dirs = set([name for name in os.listdir(dir) if os.path.isdir(os.path.join(dir, name))])

# Don't copy these dirs
copy_dirs -= set(("Past Stats", "past-stats", "poke_icons", "poke_img", "screens"))
# not a dir but should be copied
copy_files = set(("index.html",))

target_dir = os.path.join(dir, "Past Stats", target)
if not os.path.exists(target_dir):
    os.mkdir(target_dir)
else:
    print("The directory already exists.")

args = ["zip", "-r", os.path.join(target_dir, "raw.zip"), "raw"]
cwd = os.path.dirname(__file__) or "."
p = subprocess.Popen(args, cwd=cwd)
ret = p.wait()
if ret != 0:
    print("zip failed. Is 'zip' installed?")
    raise SystemExit

for file in copy_dirs:
    from_f = os.path.join(dir, file)
    to_f = os.path.join(target_dir, file)
    print("copy from {0} to {1}".format(from_f, to_f))
    shutil.copytree(from_f, to_f)
for file in copy_files:
    from_f = os.path.join(dir, file)
    to_f = os.path.join(target_dir, file)
    print("copy from {0} to {1}".format(from_f, to_f))
    shutil.copyfile(from_f, to_f)

#print("Fixing the index file")

#from_index = os.path.join(dir, "index.html")
#to_index = os.path.join(target_dir, "index.html")
#destination = open(to_index, "w")
#source = open(from_index, "r")
#for line in source:
#    if line.startswith("<title>"):
#        destination.write("<title>{0}</title>\n".format(new_title))                                                                                                                                                                                             1,1           Top
#    elif "Past%20Stats" in line:
#        destination.write('<a href="../../index.html">Current stats</a>')
#    else:
#        destination.write(line)
#source.close()
#destination.close()

#print("Fixing the Past Stats index")
#past_stats_index = os.path.join(dir, "Past Stats", "index.html")
#shutil.copy(past_stats_index, past_stats_index+"~")

#destination = open(past_stats_index, "w")
#source = open(past_stats_index+"~", "r")
#for line in source:
#    destination.write(line)
#    if '<a href="../../index.html">Current stats</a>' in line:
#        destination.write("<br>\n")
#        destination.write("        <a href='{0}/index.html'>{1} {2}</a>\n".format(target, month, year))
#source.close()
#destination.close()

                                                                                                                                                                                                                           79,0-1        Bot
