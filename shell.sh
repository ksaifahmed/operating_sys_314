#!/bin/bash
# prints user guide
print_manual() {
	echo "use command like this: script.sh directory filename"
	echo "if directory is not provided, root is assumed"
}


# checking whether cmd line arg is provided
if [ $# -eq 0 ]; then
	echo "No arguments provided!"
	print_manual
	exit 1
fi
if [ $# -gt 2 ]; then
	echo "Too many arguments"
	print_manual
	exit 1
fi


# init some vars
filename=""
rootdir=""
inputfile=""

# if only one argument is provided
if [ $# -eq 1 ]; then
	echo "Using root dir..."
	filename=$1
	inputfile=$filename
elif [ $# -eq 2 ]; then # correct no, of args
	    rootdir=$1
	    filename=$2
	    inputfile="$rootdir/$filename"
fi

# printing the filenames
if test -f "$inputfile"; then
	echo -e "\nIgnored types in $inputfile:"
	while IFS= read -r line
	do
	  echo -e "$line"
	done < "$inputfile"	
	echo -e "\n"
else
	echo "$inputfile DOES NOT exist.....Try Again!"
fi




































