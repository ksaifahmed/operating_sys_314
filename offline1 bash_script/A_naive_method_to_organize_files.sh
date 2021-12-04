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




# recursively copying the folders
echo "starting to copy files...."
rm -r -f "../output_dir"
mkdir "../output_dir"
parent_dir="../output_dir"
find "$rootdir" -type f -exec cp {} "$parent_dir" \;

# ignored files count
declare -A count_array # for CSV, contains count
ignored_file_count=0
for var in "${ignored_list[@]}";
do
	for file in ../output_dir/*.$var;
	do
		# echo "$file"
		ignored_file_count=$((ignored_file_count+1))
	done
done
count_array["ignored"]="$ignored_file_count"


# deleting ignored files
find_loc="$(basename "$rootdir")"
for ext in "${ignored_list[@]}"
do
	#echo -e "*.$ext\ta"
	#pwd
	find "$parent_dir" -type f -name "*.$ext" -exec rm -rf {} \;
done
echo "copied files to output_dir in parent directory!"

#find "$parent_dir" -type f -name "*.mp3" -delete

# getting all the extensions
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
# making the array unique
extensions_list=($(echo "${extensions_list[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' '))


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
for ext in "${extensions_list[@]}";
do
	touch "./$ext/desc_$ext.txt"
	cd "$ext/"
	count=0
	
	
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
		
		if [[ "$path_to_file" == "" ]]; then
			continue # don't write if null
		fi
		
		count=$((count+1))
		echo "$path_to_file" >> "desc_$ext.txt"
	done
	
	
	count_array["$ext"]="$count"
	# echo "$ext e $count" 
	cd ..
done

if [[ $other == "yes" ]]; then
	touch "./others/desc_others.txt"
	cd "others/"
	count=0
	
	
	for file in *;
	do	
		# echo $file
		if [[ $file == *.* ]]; then
		continue # skip if extensions
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
		
		if [[ "$path_to_file" == "" ]]; then
			continue # don't write if null
		fi
		
		count=$((count+1))
		echo "$path_to_file" >> "desc_others.txt"
	done
	
	count_array["others"]="$count"
	cd ..
fi
echo "done arranging files with path descriptions!"



# print the count
# for key in ${!count_array[@]}; do
#     echo ${key} ${count_array[${key}]}
# done


# csv tasks
cd ..
touch "output.csv"
echo "file_type,no_of_files" > "output.csv"
for key in ${!count_array[@]}; do
	echo "${key},${count_array[${key}]}" >> "output.csv"
done



# Script End Message
echo -e "script terminated!\n"