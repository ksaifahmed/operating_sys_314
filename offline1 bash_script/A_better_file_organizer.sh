#!/bin/bash
# prints user guide
print_manual() {
	echo -e "use command like this: \"./script.sh [working_directory] [path_to_filename]\""
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
rootdir=""
inputfile=""
parent_dir=""


# if only one argument is provided
if [ $# -eq 1 ]; then
	inputfile=$1
	rootdir="$(dirname "$(readlink -fm "$0")")"
	echo -e "using script's directory as root: \"$rootdir\""
elif [ $# -eq 2 ]; then # correct no, of args
	    rootdir=$1
	    inputfile=$2
	    echo -e "using this directory as root: \"$rootdir\""
fi


# printing the ignored filetypes
declare -a ignored_list
ignore_ext_count=0
if test -f "$inputfile"; then
	echo -e "\nIgnored types in \"$inputfile\":"
	while IFS= read -r line || [ -n "$line" ] ;  # OR condition checks if EOF at the end of line
	do
		line=`echo $line | sed 's/[^0-9A-Za-z]*//g'`;  #escaping SOME CHARACTERS!!!
		echo "*.$line"		
		ignored_list[$ignore_ext_count]=$line
		ignore_ext_count=$((ignore_ext_count+1))
	done < "$inputfile"	
	echo -e "\n"
else
	echo "$inputfile DOES NOT exist.....Try Again!"
fi


# setting up root and outpur dir
cd "$rootdir"
rootdir="$(basename "$rootdir")"
rm -r -f "../output_dir"
mkdir "../output_dir"
parent_out="../output_dir"



# function for matching ignored types
is_ignored() {
	for var in "${ignored_list[@]}";
	do
		if [[ "$var" == "$1" ]]; then 
			return 0
		fi
	done
	return 1
}


declare -A path_list
declare -A extensions_list
declare -A count_array

file_ext=""
other=""
ignore_count=0


# recursively copying the files
copy_recurser() {
    for i in "$1"/*;do
        if [ -d "$i" ];then
            # echo "dir: $i"
            copy_recurser "$i"
        elif [ -f "$i" ]; then
			# removes first dot
			IFS='.'; 
			split=($i); 
			unset IFS;
			
			# get path and filename
            path="$rootdir${i:1}"
			filename="$(basename "$i")"

			
			# check if ignored
			if [[ $filename == *.* ]]; then
				
				# get extension
				IFS='.'; 
				ext_split=($filename); 
				file_ext="${ext_split[1]}"
				unset IFS;
	
				# check if extension is ignored
				if is_ignored "$file_ext"; then
					ignore_count=$((ignore_count+1))
					continue
				fi
			fi
			
			
			# else store path and copy
			path_list["$filename"]="$path"
			cp -f "$i" "$parent_out"
			
			# if no extensions:			
			if [[ $filename != *.* ]]; then
				other="yes"
			else
				# store extensions
				extensions_list["$file_ext"]="$file_ext"
			fi
			
        fi
    done
}
copy_recurser "."
count_array["ignored"]="$ignore_count"



# arranging the files in subdirectories
cd "$parent_out"
for ext in "${extensions_list[@]}";
do
	mkdir $ext
	find . -name "*.$ext" -exec mv -t "./$ext" {} +
done

if [[ $other == "yes" ]]; then
	mkdir "others"
	find . -type f ! -name "*.*" -exec mv -t "./others" {} +
fi




# writing the paths into file
for ext in "${extensions_list[@]}";
do
	count=0
	paths=""
	cd "$ext/"
	
	for file in *.$ext;
	do
		paths="$paths${path_list["$file"]}\n"
		count=$((count+1))
	done
		
	touch "desc_$ext.txt"
	echo -e "$paths" >> "desc_$ext.txt"
	count_array["$ext"]="$count"
	cd ..	
done

if [[ $other == "yes" ]]; then
	count=0
	paths=""
	cd "others/"
	
	for file in *;
	do
		paths="$paths${path_list["$file"]}\n"
		count=$((count+1))
	done
		
	touch "desc_others.txt"
	echo -e "$paths" >> "desc_others.txt"
	count_array["others"]="$count"
	cd ..	
fi



# print the count
# for key in ${!count_array[@]}; do
#    echo ${key} ${count_array[${key}]}
# done
# echo "ignored: $ignore_count"



# csv tasks
cd ..
touch "output.csv"
echo "file_type,no_of_files" > "output.csv"
for key in ${!count_array[@]}; do
	echo "${key},${count_array[${key}]}" >> "output.csv"
done



# Script End Message
echo -e "script terminated!\n"	