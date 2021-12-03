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
ignore_count=0
if test -f "$inputfile"; then
	echo -e "\nIgnored types in \"$inputfile\":"
	while IFS= read line || [ -n "$line" ] ;  # OR condition checks if EOF at the end of line
	do
		echo "$line" 
		ignored_list[$ignore_count]=$line
	  	ignore_count=$((ignore_count+1))
	done < "$inputfile"	
	echo -e "\n"
else
	echo "$inputfile DOES NOT exist.....Try Again!"
fi



# recursively copying the folders
echo "starting to copy files...."
rm -r -f "../output_dir"
mkdir "../output_dir"
parent_dir="../output_dir"
find "$rootdir" -type f -exec cp {} "$parent_dir" \;
for var in "${ignored_list[@]}"
do
	find "$parent_dir" -name "*.$var" -type f -delete
done
echo "copied files to output_dir in parent directory!"



# getting all the unique extensions
echo "starting to arrange files...."
cd $parent_dir
extensions_list=()
ex_count=0
curr_ext=""
other="no"
for file in *;
do
	if [[ $file != *.* ]]; then
		other="yes"
		continue
	fi	
	IFS='.'; 
	split=($file); 
	unset IFS;
	if [[ "$curr_ext" == "${split[1]}" ]]; then
		continue
	fi
        curr_ext=${split[1]}
        extensions_list[$ex_count]=$curr_ext
        ex_count=$((ex_count+1))
done



# arranging the files in subdirectories
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
find_loc="$(basename "$rootdir")"
for ext in "${extensions_list[@]}";
do
	touch "./$ext/desc_$ext.txt"
	cd "$ext/"
	for file in *.$ext;
	do
		cd ../..
		path_to_file=`find "$find_loc" -type f -name "$file"`
		# echo -e "$path_to_file"
		
		if [[ "$path_to_file" =~ $'\n' ]]; then
			IFS=$'\n'; 
			split=($path_to_file); 
			unset IFS;
			path_to_file="${split[-1]}"
			# echo "$path_to_file- is da best"
		fi		
		
		cd "output_dir/$ext"
		echo "$path_to_file" >> "desc_$ext.txt"
	done
	cd ..
done

if [[ $other == "yes" ]]; then
	touch "./others/desc_others.txt"
	cd "others/"
	for file in *;
	do	
		# echo $file
		if [[ $file == *.* ]]; then
		continue
		fi
		cd ../..
		path_to_file=`find "$find_loc" -type f -name "$file" -printf "%p\n"`
		# echo "$path_to_file"
		
		if [[ "$path_to_file" =~ $'\n' ]]; then
			IFS=$'\n'; 
			split=($path_to_file); 
			unset IFS;
			path_to_file="${split[-1]}"
			# echo "$path_to_file- is da best"
		fi
		
		cd "output_dir/others"
		echo "$path_to_file" >> "desc_others.txt"
	done
	cd ..
fi
echo "done arranging files with path descriptions!"



# csv file task
































echo -e "script terminated!\n"