cd formatted/past-stats

echo "<html><head><title>past stats index (placeholder)</title></head><body><ul>"

dirs=$(ls -1 | grep -v .html | sort)
for dir in $dirs
do
     echo "<li><a href=\"${dir}\">${dir}</a></li>"
done

echo "</ul></body></html>"
