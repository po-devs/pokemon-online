set -e 
export LD_LIBRARY_PATH=.

PS="/home/coyotte/own-po/bin/usage_stats/formatted/Past Stats"
dirs=$(ls -1 "$PS" | grep -v .html)

for dir in $dirs
do
     mkdir /tmp/${dir}
     cp "$PS"/${dir}/raw.zip /tmp/${dir}
     (cd /tmp/${dir} && unzip raw.zip)
     echo "(cd .. && ./StatsExtracter --input /tmp/${dir} --output usage_stats/formatted/past-stats/${dir})"
     if [ -d /tmp/${dir}/raw ]
     then 
       (cd .. && ./StatsExtracter --input /tmp/${dir}/raw --output usage_stats/formatted/past-stats/${dir})
     else
       (cd .. && ./StatsExtracter --input /tmp/${dir} --output usage_stats/formatted/past-stats/${dir})
     fi
     cp "$PS"/${dir}/raw.zip formatted/past-stats/${dir}/
     rm -r /tmp/${dir}
     ./gen_past-stats_index.sh > formatted/past-stats/index.html
done
