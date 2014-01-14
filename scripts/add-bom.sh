#!/bin/bash

#find db -name "*.txt" -exec ./add-bom.sh {} \;;find db -name "*.txt~"
#find db -name "*.txt~" -exec rm {} \;

set -o nounset
set -o errexit


DELETE_ORIG=true
DELETE_FLAG=""
RECURSIVE=false
PROCESSALLFILE=false
PROCESSING_FILES=false
PROCESSALLFILE_FLAG=""
SED_EXEC=sed
USE_EXT=false
FILE_EXT=""
TMP_CMD="mktemp"
TMP_OPTS="--tmpdir="
XDEV=""
ISDARWIN=false


if [ $(uname) == "SunOS" ] ; then
  if [ -x /usr/gnu/bin/sed ] ; then
    echo "Using GNU sed..."
    SED_EXEC=/usr/gnu/bin/sed
  fi 
  TMP_OPTS="-p "
fi


if [ $(uname) == "Darwin" ] ; then
  TMP_OPTS="-t tmp"

  SED_EXEC="perl -pe"
  echo "Using perl..."
  ISDARWIN=true

fi


function usage() {
  echo "bom-remove [-adrx] [-s sed-name] [-e ext] files..."
  echo ""
  echo "  -a    Remove the BOM throughout the entire file."
  echo "  -e    Look only for files with the chosen extensions."
  echo "  -d    Do not overwrite original files and do not remove temp files."
  echo "  -r    Scan subdirectories."
  echo "  -s    Specify an alternate sed implementation."
  echo "  -x    Don't descend directories in other filesystems."
}


function checkExecutable() {
  if ( ! which "$1" > /dev/null 2>&1 ); then
    echo "Cannot find executable:" $1
    exit 4
  fi
}


function parseArgs() {
  while getopts "adfrs:e:x" flag
  do
    case $flag in
      a) PROCESSALLFILE=true ; PROCESSALLFILE_FLAG="-a" ;;
      r) RECURSIVE=true ;;
      f) PROCESSING_FILES=true ;;
      s) SED_EXEC=$OPTARG ;;
      e) USE_EXT=true ; FILE_EXT=$OPTARG ;;
      d) DELETE_ORIG=false ; DELETE_FLAG="-d" ;;
      x) XDEV="-xdev" ;;
      *) echo "Unknown parameter." ; usage ; exit 2 ;; 
    esac
  done


  shift $(($OPTIND - 1))



  if [ $# == 0 ] ; then
    usage;
    exit 2;
  fi



  # fixing darwin
  if [[ $ISDARWIN == true && $PROCESSALLFILE == false ]] ; then
    PROCESSALLFILE=true
    echo "Process all file is implicitly set on Darwin."
  fi

  FILES=("$@")


  if [ ! -n "$FILES" ]; then
    echo "No files specified. Exiting."
  fi


  if [ $RECURSIVE == true ]  && [ $PROCESSING_FILES == true ] ; then
    echo "Cannot use -r and -f at the same time."
    usage
    exit 1
  fi


  checkExecutable $SED_EXEC
  checkExecutable $TMP_CMD
}


function processFile() {
  if [ $(uname) == "Darwin" ] ; then
#    TEMPFILENAME=$($TMP_CMD $TMP_OPTS)
    TEMPFILENAME=$1~
  else
    TEMPFILENAME=$1~
  fi
  echo "Processing $1 using temp file $TEMPFILENAME"

#Convert to utf-8
  (iconv -t utf-8 "$1" -o "$1~" || iconv -f iso−8859−1 -t utf-8 "$1" -o "$1~")
#Remove BOM if there
  cat "$1~" | $SED_EXEC '1 s/\xEF\xBB\xBF//' > "$1"
#Add BOM
  cat "$1" | $SED_EXEC '0,/\(\)/ s//\xEF\xBB\xBF\0/' > "$1~"

  mv "$1~" "$1"
}


function doJob() {
  # Check if the script has been called from the outside.
  if [ $PROCESSING_FILES == true ] ; then
    for i in $(seq 1 ${#FILES[@]})
    do
      echo ${FILES[$i-1]}
      processFile "${FILES[$i-1]}"
    done


  else
    # processing every file
for i in $(seq 1 ${#FILES[@]})
do
CURRFILE=${FILES[$i-1]}
      # checking if file or directory exist
      if [ ! -e "$CURRFILE" ] ; then echo "File not found: $CURRFILE. Skipping..." ; continue ; fi
      
      # if a paremeter is a directory, process it recursively if RECURSIVE is set
      if [ -d "$CURRFILE" ] ; then
        if [ $RECURSIVE == true ] ; then
          if [ $USE_EXT == true ] ; then
            find "$CURRFILE" $XDEV -type f -name "*.$FILE_EXT" -exec "$0" $DELETE_FLAG $PROCESSALLFILE_FLAG -f "{}" \;
          else
            find "$CURRFILE" $XDEV -type f -exec "$0" $DELETE_FLAG $PROCESSALLFILE_FLAG -f "{}" \;
          fi
        else
          echo "$CURRFILE is a directory. Skipping..."
        fi
      else
        processFile "$CURRFILE"
      fi
    done
  fi
}


parseArgs "$@"
doJob